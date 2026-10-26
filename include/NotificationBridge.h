#ifndef NOTIFICATIONBRIDGE_H 
#define NOTIFICATIONBRIDGE_H 



#include <string>
#include <iostream>
#include <memory>
#include "WorkItem.h"

#include "SMS.h"
using namespace std; 

//==============IMPLEMENTOR_DATA==========//
struct NotificationData{
    string to; 
    string subject; 
    string body;
};

class NotificationChannel {
public: 
    virtual ~NotificationChannel() = default;
    virtual void deliver(const NotificationData&)=0;
};

class EmailTransport{
public: 
    virtual ~EmailTransport() = default;
    virtual bool sendEmail(string to, string subject, string body)=0; 
};

//=============ABSTRACTION====================//
class Notification {
protected: 
    NotificationChannel* channel; 
    string recipient;
    WorkItem* context;
public: 
     Notification(NotificationChannel* ch, WorkItem* ctx, string to)
      : channel(ch), recipient(to), context(ctx) {}

    virtual void send(){
        if(channel) channel->deliver(compose());
    }

    virtual NotificationData compose() = 0;

    virtual ~Notification() = default; 

};

//===============REFINED_ABSTRACTION===========//

class ResultNotice : public Notification {
public: 
    ResultNotice(NotificationChannel* ch, WorkItem* ctx, string to)
     : Notification(ch, ctx, to) {}
    NotificationData compose() override;
    
};

class DeadlineNotice : public Notification {
public: 
    DeadlineNotice(NotificationChannel* ch, WorkItem* ctx, string to)
     : Notification(ch, ctx, to) {}
    
    NotificationData compose() override; 
};





//============CONCRETE_IMPLEMENTOR==========//


class EmailChannel : public NotificationChannel{
private: 
     EmailTransport* transport; 

public: 
    explicit EmailChannel(EmailTransport* t):transport(t){};
    void deliver(const NotificationData& d) override{
        if(transport) transport->sendEmail(d.to,d.subject,d.body);
    }
    
};

class SmsChannel : public NotificationChannel {
    SmsTransport* transport;
public:
    explicit SmsChannel(SmsTransport* t) : transport(t) {}

    void deliver(const NotificationData& d) override {
       
        if (transport) {
            if (!transport->sendSms(d.to, d.body)) {
                std::cerr << "[SMS Error] Failed to send SMS to " << d.to << "\n";
            }
        }
    }
};

class AppChannel : public NotificationChannel{
public: 
    void deliver(const NotificationData& d) override{
        cout << "\n[APP ALERT for " << d.to << "]\n"
                  << d.subject << "\n" << d.body << "\n"
                  << "-----------------------------------\n";
    }; 
};

//CONCRETE ADAPTER//

class ConsoleEmailAdapter : public EmailTransport {
private: 
    ostream& out; 

public: 
    ConsoleEmailAdapter(ostream& o) : out(o){}; 
    bool sendEmail(string to, string subject, string body) override;

};

#endif //NOTIFICATIONBRIDGE_H