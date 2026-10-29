#include "NotificationBridge.h"
#include <algorithm>

NotificationData ResultNotice::compose() {
    string itemId = "N/A";
    string category = "General";

    // 1. Try to find an ID or Category if they exist 
    if (context) {
        if (context->hasData("id")) itemId = context->getData("id");
        else if (context->hasData("student_number")) itemId = context->getData("student_number");
        else if (context->hasData("policy_number")) itemId = context->getData("policy_number");

        if (context->hasData("category")) category = context->getData("category");
    }

    // 2. Dynamically build the email body from WHATEVER is in the payload
    string emailBody = 
        "Dear User,\n\n"
        "We are writing to provide you with an update on your request.\n\n"
        "--- REQUEST DETAILS ---\n";

    if (context) {
        // Loop through every single key-value pair in the WorkItem!
        for (const auto& pair : context->getPayloadData()) {
            // Skip the email address so we don't print it in the body
            if (pair.first == "email" || pair.first == "customer_email") continue;

            // Make the key look nice (e.g., "student_number" -> "Student Number")
            string formattedKey = pair.first;
            std::replace(formattedKey.begin(), formattedKey.end(), '_', ' ');
            if (!formattedKey.empty()) {
                formattedKey[0] = std::toupper(formattedKey[0]);
            }

            emailBody += formattedKey + ": " + pair.second + "\n";
        }
    }

    // 3. Add the status
    emailBody += 
        "\n--- CURRENT STATUS ---\n"
        "Your request is currently: " + currentStatus + "\n\n"
        "We will notify you as soon as there are further updates.\n\n"
        "Sincerely,\n"
        "The TIVIDY Support Team";

    // 4. Return the final composed message
    return {
        recipient,
        "Update on " + category + " Request (" + itemId + ") - Status: " + currentStatus,
        emailBody
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
    
    out << "\n--- SIMULATED EMAIL ---\n";
    out << "To: " << to << "\n";
    out << "Subject: " << subject << "\n";
    out << "Body: " << body << "\n";
    out << "--------------------------\n";
    
    return true; 
}

