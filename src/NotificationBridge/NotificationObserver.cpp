/**
 * @file NotificationObserver.cpp
 * @brief Implementation of the NotificationObserver.
 */
#include "NotificationObserver.h"
#include <iostream>

/**
 * @brief Reacts to state changes by composing and sending notifications.
 * 
 * It pulls the current state and payload from the subject (WorkItem). If the state 
 * is different from `lastState`, it instantiates `ResultNotice` objects, injecting 
 * the appropriate channels and sending them.
 */
void NotificationObserver::update(){
    if(!subject) return; 

    std::string currentState = subject->getState()->getStatus(); 

    // Anti-spam guard: if state hasn't changed, don't notify again.
    if(currentState == lastState){
        return;
    }
    lastState = currentState;

    WorkItem* item = subject->getItem(); 
    if(!item) return; 

    std::string recipientEmail = item->getData("email"); 

    if(recipientEmail.empty()){
        recipientEmail = "requester@example.com"; // Fallback
    }

    // 1. Create the Abstraction (ResultNotice), inject the Implementor (emailChannel), and send.
    ResultNotice emailNotice(emailChannel, item, recipientEmail, currentState);
    emailNotice.send();

    // 2. Send an app alert to the system admin.
    ResultNotice appNotice(appChannel, item, "system-admin", currentState);
    appNotice.send();
}