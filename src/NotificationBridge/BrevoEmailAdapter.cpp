#include "BrevoEmailAdapter.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

BrevoEmailAdapter::BrevoEmailAdapter(std::string apiKey, std::string fromEmail)
    : apiKey_(std::move(apiKey)), fromEmail_(std::move(fromEmail)) {}

bool BrevoEmailAdapter::sendEmail(std::string to, std::string subject, std::string body) {
    lastError_.clear();

    // Brevo v3 Payload
    json payload = {
        {"sender", {{"email", fromEmail_}}},
        {"to", json::array({{{"email", to}}})},
        {"subject", subject},
        {"textContent", body}
    };

    CURL* curl = curl_easy_init();
    std::string readBuffer;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("api-key: " + apiKey_).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.brevo.com/v3/smtp/email");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.dump().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res == CURLE_OK && http_code == 201) { // Brevo returns 201 Created
        return true;
    }
    lastError_ = "Brevo HTTP " + std::to_string(http_code) + ": " + readBuffer;
    return false;
}