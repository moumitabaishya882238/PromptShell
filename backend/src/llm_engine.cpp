#include "llm_engine.h"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>


LLMEngine::LLMEngine(const std::string &enginePath,
                     const std::string &modelPath, const std::string &outFile)
    : engine_path_(enginePath), model_path_(modelPath), out_file_(outFile) {}

int LLMEngine::run_prompt(const std::string &full_prompt) {
  // Write prompt to temp file
  std::string prompt_file = out_file_ + ".prompt.txt";
  std::ofstream pfs(prompt_file);
  if (!pfs.is_open()) {
    std::cerr << "[ERROR] Failed to write prompt file: " << prompt_file
              << std::endl;
    return -1;
  }
  pfs << full_prompt;
  pfs.close();

  // Build command with timeout wrapper (30 seconds max)
  // Use forward slashes for MSYS2 bash compatibility
  std::string cmd = "timeout 30 " + engine_path_ + " -m \"" + model_path_ + 
                    "\" -f \"" + prompt_file + "\" --no-display-prompt " +
                    "-c 4096 --temp 0 -n 512 > \"" + out_file_ + "\" 2>&1";

  int rc = system(cmd.c_str());

  // Check if output file has content
  std::ifstream check(out_file_);
  if (check.good()) {
    std::string content((std::istreambuf_iterator<char>(check)),
                        std::istreambuf_iterator<char>());
    check.close();
  } else {
    std::cerr << "[ERROR] Cannot open output file: " << out_file_ << std::endl;
  }

  return rc;
}

std::string LLMEngine::read_output() {
  std::ifstream ifs(out_file_, std::ios::in | std::ios::binary);
  std::ostringstream oss;
  oss << ifs.rdbuf();
  return oss.str();
}
