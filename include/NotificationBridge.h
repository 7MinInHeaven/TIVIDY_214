#ifndef NOTIFICATIONBRIDGE_H 
#define NOTIFICATIONBRIDGE_H 



#include <string>
using namespace std; 

//=============ABSTRACTION====================//
class Notification {
private: 
    NotificationChannel* channel; 
    Participant* recipient; 

public: 
    void send(); 
    string compose();

};

//===============REFINED_ABSTRACTION===========//

class ResultNotice : public Notification {
public: 
    compose();
}


//============IMPLEMENTOR==============//

class NotificationChannel {
public: 
    virtual void deliver(string to, string text)=0;
}

//============CONCRETE_IMPLEMENTOR==========//

class EmailChannel : public Notification{
private: 
    unique_ptr<EmailTransport> transport; 

public: 
    EmailChannel(EmailTransport* t);
    void deliver(string to, string subject, string body); 

};

class AppChannel : public NotificationChannel{
public: 
    void deliver(string to, string subject, string body); 
};

//=======================================//
//======ADAPTER==========================//
//=======================================//

//===============TARGET===================//


class EmailTransport{
public: 
    virtual bool sendEmail(string to, string subject, string body)=0; 
}

class ConsoleEmailAdapter : public EmailTransport {
private: 
    ostream& out; 

public: 
    ConsoleEmailAdapter(ostream& o); 
    bool sendEmail(string to, string subject, string body);

}

#endif //NOTIFICATIONBRIDGE_H