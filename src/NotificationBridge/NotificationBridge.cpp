#include "NotificationBridge.h"

string ResultNotice::compose(){
    //needs to still pull from payload

    return "Your claim has been processed";
}

void ResultNotice::send(){

    if(channel){
        channel->deliver(recipient,compose());
    }
}

string DeadlineNotice::compose(){
    return "Urgent: 7-day deadline";
}

void DeadlineNotice::send() {
    if (channel) {
        channel->deliver(recipient, compose());
    }
}

EmailChannel::EmailChannel(EmailTransport* t) : transport(t) {}

EmailChannel::~EmailChannel() = default;

void EmailChannel::deliver(string to, string text) {
    if (transport) {
        // NotificationChannel interface only passes 'text', 
        // we have to adapt it here to fit the EmailTransport's need for a subject and body.
        // For now hardcode a subject.
        string subject = "TIVIDY Claim Update";
        transport->sendEmail(to, subject, text);
    }
}


void AppChannel::deliver(string to, string text) {
    // Since this is a TUI delivering to the app printing to the console
    cout << "\n[APP ALERT for " << to << "]\n" 
         << text << "\n" 
         << "-----------------------------------\n";
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

