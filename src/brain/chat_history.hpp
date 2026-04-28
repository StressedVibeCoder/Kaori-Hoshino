#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <nlohmann/json.hpp>
#include "memory.hpp"

namespace fs = std::filesystem;

struct ChatMessage {
    std::string role;
    std::string content;
    std::string timestamp;
};

class ChatHistory {
public:
    explicit ChatHistory(const std::string& user_name = Memory().name());

    void add_message(const std::string& role, const std::string& content);
    void save() const;
    std::vector<ChatMessage> load_today() const;

private:
    std::string m_user_name;
    std::string m_today_dir;
    std::string m_session_file;
    std::vector<ChatMessage> m_messages;

    static std::string get_today_str();
    static std::string get_timestamp();
};

ChatHistory::ChatHistory(const std::string& user_name)
: m_user_name(user_name.empty() ? Memory().name() : user_name)
{
    auto today = get_today_str();
    m_today_dir = "chats/" + today;

    fs::create_directories(m_today_dir);

    // Nome do arquivo da sessão: kaori_YYYY-MM-DD_HH-MM-SS.json
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t), "%H-%M-%S");
    m_session_file = m_today_dir + "/kaori_" + today + "_" + oss.str() + ".json";

    load_today();
}

void ChatHistory::add_message(const std::string& role, const std::string& content) {
    m_messages.push_back({role, content, get_timestamp()});
}

void ChatHistory::save() const {
    if (m_messages.empty()) return;

    json j;
    j["user"] = m_user_name;
    j["date"] = get_today_str();
    j["session_file"] = m_session_file;

    json msgs = json::array();
    for (const auto& msg : m_messages) {
        msgs.push_back({
            {"role", msg.role},
            {"content", msg.content},
            {"timestamp", msg.timestamp}
        });
    }
    j["messages"] = msgs;

    std::ofstream file(m_session_file);
    if (file.is_open()) {
        file << j.dump(2);
    }
}

std::string ChatHistory::get_today_str() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t), "%Y-%m-%d");
    return oss.str();
}

std::string ChatHistory::get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::vector<ChatMessage> ChatHistory::load_today() const {
    return m_messages;
}
