#pragma once
#include <string>

class LLMEngine {
public:
    LLMEngine(const std::string &enginePath, const std::string &modelPath, const std::string &outFile);
    int run_prompt(const std::string &full_prompt);
    std::string read_output();

private:
    std::string engine_path_;
    std::string model_path_;
    std::string out_file_;
};
