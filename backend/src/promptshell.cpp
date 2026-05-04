#include "promptshell.h"
#include "assistant_mode.h"
#include "llm_engine.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>

static void type_animation(const std::string &text) {
  // Print text with typing animation (15ms per character)
  for (char c : text) {
    std::cout << c << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
  }
}

PromptShell::PromptShell(const std::string &enginePath,
                         const std::string &modelPath,
                         const std::string &outFile)
    : engine_path_(enginePath), model_path_(modelPath), out_file_(outFile) {
  system_prompt_ =
      "You are PromptShell, a focused C++ coding assistant.\n"
      "Follow these rules exactly:\n"
      "- Answer directly; never say you cannot access a language or tools.\n"
      "- When the user provides source code, compiler output, or runtime "
      "output, answer only about that code and ignore unrelated topics.\n"
      "- Provide a concise explanation and, when relevant, a short C++ code "
      "example.\n"
      "- Keep output terminal-friendly (no markdown fences), simple, and "
      "brief.\n"
      "Examples:\n"
      "Q: What is a pointer in C++?\n"
      "A: A pointer holds the memory address of a variable. Example: int x=5; "
      "int* p=&x; // p points to x\n";
}

std::string
PromptShell::build_full_prompt(const std::string &user_prompt) const {
  return std::string("<|system|>\n") + system_prompt_ + "\n<|user|>\n" +
         user_prompt + "\n<|assistant|>\n";
}

static std::string trim_copy(std::string s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
  return s;
}

std::string PromptShell::extract_assistant_reply(const std::string &raw) const {
  std::string candidate = raw;
  
  // Look for the assistant tag
  size_t pos = raw.rfind("<|assistant|>");
  if (pos != std::string::npos) {
    size_t start = pos + 13;
    if (start < raw.size() && raw[start] == '\n') start++;
    else if (start < raw.size() && raw[start] == '\r') {
      start++;
      if (start < raw.size() && raw[start] == '\n') start++;
    }
    candidate = raw.substr(start);
  } else {
    size_t t_pos = raw.rfind("... (truncated)");
    if (t_pos != std::string::npos) {
      candidate = raw.substr(t_pos + 15);
    } else {
      size_t p2 = raw.find("\n\n");
      if (p2 != std::string::npos)
        candidate = raw.substr(p2 + 2);
    }
  }
  
  std::istringstream iss(candidate);
  std::string line;
  std::vector<std::string> lines;
  bool started_content = false;
  
  while (std::getline(iss, line)) {
    std::string t = trim_copy(line);
    if (t.empty()) continue;
    if (t == ">") continue;
    
    // Skip debug/prompt markers
    if (!t.empty() && t.front() == '[') {
      std::string lower = t;
      std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
      if (lower.find("prompt:") != std::string::npos || 
          lower.find("generation:") != std::string::npos)
        continue;
    }
    
    // Skip verbose preamble ("Sure, here's...", "Here's an...", etc.)
    if (!started_content) {
      std::string lower = t;
      std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
      if (lower.find("sure") != std::string::npos || 
          lower.find("here's") != std::string::npos ||
          lower.find("here is") != std::string::npos ||
          lower.find("example") != std::string::npos ||
          lower.find("response") != std::string::npos ||
          lower.find("begin") != std::string::npos ||
          lower.find("source") != std::string::npos) {
        continue;
      }
      started_content = true;
    }
    
    if (t.size() >= 2 && t[0] == '>' && t[1] == ' ')
      t = t.substr(2);
    lines.push_back(t);
  }
  
  if (lines.empty())
    return trim_copy(candidate);
    
  std::string out;
  for (size_t i = 0; i < lines.size(); ++i) {
    out += lines[i];
    if (i + 1 < lines.size())
      out += "\n";
  }
  return trim_copy(out);
}

std::string PromptShell::escape_json(const std::string &s) const {
  std::string out;
  out.reserve(s.size() * 2);
  for (char c : s) {
    switch (c) {
    case '\\':
      out += "\\\\";
      break;
    case '"':
      out += "\\\"";
      break;
    case '\n':
      out += "\\n";
      break;
    case '\r':
      out += "\\r";
      break;
    case '\t':
      out += "\\t";
      break;
    default:
      out += c;
      break;
    }
  }
  return out;
}

static std::unique_ptr<AssistantMode> make_mode(const std::string &name) {
  std::string n = name;
  for (auto &c : n)
    c = std::tolower((unsigned char)c);
  if (n == "code-only" || n == "codeonly" || n == "code")
    return std::make_unique<CodeOnlyMode>();
  if (n == "concise")
    return std::make_unique<ConciseMode>();
  if (n == "debug")
    return std::make_unique<DebugMode>();
  return std::make_unique<DefaultMode>();
}

int PromptShell::run(const std::string &user_prompt,
                     const std::string &output_mode,
                     const std::string &assistant_mode_name) {
  auto mode = make_mode(assistant_mode_name);
  // incorporate assistant system instructions
  std::string full_prompt = build_full_prompt(user_prompt);
  std::string instr = mode->system_instructions();
  if (!instr.empty()) {
    full_prompt = std::string("<|system|>\n") + system_prompt_ + "\n" + instr +
                  "\n<|user|>\n" + user_prompt + "\n<|assistant|>\n";
  }

  LLMEngine engine(engine_path_, model_path_, out_file_);
  int rc = engine.run_prompt(full_prompt);

  // Always attempt to read output, even if exit code is non-zero
  std::string raw = engine.read_output();

  // Only fail if output is empty
  if (raw.empty()) {
    std::cerr << "[ERROR] No output from LLMEngine (exit code: " << rc << ")"
              << std::endl;
    return (rc != 0) ? rc : -1;
  }
  std::string cleaned = extract_assistant_reply(raw);
  // allow mode-specific postprocessing
  cleaned = mode->postprocess(cleaned);

  if (output_mode == "json") {
    std::cout << "{\"response\":\"" << escape_json(cleaned)
              << "\",\"mode\":\"json\"}" << std::endl;
  } else if (output_mode == "both") {
    std::cout << "--- Response (pretty) ---\n";
    type_animation(cleaned);
    std::cout << "\n--- JSON ---\n";
    std::cout << "{\"response\":\"" << escape_json(cleaned)
              << "\",\"mode\":\"both\"}" << std::endl;
  } else {
    type_animation(cleaned);
    std::cout << "\n" << std::endl;
  }
  return 0;
}
