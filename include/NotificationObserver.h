#ifndef NOTIFICATIONOBSERVER_H 
#define NOTIFICATIONOBSERVER_H 

#include "WorkflowObserver.h"
#include "WorkflowSubject.h"
#include "NotificationBridge.h"
#include "WorkItem.h"
#include "Composite.h"
#include <string>



class NotificationObserver : public WorkflowObserver {
private: 
    WorkComponent* subject; 
    NotificationChannel* emailChannel;
    NotificationChannel* appChannel;
    std::string lastState;
public: 
    NotificationObserver(WorkComponent* w, NotificationChannel* e, NotificationChannel* a) 
        : subject(w), emailChannel(e), appChannel(a), lastState("") {}

    void update() override;

    //getState.getStatus
};


#endif //NOTIFICATIONOBSERVER_H