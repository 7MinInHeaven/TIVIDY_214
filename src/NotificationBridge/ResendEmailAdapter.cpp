#include "ResendEmailAdapter.h"
#include "MailjetHttpClient.h" 
#include <nlohmann/json.hpp>

#include <curl/curl.h>

namespace {
size_t writeCb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* out = static_cast<std::string*>(userdata);
    out->append(ptr, size * nmemb);
    return size * nmemb;
}
}

using json = nlohmann::json;

ResendEmailAdapter::ResendEmailAdapter(std::string apiKey, std::string fromEmail)
    : apiKey_(std::move(apiKey)), fromEmail_(std::move(fromEmail)) {}

bool ResendEmailAdapter::sendEmail(std::string to, std::string subject, std::string body) {
    lastError_.clear();

    // Resend v1 Payload
    json payload = {
        {"from", fromEmail_},
        {"to", json::array({to})},
        {"subject", subject},
        {"text", body}
    };

    // Resend uses Bearer token auth, not Basic Auth
    
    
    // Note: You may need to adapt your MailjetHttpClient to accept custom headers
    // for the Bearer token. For now, let's assume it supports it or we use a raw curl call.
    // A simplified version:
    
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey_).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    // Save the dump to a variable so it doesn't get destroyed instantly
    std::string jsonPayload = payload.dump();

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.resend.com/emails");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str()); // Use the variable here!
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res == CURLE_OK && (http_code == 200 || http_code == 201)) {
        return true;
    }
    lastError_ = "Resend HTTP " + std::to_string(http_code) + ": " + readBuffer;
    return false;
}