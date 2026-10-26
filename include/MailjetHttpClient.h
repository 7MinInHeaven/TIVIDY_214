#ifndef MAILJETHTTPCLIENT_H
#define MAILJETHTTPCLIENT_H

#include <string>

struct HttpResponse { 
    long status = 0; 
    std::string body; 
    std::string error; 
}; 

class MailjetHttpClient { 
private: 
    std::string apiKey_; 
    std::string apiSecret_;

public: 
    
    MailjetHttpClient(std::string apiKey, std::string apiSecret);

    HttpResponse postJson(const std::string& url, const std::string& json);
};

#endif // MAILJETHTTPCLIENT_H