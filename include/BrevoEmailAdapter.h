#ifndef BREVOEMAILADAPTER_H
#define BREVOEMAILADAPTER_H

#include "NotificationBridge.h"
#include <string>

class BrevoEmailAdapter : public EmailTransport {
private:
    std::string apiKey_;
    std::string fromEmail_;
    std::string lastError_;

public:
    BrevoEmailAdapter(std::string apiKey, std::string fromEmail);
    bool sendEmail(std::string to, std::string subject, std::string body) override;
    std::string lastError() const { return lastError_; }
};

#endif