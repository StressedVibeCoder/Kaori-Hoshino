#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct UserMemory {
    std::string name;
    std::vector<std::string> facts;
    int sessions = 0;
    std::string last_seen;
};

class Memory {
public:
    explicit Memory(const std::string& filepath = "kaori_memory.json")
        : m_filepath(filepath)
    {
        load();
    }

    ~Memory() {
        save();
    }

    std::string context_block() const {
        if (m_data.name.empty() && m_data.facts.empty()) return "";

        std::string block = "\nO QUE VOCÊ JÁ SABE SOBRE O UTILIZADOR:\n";

        if (!m_data.name.empty())
            block += "- O utilizador chama-se " + m_data.name + ".\n";

        for (const auto& fact : m_data.facts)
            block += "- " + fact + "\n";

        if (m_data.sessions > 0)
            block += "- Esta é a sessão número " + std::to_string(m_data.sessions + 1) + ".\n";

        return block;
    }

    const std::string& name()              const { return m_data.name; }
    const std::vector<std::string>& facts() const { return m_data.facts; }
    int sessions()                          const { return m_data.sessions; }

    void set_name(const std::string& name)      { m_data.name = name; }
    void add_fact(const std::string& fact)      { m_data.facts.push_back(fact); }
    void increment_sessions()                   { m_data.sessions++; }
    void set_last_seen(const std::string& date) { m_data.last_seen = date; }

    void save() const {
        std::ofstream file(m_filepath);
        if (!file.is_open()) {
            return;
        }
        json j;
        j["user"]["name"]  = m_data.name;
        j["user"]["facts"] = m_data.facts;
        j["sessions"]      = m_data.sessions;
        j["last_seen"]     = m_data.last_seen;
        file << j.dump(2);
    }

private:
    std::string m_filepath;
    UserMemory  m_data;

    void load() {
        std::ifstream file(m_filepath);
        if (!file.is_open()) return;

        try {
            json j = json::parse(file);
            m_data.name      = j.value("user", json{}).value("name", "");
            m_data.facts     = j.value("user", json{}).value("facts", std::vector<std::string>{});
            m_data.sessions  = j.value("sessions", 0);
            m_data.last_seen = j.value("last_seen", "");
        } catch (...) {}
    }
};
