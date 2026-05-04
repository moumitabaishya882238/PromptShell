#include "llm_engine.h"
#include <cstdlib>
#include <fstream>
#include <sstream>

LLMEngine::LLMEngine(const std::string &enginePath, const std::string &modelPath, const std::string &outFile)
    : engine_path_(enginePath), model_path_(modelPath), out_file_(outFile) {}

int LLMEngine::run_prompt(const std::string &full_prompt) {
    std::string command = engine_path_ + " -m \"" + model_path_ + "\" -p \"" + full_prompt + "\"";
    std::string cmd_with_redirect = command + " > \"" + out_file_ + "\" 2>&1";
    return system(cmd_with_redirect.c_str());
}

std::string LLMEngine::read_output() {
    std::ifstream ifs(out_file_, std::ios::in | std::ios::binary);
    std::ostringstream oss;
    oss << ifs.rdbuf();
    return oss.str();
}
