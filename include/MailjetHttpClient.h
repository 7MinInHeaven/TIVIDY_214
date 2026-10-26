#ifndef MAILJETHTTPCLIENT_H
#define MAILJETHTTPCLIENT_H

#include <string>
using namespace std;

struct HttpResponse { 
    long status = 0; 
    string body; 
    string error; 
}; 

class MailjetHttpClient { 
private: 
    string apiKey; 
    string apiSecret;

public: 
    MailjetHttpClient(string apiKey, string apiSecret): 
    apiKey(this->apiKey), apiSecret(this->apiSecret){}

    HttpResponse postJson(const string& url, const string& json);
};

#endif