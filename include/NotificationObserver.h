/**
 * @file NotificationObserver.h
 * @brief The Observer that listens to workflow state changes and triggers notifications.
 */
#ifndef NOTIFICATIONOBSERVER_H 
#define NOTIFICATIONOBSERVER_H 

#include "WorkflowObserver.h"
#include "WorkflowSubject.h"
#include "NotificationBridge.h"
#include "WorkItem.h"
#include "Composite.h"
#include <string>

/**
 * @class NotificationObserver
 * @brief Concrete Observer in the Observer Pattern.
 * 
 * Acts as the glue between the workflow engine and the notification bridge.
 * When a WorkComponent changes state, this observer is notified, checks if the 
 * state has actually changed (to prevent spam), and dynamically builds a Notice.
 */
class NotificationObserver : public WorkflowObserver {
private: 
    WorkComponent* subject;             /**< The observable workflow component */
    NotificationChannel* emailChannel;  /**< The email delivery mechanism */
    NotificationChannel* appChannel;    /**< The terminal delivery mechanism */
    std::string lastState;              /**< Tracks the last state to prevent duplicate notifications */
public: 
    NotificationObserver(WorkComponent* w, NotificationChannel* e, NotificationChannel* a) 
        : subject(w), emailChannel(e), appChannel(a), lastState("") {}

    /**
     * @brief Triggered automatically by the WorkflowSubject when a state change occurs.
     */
    void update() override;
};

#endif //NOTIFICATIONOBSERVER_H