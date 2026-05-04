#include "coderunner.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>

CodeRunner::CodeRunner(const std::string &workdir) : workdir_(workdir) {
    std::filesystem::create_directories(workdir_);
}

static std::string read_file(const std::string &path) {
    std::ifstream ifs(path, std::ios::binary);
    std::ostringstream oss; oss << ifs.rdbuf();
    return oss.str();
}

RunResult CodeRunner::compile_and_run(const std::string &fileName, const std::string &code, int timeout_seconds) {
    RunResult res;
    std::string src = workdir_ + "/" + fileName;
    std::ofstream ofs(src, std::ios::binary);
    ofs << code;
    ofs.close();

    std::string compile_out = workdir_ + "/compile.txt";
    std::string bin = workdir_ + "/runprog";

    // Check which g++ will be used (write to workdir_)
    std::string which_out = workdir_ + "/which_gpp.txt";
    std::string which_cmd = "which g++ > \"" + which_out + "\" 2>&1";
    system(which_cmd.c_str());

    // Compile using g++ (must be in PATH via MSYS2)
    std::string compile_cmd = "g++ -std=c++17 -O0 -g -o \"" + bin + "\" \"" + src + "\" 2> \"" + compile_out + "\"";
    int c_rc = system(compile_cmd.c_str());
    std::string orig_compile_output = read_file(compile_out);
    // Determine which binary was produced. On Windows g++ will create an .exe.
    std::string exec_path = bin;
    if (!std::filesystem::exists(exec_path)) {
        std::string alt = bin + ".exe";
        if (std::filesystem::exists(alt)) exec_path = alt;
    }
    // Determine compile_ok from exit code and binary existence
    bool compile_ok = (std::filesystem::exists(exec_path) && c_rc == 0);
    res.compile_output = orig_compile_output;
    res.compile_ok = compile_ok;

    if (res.compile_ok) {
        std::string run_out = workdir_ + "/runout.txt";
        // Use the actual exec_path (may include .exe on Windows)
        // Use timeout command available in MSYS2 coreutils: timeout <s> <cmd>
        std::string run_cmd = "timeout " + std::to_string(timeout_seconds) + "s \"" + exec_path + "\" > \"" + run_out + "\" 2>&1";
        int r_rc = system(run_cmd.c_str());
        res.run_output = read_file(run_out);
        // Keep only the program's stdout/stderr in run_output
        res.run_ok = (r_rc == 0);
        // Try to decode exit code when possible
#ifdef WIFEXITED
        if (WIFEXITED(r_rc)) res.exit_code = WEXITSTATUS(r_rc);
        else res.exit_code = r_rc;
#else
        res.exit_code = r_rc;
#endif
    } else {
        res.run_ok = false;
        res.run_output.clear();
        res.exit_code = -1;
    }

    return res;
}
