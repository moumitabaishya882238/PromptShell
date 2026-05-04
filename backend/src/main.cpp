#include "promptshell.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <cctype>
#include <sstream>
#include <vector>
#include <cstdio>
#include "coderunner.h"

namespace {

struct AppConfig {
    std::string mode = "pretty";
    std::string assistant_mode = "default";
    std::string run_file;
    std::string prompt;
    bool interactive = false;
    bool show_help = false;
};

bool starts_with(const std::string &value, const std::string &prefix) {
    return value.rfind(prefix, 0) == 0;
}

std::string trim_copy(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){ return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
    return s;
}

std::string recover_missing_first_letter(const std::string &line) {
    static const std::vector<std::string> known_commands = {
        "help", "?", "pwd", "ls", "touch", "mkdir", "cat", "edit", "open", "gedit",
        "shell", "ask", "ai", "chat", "run", "debug", "cd", "exit", "quit", ":q", "deactivate"
    };

    auto is_known = [&](const std::string &candidate) {
        for (const auto &cmd : known_commands) {
            if (candidate == cmd) return true;
            if (starts_with(candidate, cmd + " ")) return true;
        }
        return false;
    };

    if (is_known(line)) return line;

    static const std::string prefixes = "acdeghilmopqrstux";
    for (char prefix : prefixes) {
        std::string candidate = std::string(1, prefix) + line;
        if (is_known(candidate)) return candidate;
    }

    return line;
}

std::string get_option_value(const std::string &arg, const std::string &prefix) {
    if (starts_with(arg, prefix)) return arg.substr(prefix.size());
    return {};
}

AppConfig parse_args(int argc, char** argv) {
    AppConfig config;
    std::vector<std::string> prompt_parts;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            config.show_help = true;
            continue;
        }
        if (arg == "activate" || arg == "chat" || arg == "repl") {
            config.interactive = true;
            continue;
        }

        std::string value;
        if (!(value = get_option_value(arg, "--mode=")).empty()) {
            config.mode = value;
            continue;
        }
        if (!(value = get_option_value(arg, "--assistant-mode=")).empty()) {
            config.assistant_mode = value;
            continue;
        }
        if (!(value = get_option_value(arg, "--run-file=")).empty()) {
            config.run_file = value;
            continue;
        }
        if (!(value = get_option_value(arg, "--prompt=")).empty()) {
            config.prompt = value;
            continue;
        }
        if (arg == "--interactive") {
            config.interactive = true;
            continue;
        }

        // Fallback: treat a lone, non-option argument as a one-shot prompt.
        if (!starts_with(arg, "--")) {
            prompt_parts.push_back(arg);
        }
    }

    if (config.prompt.empty() && !prompt_parts.empty()) {
        for (size_t i = 0; i < prompt_parts.size(); ++i) {
            if (i > 0) config.prompt += " ";
            config.prompt += prompt_parts[i];
        }
    }

    if (config.prompt.empty() && !config.interactive && config.run_file.empty() && argc == 1) {
        config.interactive = true;
    }

    const char* env_mode = std::getenv("PROMPTSHELL_OUTPUT_MODE");
    if (!env_mode && config.mode.empty()) config.mode = "pretty";
    if (env_mode && config.mode == "pretty") config.mode = env_mode;

    const char* env_amode = std::getenv("PROMPTSHELL_ASSISTANT_MODE");
    if (env_amode && config.assistant_mode == "default") config.assistant_mode = env_amode;

    return config;
}

void print_help() {
    std::cout << "PromptShell - local AI coding assistant\n\n"
              << "Usage:\n"
              << "  promptshell activate\n"
              << "  promptshell chat\n"
              << "  promptshell --prompt=\"Hello\"\n"
              << "  promptshell --run-file=tests/xyz.cpp\n\n"
              << "Options:\n"
              << "  --mode=pretty|json|both\n"
              << "  --assistant-mode=default|code-only|concise|debug\n"
              << "  --run-file=<path>\n"
              << "  --prompt=<text>\n"
              << "  --interactive\n";
}

std::string read_file_to_string(const std::string &path) {
    std::ifstream ifs(path, std::ios::binary);
    std::ostringstream oss;
    oss << ifs.rdbuf();
    return oss.str();
}

std::string colored(const std::string &text, const std::string &code);

