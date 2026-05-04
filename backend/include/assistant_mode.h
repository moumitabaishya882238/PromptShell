#pragma once
#include <string>

class AssistantMode {
public:
    virtual ~AssistantMode() = default;
    // Provide additional instructions to prepend to the system prompt
    virtual std::string system_instructions() const = 0;
    // Optionally post-process the assistant reply (e.g., strip commentary)
    virtual std::string postprocess(const std::string &reply) const { return reply; }
};

class DefaultMode : public AssistantMode {
public:
    std::string system_instructions() const override {
        return ""; // no extra instructions
    }
};

class CodeOnlyMode : public AssistantMode {
public:
    std::string system_instructions() const override {
        return "When appropriate, respond with code-only answers: give the minimal working C++ code example and a one-line explanation.";
    }
    std::string postprocess(const std::string &reply) const override {
        return reply; // could trim non-code later
    }
};

class ConciseMode : public AssistantMode {
public:
    std::string system_instructions() const override {
        return "Be extremely concise: one short paragraph and a tiny code snippet when needed.";
    }
};

class DebugMode : public AssistantMode {
public:
    std::string system_instructions() const override {
        return "Act as a C++ debugger and teacher. When given user code and compiler/runtime output,\n"
               "1) Provide a numbered annotated listing indicating error/important lines.\n"
               "2) For each problem, give a concise root-cause explanation and severity.\n"
               "3) Provide a minimal corrected code listing that fixes the problem.\n"
               "4) Give a short concept explanation why the fix works and next tests to run.";
    }
    std::string postprocess(const std::string &reply) const override {
        return reply;
    }
};
