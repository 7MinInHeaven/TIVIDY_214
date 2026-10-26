#ifndef MAILJETEMAILADAPTER_H
#define MAILJETEMAILADAPTER_H

#include "NotificationBridge.h" 
#include <string>
using namespace std;

class MailjetEmailAdapter : public EmailTransport {

    private:
      string apiKey_;
      string apiSecret_;
      string fromEmail_;
      string fromName_;
    bool sandboxMode_;
      string lastError_;

public:
    MailjetEmailAdapter(  string apiKey,
                          string apiSecret,
                          string fromEmail,
                          string fromName = "TIVIDY",
                        bool sandboxMode = false);

    bool sendEmail(  string to, 
                     string subject, 
                     string body) override;

    
      string lastError() const { return lastError_; }


};

#endif // MAILJETEMAILADAPTER_H