std::filesystem::path normalize_user_path(const std::string &input) {
    if (input.empty()) {
        return std::filesystem::path(std::getenv("HOME") ? std::getenv("HOME") : ".");
    }

    std::string path = input;
    std::replace(path.begin(), path.end(), '\\', '/');

    // Convert MSYS style /e/foo to Windows style E:/foo
    if (path.size() >= 3 && path[0] == '/' && std::isalpha(static_cast<unsigned char>(path[1])) && path[2] == '/') {
        char drive = static_cast<char>(std::toupper(static_cast<unsigned char>(path[1])));
        path = std::string(1, drive) + ":" + path.substr(2);
    }

    return std::filesystem::path(path);
}

void write_empty_file(const std::filesystem::path &path) {
    if (!std::filesystem::exists(path)) {
        std::ofstream ofs(path);
        ofs.close();
    }
}

void handle_touch(const std::string &args) {
    std::istringstream iss(args);
    std::string file;
    bool any = false;
    while (iss >> file) {
        std::filesystem::path p = std::filesystem::current_path() / file;
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }
        write_empty_file(p);
        any = true;
    }
    if (!any) {
        std::cerr << "touch: missing file operand" << std::endl;
    }
}

void handle_mkdir(const std::string &args) {
    std::istringstream iss(args);
    std::string dir;
    bool any = false;
    while (iss >> dir) {
        std::filesystem::create_directories(std::filesystem::current_path() / dir);
        any = true;
    }
    if (!any) {
        std::cerr << "mkdir: missing directory operand" << std::endl;
    }
}

void handle_cat(const std::string &args) {
    std::istringstream iss(args);
    std::string file;
    bool any = false;
    while (iss >> file) {
        std::filesystem::path p = std::filesystem::current_path() / file;
        std::ifstream ifs(p, std::ios::binary);
        if (!ifs) {
            std::cerr << "cat: cannot open " << p.string() << std::endl;
            continue;
        }
        std::cout << ifs.rdbuf();
        any = true;
    }
    if (!any) {
        std::cerr << "cat: missing file operand" << std::endl;
    }
}

void handle_edit(const std::string &args) {
    std::istringstream iss(args);
    std::string file;
    if (!(iss >> file)) {
        std::cerr << "edit: missing file operand" << std::endl;
        return;
    }

    std::filesystem::path p = std::filesystem::current_path() / file;
    std::string command = std::string("cmd /c start \"\" \"") + p.string() + "\"";
    int rc = std::system(command.c_str());
    if (rc != 0) {
        std::cerr << "edit: failed to open editor for " << p.string() << std::endl;
    }
}

int passthrough_shell_command(const std::string &command) {
    std::cout << colored("[shell] ", "1;30") << command << std::endl;
    return std::system(command.c_str());
}

void print_output_box(const std::vector<std::string> &lines) {
    size_t maxw = 0;
    for (const auto &line : lines) {
        if (line.size() > maxw) maxw = line.size();
    }

    const size_t padding = 1;
    std::string border = "+" + std::string(maxw + padding * 2, '-') + "+";
    std::cout << border << "\n";
    for (const auto &line : lines) {
        std::cout << "| " << line << std::string(maxw - line.size(), ' ') << " |\n";
    }
    std::cout << border << std::endl;
}

