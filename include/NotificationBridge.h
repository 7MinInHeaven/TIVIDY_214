#ifndef NOTIFICATIONBRIDGE_H 
#define NOTIFICATIONBRIDGE_H 



#include <string>
#include <iostream>
#include <memory>
using namespace std; 

class NotificationChannel;
//=============ABSTRACTION====================//
class Notification {
protected: 
    NotificationChannel* channel; 
    //Participant* recipient;
    string recipient; //For now

public: 
     Notification(NotificationChannel* ch, string rec)
      : channel(ch), recipient(rec) {};
    virtual void send()= 0; 
    virtual string compose() = 0;

    virtual ~Notification() = default; 

};

//===============REFINED_ABSTRACTION===========//

class ResultNotice : public Notification {
public: 
    ResultNotice(NotificationChannel* ch, string rec)
     : Notification(ch, rec) {}
    string compose() override;
    void send() override;
};

class DeadlineNotice : public Notification {
public: 
    DeadlineNotice(NotificationChannel* ch, string rec)
     : Notification(ch, rec) {}
    void send() override; 
    string compose() override; 
};


//============IMPLEMENTOR==============//

class NotificationChannel {
public: 
    virtual ~NotificationChannel() = default;
    virtual void deliver(string to, string text)=0;
};

//============CONCRETE_IMPLEMENTOR==========//
class EmailTransport;
class EmailChannel : public NotificationChannel{
private: 
    unique_ptr<EmailTransport> transport; 

public: 
    EmailChannel(EmailTransport* t);
    void deliver(string to, string text) override; 
    ~EmailChannel() override;
};

class AppChannel : public NotificationChannel{
public: 
    void deliver(string to, string text) override; 
};

//=======================================//
//======ADAPTER==========================//
//=======================================//

//===============TARGET===================//


class EmailTransport{
public: 
    virtual ~EmailTransport() = default;
    virtual bool sendEmail(string to, string subject, string body)=0; 
};

class ConsoleEmailAdapter : public EmailTransport {
private: 
    ostream& out; 

public: 
    ConsoleEmailAdapter(ostream& o) : out(o){}; 
    bool sendEmail(string to, string subject, string body) override;

};

#endif //NOTIFICATIONBRIDGE_H