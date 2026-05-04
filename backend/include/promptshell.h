#pragma once
#include <string>
#include "assistant_mode.h"

class PromptShell {
public:
    PromptShell(const std::string &enginePath, const std::string &modelPath, const std::string &outFile);
    int run(const std::string &user_prompt, const std::string &output_mode, const std::string &assistant_mode_name="default");

private:
    std::string system_prompt_;
    std::string engine_path_;
    std::string model_path_;
    std::string out_file_;

    std::string build_full_prompt(const std::string &user_prompt) const;
    std::string extract_assistant_reply(const std::string &raw) const;
    std::string escape_json(const std::string &s) const;
};
