#include "SmsAdapter.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <utility>

using json = nlohmann::json; 


namespace {
size_t writeCb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* out = static_cast<std::string*>(userdata);
    out->append(ptr, size * nmemb);
    return size * nmemb;
}
}

SmsAdapter::SmsAdapter(std::string id, std::string secret, bool testMode)
    : clientId_(std::move(id)), apiSecret_(std::move(secret)), testMode_(testMode) {}



    bool SmsPortalAdapter::sendSms(const std::string& toPhone, const std::string& body) {
    lastError_.clear();

    // 1. Build the SMSPortal JSON Payload
    json payload = {
        {"messages", json::array({
            {
                {"content", body},
                {"destination", toPhone} 
            }
        })}
    };

    if (testMode_) {
        payload["testMode"] = true; 
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        lastError_ = "curl init failed";
        return false;
    }

    std::string readBuffer;
    std::string userpwd = clientId_ + ":" + apiSecret_;
    
    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, "https://rest.smsportal.com/v3/BulkMessages");
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd.c_str()); // Basic Auth
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    std::string jsonStr = payload.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // SMSPortal returns 200 OK on success
    if (res == CURLE_OK && http_code == 200) {
        return true;
    }

    lastError_ = "SMSPortal HTTP " + std::to_string(http_code) + ": " + readBuffer;
    return false;
}