int run_code_file(PromptShell &shell, const std::string &mode, const std::string &run_file_path, bool force_debug = false) {
    // Resolve the run file path relative to cwd and normalize MSYS-style paths
    std::filesystem::path rp = normalize_user_path(run_file_path);
    if (!rp.is_absolute()) rp = std::filesystem::current_path() / rp;
    std::string abs_path = rp.string();

    std::string code = read_file_to_string(abs_path);
    if (code.empty()) {
        std::cerr << "Failed to open or read run file: " << abs_path << std::endl;
        return 1;
    }

    // Temporary working directory per file (hash the absolute path)
    std::filesystem::path tmp = std::filesystem::temp_directory_path() / "promptshell_run";
    tmp += std::to_string((int)std::hash<std::string>{}(abs_path));
    CodeRunner runner(tmp.string());
    RunResult rr = runner.compile_and_run("main.cpp", code, 5);

    if (rr.compile_ok && rr.run_ok) {
        std::string prog_out = rr.run_output;

        // Trim trailing newlines
        while (!prog_out.empty() && (prog_out.back() == '\n' || prog_out.back() == '\r')) prog_out.pop_back();

        // Split into lines
        std::vector<std::string> lines;
        std::istringstream lsin(prog_out);
        std::string ln;
        size_t maxw = 0;
        while (std::getline(lsin, ln)) {
            // remove trailing CR if present
            if (!ln.empty() && ln.back() == '\r') ln.pop_back();
            lines.push_back(ln);
            if (ln.size() > maxw) maxw = ln.size();
        }

        if (lines.empty()) {
            lines.push_back("");
        }

        print_output_box(lines);
        return 0;
    }

    // If user explicitly asked for debug, forward to the LLM. Otherwise return non-zero
    if (!force_debug) {
        std::cerr << "Run failed. Use 'debug <file>' to ask the assistant for help." << std::endl;
        return 2;
    }

    // Show compiler output first before asking the model
    std::cout << "--- Compiler Output ---\n";
    if (!rr.compile_output.empty()) {
        std::cout << rr.compile_output << "\n";
    } else {
        std::cout << "(no compiler errors)\n";
    }
    if (!rr.run_output.empty()) {
        std::cout << "--- Runtime Output ---\n" << rr.run_output << "\n";
    }
    std::cout << "\n--- Explanation ---\n\n" << std::flush;

    // Build debug prompt that hints at educational format
    std::string user_debug_prompt =
        "This C++ code has a compiler error.\n\n"
        "CODE:\n" + code + "\n\n"
        "ERROR:\n" + rr.compile_output + "\n";
    
    int debug_rc = shell.run(user_debug_prompt, mode, std::string("debug"));
    std::cout << std::flush;
    return debug_rc;
}

std::string cwd_string() {
    return std::filesystem::current_path().string();
}

std::string git_branch_string() {
    try {
        std::string cmd = "git -C \"" + cwd_string() + "\" branch --show-current 2> NUL";
        FILE* pipe = _popen(cmd.c_str(), "r");
        if (!pipe) return "";

        std::string output;
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe)) {
            output += buffer;
        }
        _pclose(pipe);

        output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
        output.erase(std::remove(output.begin(), output.end(), '\r'), output.end());
        return output;
    } catch (...) {
        return "";
    }
}

bool is_git_repo() {
    try {
        std::string cmd = "git -C \"" + cwd_string() + "\" rev-parse --is-inside-work-tree 2> NUL";
        FILE* pipe = _popen(cmd.c_str(), "r");
        if (!pipe) return false;

        std::string output;
        char buffer[64];
        while (fgets(buffer, sizeof(buffer), pipe)) {
            output += buffer;
        }
        _pclose(pipe);

        output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
        output.erase(std::remove(output.begin(), output.end(), '\r'), output.end());
        return output == "true";
    } catch (...) {
        return false;
    }
}

std::string compact_cwd_label() {
    std::string path = cwd_string();
    std::replace(path.begin(), path.end(), '\\', '/');

    if (path.size() >= 3 && std::isalpha(static_cast<unsigned char>(path[0])) && path[1] == ':' && path[2] == '/') {
        char drive = static_cast<char>(std::tolower(static_cast<unsigned char>(path[0])));
        path = std::string(1, drive) + path.substr(2);
    } else if (path.size() >= 3 && path[0] == '/' && std::isalpha(static_cast<unsigned char>(path[1])) && path[2] == '/') {
        char drive = static_cast<char>(std::tolower(static_cast<unsigned char>(path[1])));
        path = std::string(1, drive) + path.substr(2);
    }

    while (!path.empty() && path.front() == '/') {
        path.erase(path.begin());
    }

    return path;
}

std::string colored(const std::string &text, const std::string &code) {
    return "\033[" + code + "m" + text + "\033[0m";
}

void print_banner() {
    std::cout << colored("===========================================================", "1;35") << "\n";
    std::cout << colored("PromptShell - Local AI Coding Assistant", "1;36") << "\n";
    std::cout << colored("Type 'exit' / 'deactivate' to leave the session.", "1;33") << "\n";
    std::cout << colored("Commands: pwd | cd <path> | ls | touch <file> | mkdir <dir> | cat <file>", "1;37") << "\n";
    std::cout << colored("           edit <file> | open <file> | run <file> | debug <file>", "1;37") << "\n";
    std::cout << colored("           shell <command> | ask <prompt> | ai <prompt> | chat <prompt> | help", "1;37") << "\n";
    std::cout << colored("===========================================================", "1;35") << "\n";
}

