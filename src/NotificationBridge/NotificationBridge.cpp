#include "NotificationBridge.h"

NotificationData ResultNotice::compose(){
    const string itemId = context ? context->getData("id") : "unknown";
    const string category = context ? context->getData("category") : "unknown";

    return {
        recipient,
        "Your request has been completed",
       "Hello, the status of your " + category + " request (" + itemId + ") is now: " + currentStatus
    };
}



NotificationData DeadlineNotice::compose(){
   const string itemId = context ? context->getData("id") : "unknown";

    return {
        recipient,
        "Deadline approaching for request " + itemId,
        "The deadline for request " + itemId + " is approaching. Current status: " + currentStatus
    };
}







bool ConsoleEmailAdapter::sendEmail(string to, string subject, string body) {
    // Simulates sending an email by printing it to the console.
    // Later, use a real Mailjet adapter 
    out << "\n--- SIMULATED EMAIL ---\n";
    out << "To: " << to << "\n";
    out << "Subject: " << subject << "\n";
    out << "Body: " << body << "\n";
    out << "--------------------------\n";
    
    return true; 
}

