#include "NotificationObserver.h"
#include <iostream>

void NotificationObserver::update() override{
    if(!subject) return; 

    std::string currentState = subject->getState()->getStatus(); 

    if(currentState == lastState){
        return;
    }
    lastState = currentState;

    WorkItem* item = subject->getItem(); 
    if(!item) return; 

    std::string recipientEmail = item->getData("email"); 

    if(recipientEmail.empty()){
        recipientEmail = "requester@example.com";
    }

    ResultNotice emailNotice(emailChannel, item, recipientEmail, currentState);
    emailNotice.send();


    ResultNotice appNotice(appChannel, item, "system-admin", currentState);
    appNotice.send();
    
}