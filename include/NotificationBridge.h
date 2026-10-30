/**
 * @file NotificationBridge.h
 * @brief Defines the Bridge pattern for the notification subsystem.
 * 
 * This file contains the Abstractions (Notification, ResultNotice) and the 
 * Implementors (NotificationChannel, EmailChannel) that decouple the message 
 * content from the delivery mechanism.
 */

#ifndef NOTIFICATIONBRIDGE_H 
#define NOTIFICATIONBRIDGE_H 

#include <string>
#include <iostream>
#include <memory>
#include "WorkItem.h"

using namespace std; 

/**
 * @struct NotificationData
 * @brief Data Transfer Object (DTO) passed from the Abstraction to the Implementor.
 */
struct NotificationData{
    string to;      /**< The recipient's address (email, ID, etc.) */
    string subject; /**< The subject line of the notification */
    string body;    /**< The main content/body of the notification */
};

/**
 * @class NotificationChannel
 * @brief The Implementor interface in the Bridge Pattern.
 * 
 * Defines how a notification is delivered, independent of what the notification is.
 */
class NotificationChannel {
public: 
    virtual ~NotificationChannel() = default;
    
    /**
     * @brief Delivers the composed notification data to the target.
     * @param data The NotificationData containing recipient, subject, and body.
     */
    virtual void deliver(const NotificationData& data) = 0;
};

/**
 * @class EmailTransport
 * @brief The Target interface in the Adapter Pattern.
 * 
 * Represents the generic interface for sending emails. Concrete adapters 
 * (Mailjet, Brevo, Resend) will implement this to connect to specific external APIs.
 */
class EmailTransport{
public: 
    virtual ~EmailTransport() = default;
    
    /**
     * @brief Sends an email via a specific provider.
     * @return True if successful, false otherwise.
     */
    virtual bool sendEmail(string to, string subject, string body) = 0; 
};

/**
 * @class Notification
 * @brief The Abstraction in the Bridge Pattern.
 * 
 * Holds the message context and delegates the actual delivery to a NotificationChannel.
 */
class Notification {
protected: 
    NotificationChannel* channel; /**< The delivery mechanism (Implementor) */
    string recipient;             /**< The target recipient */
    WorkItem* context;            /**< The workflow data used to compose the message */
    string currentStatus;         /**< The current state of the workflow (e.g., "Pending") */
public: 
    /**
     * @brief Constructs a Notification.
     * @param ch Pointer to the delivery channel.
     * @param ctx Pointer to the WorkItem context.
     * @param to The recipient.
     * @param status The current workflow status.
     */
    Notification(NotificationChannel* ch, WorkItem* ctx, string to, string status)
      : channel(ch), recipient(to), context(ctx), currentStatus(status) {}

    /**
     * @brief Triggers the composition and delivery of the notification.
     */
    virtual void send() {
        if(channel) channel->deliver(compose());
    }

    /**
     * @brief Abstract method to compose the message. Must be implemented by subclasses.
     * @return The fully formatted NotificationData.
     */
    virtual NotificationData compose() = 0;

    virtual ~Notification() = default; 
};

/**
 * @class ResultNotice
 * @brief A Refined Abstraction in the Bridge Pattern.
 * 
 * Represents a standard workflow status update notification.
 */
class ResultNotice : public Notification {
public: 
   ResultNotice(NotificationChannel* ch, WorkItem* ctx, string to, string status)
     : Notification(ch, ctx, to, status) {}
    NotificationData compose() override;
};

/**
 * @class DeadlineNotice
 * @brief A Refined Abstraction in the Bridge Pattern.
 * 
 * Represents an alert regarding an approaching deadline.
 */
class DeadlineNotice : public Notification {
public: 
    DeadlineNotice(NotificationChannel* ch, WorkItem* ctx, string to, string status)
     : Notification(ch, ctx, to, status) {}
    NotificationData compose() override; 
};

/**
 * @class EmailChannel
 * @brief A Concrete Implementor in the Bridge Pattern (and Context in the Strategy Pattern).
 * 
 * Bridges the notification system to the external email APIs by wrapping an EmailTransport.
 */
class EmailChannel : public NotificationChannel{
private: 
     EmailTransport* transport; /**< The specific API adapter (Strategy) to use */
public: 
    explicit EmailChannel(EmailTransport* t):transport(t){};
    
    void deliver(const NotificationData& d) override{
        if(transport) transport->sendEmail(d.to,d.subject,d.body);
    }
};

/**
 * @class AppChannel
 * @brief A Concrete Implementor in the Bridge Pattern.
 * 
 * Delivers notifications directly to the terminal/TUI.
 */
class AppChannel : public NotificationChannel{
public: 
    void deliver(const NotificationData& d) override{
        cout << "\n[APP ALERT for " << d.to << "]\n"
             << d.subject << "\n" << d.body << "\n"
             << "-----------------------------------\n";
    }; 
};

/**
 * @class ConsoleEmailAdapter
 * @brief A Concrete Adapter for testing and offline fallback.
 * 
 * Adapts the EmailTransport interface to standard console output.
 */
class ConsoleEmailAdapter : public EmailTransport {
private: 
    ostream& out; /**< The output stream (usually std::cout) */
public: 
    ConsoleEmailAdapter(ostream& o) : out(o){}; 
    bool sendEmail(string to, string subject, string body) override;
};

#endif //NOTIFICATIONBRIDGE_H