#ifndef BRIDGE_H 
#define BRIDGE_H 



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
    void deliver(string to, string text);
}

//============CONCRETE_IMPLEMENTOR==========//

class EmailChannel : public Notification{
private: 
    unique_ptr<EmailTransport> transport; 

public: 
    EmailChannel(EmailTransport* t);
    void deliver(string to, string subject, string body); 

}





#endif //BRIDGE_H