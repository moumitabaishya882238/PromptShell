#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

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
        return "You are a C++ code debugger. For every code error, you MUST follow this exact format:\n\n"
               "1. A paragraph starting with 'These type of errors are due to...' explaining the conceptual reason for the error.\n"
               "2. A sentence like 'In your case, look around near line [X]...' (identify the line number from the compiler output if possible).\n"
               "3. A section starting with 'Here is an example of one of your use cases type:' followed by a short snippet showing the pattern.\n"
               "4. The complete corrected code block.\n\n"
               "Be professional, educational, and follow this sequence strictly.";
    }
    std::string postprocess(const std::string &reply) const override {
        return reply;
    }
};
