#ifndef SMS_H
#define SMS_H

#include <string> 
#include <iostream> 
using namespace std;

class SmsTransport {

public: 
    virtual ~SmsTransport(); 
    virtual bool sendSms(const string& toPhone, const string& body) = 0;


};

class ConsoleSmsAdapter : public SmsTransport { 
     bool sendSms(const string& toPhone, const string& body) override {
        cout << "\n--- SIMULATED SMS ---\n"
                  << "To: " << toPhone << "\n"
                  << "Body: " << body << "\n"
                  << "----------------------\n";
        return true;
    }
};

#endif //SMS_H