#include <iostream>
#include <string>
#include <cstdlib>

int main() {
    std::string prompt;
    
    std::cout << "PromptShell - AI Coding Assistant (Phase 5: System Prompt)\n";
    std::cout << "===========================================================\n\n";
    
    std::cout << "Enter your prompt: ";
    std::getline(std::cin, prompt);
    
    if (prompt.empty()) {
        std::cout << "Error: Empty prompt. Exiting.\n";
        return 1;
    }
    
    std::cout << "\n[Processing...]\n\n";
    
    // Master/system prompt to guide the assistant
    std::string system_prompt = "You are PromptShell, a helpful C++ coding assistant focused on beginners. "
                                "Explain concepts clearly with simple examples. Keep answers concise and terminal-friendly.";

    // Build the llama-cli command by prefixing the system prompt to the user prompt
    // Some llama-cli builds accept a --system flag, but to be compatible we'll prepend the system prompt
    std::string engine_path = "E:/PromptShell/llama-cpp/build/bin/llama-cli.exe";
    std::string model_path = "E:/PromptShell/models/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf";

    std::string full_prompt = system_prompt + "\n\nUser: " + prompt;
    std::string command = engine_path + " -m \"" + model_path + "\" -p \"" + full_prompt + "\"";

    // Execute the command and capture response
    int result = system(command.c_str());
    
    if (result != 0) {
        std::cerr << "\nError: Failed to execute llama-cli (exit code: " << result << ")\n";
        return 1;
    }
    
    std::cout << "\n✓ Response generated successfully!\n";
    return 0;
}

