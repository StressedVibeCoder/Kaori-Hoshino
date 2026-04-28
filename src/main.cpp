#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

#include "ollama_client.hpp"
#include "personality.hpp"
#include "brain/memory.hpp"
#include "brain/extractor.hpp"
#include "brain/chat_history.hpp"

static std::string today() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&t));
    return buf;
}

int main() {
    const std::string model = "llama3.1:8b";

    OllamaClient client(model);
    Memory memory("kaori_memory.json");
    Extractor extractor;

    memory.increment_sessions();
    memory.set_last_seen(today());
    ChatHistory chat_history(memory.name());

    std::string system_prompt = SYSTEM_PROMPT;
    system_prompt += memory.context_block();

    std::vector<Message> history;
    history.push_back({ "system", system_prompt });

    std::cout << "=== Kaori AI ===\n";
    std::cout << "Sessão #" << memory.sessions() << "\n";
    if (!memory.name().empty())
std::cout << "Bem-vindo de volta, " << memory.name() << ".\n";
    std::cout << "Digite 'sair' para encerrar.\n\n";

    while (true) {
        std::cout << "Tu: ";
        std::string input;
        std::getline(std::cin, input);

        if (input.empty()) continue;
        if (input == "sair") break;

        history.push_back({ "user", input });

        try {
            std::string response = client.chat(history);
            history.push_back({ "assistant", response });
            std::cout << "\nKaori: " << response << "\n\n";

            // Extração usando o mesmo modelo principal para evitar inconsistências
            // causa lentidão, principalmente em pc fraco, mas é o mais simples e eficaz por enquanto
            extractor.extract(input, memory, client);
            chat_history.add_message("user", input);
            chat_history.add_message("assistant", response);

        } catch (const std::exception& e) {
            std::cerr << "[erro] " << e.what() << "\n";
            history.pop_back();
        }
    }
    chat_history.save();
    memory.save();
    std::cout << "Até logo!\n";
    return 0;
}
