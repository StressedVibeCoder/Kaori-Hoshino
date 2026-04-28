#pragma once

#include <string>
#include <vector>
#include "../ollama_client.hpp"
#include "memory.hpp"
#include <iostream>

class Extractor {
public:
    void extract(const std::string& user_message, Memory& memory, OllamaClient& client) {
        const std::string prompt =
            "Extract information from this message. Reply ONLY with JSON, no explanation, no markdown.\n"
            "JSON format: {\"name\": \"\", \"facts\": []}\n"
            "Rules:\n"
            "- name: first name of the person speaking, or empty string if not mentioned\n"
            "- facts: ONLY facts explicitly stated in the message (hobbies, projects, age, job)\n"
            "- DO NOT invent or assume anything not explicitly written\n"
            "- if nothing relevant, return {\"name\": \"\", \"facts\": []}\n"
            "Message: \"" + user_message + "\"";

        try {
            std::vector<Message> msgs;
            msgs.push_back({ "user", prompt });

            std::string raw = client.chat(msgs);

            auto start = raw.find('{');
            auto end   = raw.rfind('}');
            if (start == std::string::npos || end == std::string::npos) return;
            raw = raw.substr(start, end - start + 1);

            auto j = json::parse(raw);

            std::string name = j.value("name", "");
            if (!name.empty() && memory.name() != name)
                memory.set_name(name);

            auto facts  = j.value("facts", std::vector<std::string>{});
            bool changed = false;

            for (const auto& fact : facts) {
                if (fact.empty() || fact.size() < 4)             continue;
                if (fact.find("nao ha")    != std::string::npos) continue;
                if (fact.find("nenhum")    != std::string::npos) continue;
                if (fact.find("N/A")       != std::string::npos) continue;
                if (fact.find("relevante") != std::string::npos) continue;

                bool duplicate = false;
                for (const auto& f : memory.facts())
                    if (f == fact) { duplicate = true; break; }

                if (!duplicate) {
                    memory.add_fact(fact);
                    changed = true;
                }
            }

            if (!name.empty() || changed)
                memory.save();

        } catch (...) {}
    }
};