#ifndef NOTIFICATIONOBSERVER_H 
#define NOTIFICATIONOBSERVER_H 

#include "WorkflowObserver.h"
#include "WorkflowSubject.h"
#include "NotificationBridge.h"
#include "WorkItem.h"
//Component.h



class NotificationObserver : public WorkflowObserver {
private: 
    WorkflowSubject* subject; 
    NotificationChannel* emailChannel;
    NotificationChannel* appChannel;

public: 
    NotificationObserver(WorkflowSubject* w, NotificationChannel* e,
    NotificationChannel* a) :
    emailChannel(e), appChannel(a) , subject(w){}

    void update() override;

    //getState.getStatus
};


#endif //NOTIFICATIONOBSERVER_H