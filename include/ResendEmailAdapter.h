/**
 * @file ResendEmailAdapter.h
 * @brief Adapter for the Resend API.
 */
#ifndef RESENDEMAILADAPTER_H
#define RESENDEMAILADAPTER_H

#include "NotificationBridge.h"
#include <string>

/**
 * @class ResendEmailAdapter
 * @brief A Concrete Strategy/Adapter that sends emails via the Resend REST API.
 * 
 * Implements the EmailTransport interface. Handles bearer token authentication
 * and translates the payload into Resend's expected JSON format.
 */
class ResendEmailAdapter : public EmailTransport {
private:
    std::string apiKey_;
    std::string fromEmail_;
    std::string lastError_;

public:
    ResendEmailAdapter(std::string apiKey, std::string fromEmail);
    
    /**
     * @brief Sends an email via Resend.
     * @return True if HTTP 200 or 201 is received, false otherwise.
     */
    bool sendEmail(std::string to, std::string subject, std::string body) override;
    
    std::string lastError() const { return lastError_; }
};

#endif