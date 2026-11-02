/**
 * @file BrevoEmailAdapter.h
 * @brief Adapter for the Brevo (formerly Sendinblue) API.
 */
#ifndef BREVOEMAILADAPTER_H
#define BREVOEMAILADAPTER_H

#include "NotificationBridge.h"
#include <string>

/**
 * @class BrevoEmailAdapter
 * @brief A Concrete Strategy/Adapter that sends emails via the Brevo REST API.
 * 
 * Implements the EmailTransport interface. Uses libcurl to make HTTP requests
 * and nlohmann::json to format the payload according to Brevo's v3 API specifications.
 */
class BrevoEmailAdapter : public EmailTransport {
private:
    std::string apiKey_;
    std::string fromEmail_;
    std::string lastError_;

public:
    BrevoEmailAdapter(std::string apiKey, std::string fromEmail);
    
    /**
     * @brief Sends an email via Brevo.
     * @return True if HTTP 201 Created is received, false otherwise.
     */
    bool sendEmail(std::string to, std::string subject, std::string body) override;
    
    std::string lastError() const { return lastError_; }
};

#endif