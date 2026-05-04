#pragma once
#include <string>

struct RunResult {
    bool compile_ok = false;
    std::string compile_output;
    bool run_ok = false;
    int exit_code = -1;
    std::string run_output;
};

class CodeRunner {
public:
    explicit CodeRunner(const std::string &workdir);
    // write code to fileName (e.g., "main.cpp"), compile and run, returns RunResult
    RunResult compile_and_run(const std::string &fileName, const std::string &code, int timeout_seconds = 5);
private:
    std::string workdir_;
};