void print_repl_help() {
    std::cout << colored("PromptShell commands", "1;36") << "\n";
    std::cout << colored("  pwd", "1;32") << " - show the current directory\n";
    std::cout << colored("  cd <path>", "1;32") << " - change directory\n";
    std::cout << colored("  ls", "1;32") << " - list files in the current directory\n";
    std::cout << colored("  touch <file>", "1;32") << " - create an empty file\n";
    std::cout << colored("  mkdir <dir>", "1;32") << " - create a directory\n";
    std::cout << colored("  cat <file>", "1;32") << " - print file contents\n";
    std::cout << colored("  edit <file>", "1;32") << " - open a file in the default editor\n";
    std::cout << colored("  open <file>", "1;32") << " - same as edit\n";
    std::cout << colored("  run <file>", "1;32") << " - compile and run a C++ file\n";
    std::cout << colored("  debug <file>", "1;32") << " - compile/run and ask PromptShell to explain errors\n";
    std::cout << colored("  shell <command>", "1;32") << " - pass a command to the system shell\n";
    std::cout << colored("  ask <prompt>", "1;32") << " - ask PromptShell an AI question\n";
    std::cout << colored("  ai <prompt>", "1;32") << " - same as ask\n";
    std::cout << colored("  chat <prompt>", "1;32") << " - same as ask\n";
    std::cout << colored("  help", "1;32") << " - show this help text\n";
    std::cout << colored("  deactivate / exit", "1;32") << " - leave PromptShell\n";
}

bool change_directory(const std::string &target) {
    try {
        std::filesystem::path next = normalize_user_path(target);
        if (!next.is_absolute()) {
            next = std::filesystem::current_path() / next;
        }
        if (!std::filesystem::exists(next) || !std::filesystem::is_directory(next)) {
            std::cerr << "cd: no such directory: " << next.string() << std::endl;
            return false;
        }
        std::filesystem::current_path(next);
        return true;
    } catch (const std::exception &ex) {
        std::cerr << "cd: " << ex.what() << std::endl;
        return false;
    }
}

void list_directory() {
    try {
        for (const auto &entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
            std::cout << entry.path().filename().string();
            if (entry.is_directory()) std::cout << "/";
            std::cout << "  ";
        }
        std::cout << std::endl;
    } catch (const std::exception &ex) {
        std::cerr << "ls: " << ex.what() << std::endl;
    }
}

