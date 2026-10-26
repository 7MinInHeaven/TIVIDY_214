#ifndef SMSADAPTER_H
#define SMSADAPTER_H 

#include "SMS.h"
#include <string> 
using namespace std;

class SmsAdapter : public SmsTransport { 
private: 
    string clientId_; 
    string apiSecret_;
    bool testMode_;
    string lastError_;


    public:
    SmsAdapter(string clientId, string apiSecret, bool testMode = true); 

    bool sendSms(const string& toPhone, const string& body) override; 
    string lastError() const{return lastError_;}


};



#endif 