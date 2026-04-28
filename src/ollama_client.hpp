#pragma once

#include <string>
#include <vector>

struct Message {
    std::string role;
    std::string content;
};

class OllamaClient {
public:
    explicit OllamaClient(const std::string& model, const std::string& base_url = "http://localhost:11434");

    std::string chat(const std::vector<Message>& messages);

private:
    std::string m_model;
    std::string m_base_url;

    static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata);
};