#include "ollama_client.hpp"

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

OllamaClient::OllamaClient(const std::string& model, const std::string& base_url)
    : m_model(model), m_base_url(base_url)
{}

size_t OllamaClient::write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* buffer = static_cast<std::string*>(userdata);
    buffer->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string OllamaClient::chat(const std::vector<Message>& messages) {
    json payload;
    payload["model"]  = m_model;
    payload["stream"] = false;

    json msgs = json::array();
    for (const auto& msg : messages)
        msgs.push_back({ {"role", msg.role}, {"content", msg.content} });
    payload["messages"] = msgs;

    const std::string body = payload.dump();
    const std::string url  = m_base_url + "/api/chat";

    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Falha ao inicializar curl");

    std::string response_buffer;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL,           url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,    body.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,    headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &response_buffer);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        throw std::runtime_error(std::string("curl falhou: ") + curl_easy_strerror(res));

    try {
        auto parsed = json::parse(response_buffer);
        return parsed["message"]["content"].get<std::string>();
    } catch (const json::exception& e) {
        throw std::runtime_error(std::string("Falha ao parsear resposta: ") + e.what());
    }
}