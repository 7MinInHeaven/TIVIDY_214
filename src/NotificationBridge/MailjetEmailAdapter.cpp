#include "MailjetEmailAdapter.h"
#include "MailjetHttpClient.h"
#include <nlohmann/json.hpp>
#include <utility>

using json = nlohmann::json;

MailjetEmailAdapter::MailjetEmailAdapter(std::string key,
                                         std::string secret,
                                         std::string from,
                                         std::string name,
                                         bool sandbox)
    : apiKey_(std::move(key))
    , apiSecret_(std::move(secret))
    , fromEmail_(std::move(from))
    , fromName_(std::move(name))
    , sandboxMode_(sandbox) {}


bool MailjetEmailAdapter::sendEmail(std::string to, std::string subject, std::string body) {
    lastError_.clear();

    // 1. Build the Mailjet JSON Payload
    json message = {
        {"From",     {{"Email", fromEmail_}, {"Name", fromName_}}},
        {"To",       json::array({{{"Email", to}}})},
        {"Subject",  subject},
        {"TextPart", body}
    };

    json payload = {{"Messages", json::array({message})}};
    
    // SandboxMode validates credentials/JSON without actually sending an email!
    if (sandboxMode_) {
        payload["SandboxMode"] = true;
    }

    // 2. Send HTTP Request
    MailjetHttpClient client(apiKey_, apiSecret_);
    HttpResponse resp = client.postJson("https://api.mailjet.com/v3.1/send", payload.dump());

    if (!resp.error.empty()) {
        lastError_ = "Network error: " + resp.error;
        return false;
    }

    // 3. Parse Response
    auto parsed = json::parse(resp.body, nullptr, /*allow_exceptions=*/false);
    
    if (resp.status == 200 
        && !parsed.is_discarded()
        && parsed.contains("Messages")
        && !parsed["Messages"].empty()
        && parsed["Messages"][0].value("Status", "") == "success") {
        return true;
    }

    // Capture the exact error for debugging
    lastError_ = "Mailjet HTTP " + std::to_string(resp.status) + ": " + resp.body;
    return false;
}