void run_interactive_session(PromptShell &shell, const std::string &mode, const std::string &assistant_mode) {
    print_banner();

    std::string prompt;
    while (true) {
        std::cout << colored("Current Directory: ", "1;34") << colored(compact_cwd_label(), "1;32");
        std::string branch;
        if (is_git_repo()) {
            branch = git_branch_string();
        }
        if (!branch.empty()) {
            std::cout << colored("  [git: ", "1;30") << colored(branch, "1;33") << colored("]", "1;30");
        }
        std::cout << "\n" << colored("PromptShell>", "1;36") << " ";
        if (!std::getline(std::cin, prompt)) break;

        std::string trimmed = recover_missing_first_letter(trim_copy(prompt));

        if (trimmed.empty()) continue;
        if (trimmed == "exit" || trimmed == "quit" || trimmed == ":q" || trimmed == "deactivate") break;

        if (trimmed == "help" || trimmed == "?") {
            print_repl_help();
            continue;
        }

        if (starts_with(trimmed, "ask ") || starts_with(trimmed, "ai ") || starts_with(trimmed, "chat ")) {
            std::string prompt_text;
            if (starts_with(trimmed, "ask ")) prompt_text = trimmed.substr(4);
            else if (starts_with(trimmed, "ai ")) prompt_text = trimmed.substr(3);
            else prompt_text = trimmed.substr(5);

            prompt_text.erase(prompt_text.begin(), std::find_if(prompt_text.begin(), prompt_text.end(), [](unsigned char ch){ return !std::isspace(ch); }));
            prompt_text.erase(std::find_if(prompt_text.rbegin(), prompt_text.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), prompt_text.end());

            if (prompt_text.empty()) {
                std::cerr << "ask/ai/chat: missing prompt" << std::endl;
                continue;
            }

            shell.run(prompt_text, mode, assistant_mode);
            std::cout << "\n";
            continue;
        }

        if (trimmed == "pwd") {
            std::cout << colored("Current Directory: ", "1;34") << colored(compact_cwd_label(), "1;32");
            if (!branch.empty()) {
                std::cout << colored("  [git: ", "1;30") << colored(branch, "1;33") << colored("]", "1;30");
            }
            std::cout << std::endl;
            continue;
        }

        if (trimmed == "ls") {
            list_directory();
            continue;
        }

        if (starts_with(trimmed, "touch ")) {
            std::string args = trimmed.substr(6);
            handle_touch(args);
            continue;
        }

        if (starts_with(trimmed, "mkdir ")) {
            std::string args = trimmed.substr(6);
            handle_mkdir(args);
            continue;
        }

        if (starts_with(trimmed, "cat ")) {
            std::string args = trimmed.substr(4);
            handle_cat(args);
            continue;
        }

        if (starts_with(trimmed, "edit ") || starts_with(trimmed, "open ") || starts_with(trimmed, "gedit ")) {
            std::string args;
            if (starts_with(trimmed, "edit ")) args = trimmed.substr(5);
            else if (starts_with(trimmed, "open ")) args = trimmed.substr(5);
            else args = trimmed.substr(6);
            args.erase(args.begin(), std::find_if(args.begin(), args.end(), [](unsigned char ch){ return !std::isspace(ch); }));
            handle_edit(args);
            continue;
        }

        if (starts_with(trimmed, "shell ")) {
            std::string args = trimmed.substr(6);
            args.erase(args.begin(), std::find_if(args.begin(), args.end(), [](unsigned char ch){ return !std::isspace(ch); }));
            if (args.empty()) {
                std::cerr << "shell: missing command" << std::endl;
                continue;
            }
            passthrough_shell_command(args);
            continue;
        }

        if (starts_with(trimmed, "run ") || starts_with(trimmed, "debug ")) {
            bool debug_command = starts_with(trimmed, "debug ");
            std::string file = trimmed.substr(debug_command ? 6 : 4);
            file.erase(file.begin(), std::find_if(file.begin(), file.end(), [](unsigned char ch){ return !std::isspace(ch); }));
            file.erase(std::find_if(file.rbegin(), file.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), file.end());
            if (file.empty()) {
                std::cerr << (debug_command ? "debug" : "run") << ": missing file path" << std::endl;
                continue;
            }
            run_code_file(shell, mode, file, debug_command);
            continue;
        }

        if (starts_with(trimmed, "cd")) {
            std::string target;
            if (trimmed.size() > 2) {
                target = trimmed.substr(2);
                target.erase(target.begin(), std::find_if(target.begin(), target.end(), [](unsigned char ch){ return !std::isspace(ch); }));
                target.erase(std::find_if(target.rbegin(), target.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), target.end());
            }
            change_directory(target);
            continue;
        }

        std::cerr << "Command not found: " << trimmed << "\n";
        std::cerr << "Type 'help' for available PromptShell commands.\n";
        continue;
    }
}

} // namespace

int main(int argc, char** argv) {
    AppConfig config = parse_args(argc, argv);

    if (config.show_help) {
        print_help();
        return 0;
    }

    // Paths (keep same defaults)
    std::string engine_path = "E:/PromptShell/llama-cpp/build/bin/llama-cli.exe";
    std::string model_path = "E:/PromptShell/models/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf";
    std::string out_file = "E:/PromptShell/backend/build/llm_output.txt";

    PromptShell shell(engine_path, model_path, out_file);

    if (!config.run_file.empty()) {
        return run_code_file(shell, config.mode, config.run_file, false);
    }

    if (config.interactive) {
        run_interactive_session(shell, config.mode, config.assistant_mode);
        return 0;
    }

    if (!config.prompt.empty()) {
        std::cout << "\n[Processing...]\n\n";
        return shell.run(config.prompt, config.mode, config.assistant_mode);
    }

    std::cout << "No prompt provided. Use --help for usage.\n";
    return 1;
}

