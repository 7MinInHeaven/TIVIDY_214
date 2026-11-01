/**
 * @file NotificationBridge.cpp
 * @brief Implementation of the Notification Abstractions and the Console Adapter.
 */
#include "NotificationBridge.h"
#include <algorithm>

/**
 * @brief Dynamically composes the workflow update email.
 * 
 * This method iterates through the generic WorkItem payload, format the keys 
 * (e.g., replacing underscores with spaces), and builds a text body. This ensures 
 * the TIVIDY engine remains generic for any workflow blueprint (Insurance, Maintenance, etc.).
 */
NotificationData ResultNotice::compose() {
    string itemId = "N/A";
    string category = "General";

    // Try to find an ID or Category if they exist 
    if (context) {
        if (context->hasData("id")) itemId = context->getData("id");
        else if (context->hasData("student_number")) itemId = context->getData("student_number");
        else if (context->hasData("policy_number")) itemId = context->getData("policy_number");

        if (context->hasData("category")) category = context->getData("category");
    }

    string emailBody = 
        "Dear User,\n\n"
        "We are writing to provide you with an update on your request.\n\n"
        "--- REQUEST DETAILS ---\n";

    if (context) {
        for (const auto& pair : context->getPayloadData()) {
            if (pair.first == "email" || pair.first == "customer_email") continue;

            string formattedKey = pair.first;
            std::replace(formattedKey.begin(), formattedKey.end(), '_', ' ');
            if (!formattedKey.empty()) {
                formattedKey[0] = std::toupper(formattedKey[0]);
            }
            emailBody += formattedKey + ": " + pair.second + "\n";
        }
    }

    emailBody += 
        "\n--- CURRENT STATUS ---\n"
        "Your request is currently: " + currentStatus + "\n\n"
        "We will notify you as soon as there are further updates.\n\n"
        "Sincerely,\n"
        "The TIVIDY Support Team";

    return {
        recipient,
        "Update on " + category + " Request (" + itemId + ") - Status: " + currentStatus,
        emailBody
    };
}

/**
 * @brief Composes a specific alert for approaching deadlines.
 */
NotificationData DeadlineNotice::compose(){
   const string itemId = context ? context->getData("id") : "unknown";

    return {
        recipient,
        "Deadline approaching for request " + itemId,
        "The deadline for request " + itemId + " is approaching. Current status: " + currentStatus
    };
}

/**
 * @brief Mocks email delivery by printing the notification to the terminal.
 */
bool ConsoleEmailAdapter::sendEmail(string to, string subject, string body) {
    string email = "";
    email += "\n--- SIMULATED EMAIL ---\n";
    email += "To: " + to + "\n";
    email += "Subject: " + subject + "\n";
    email += "Body: " + body + "\n";
    email += "--------------------------\n";
    cout << email; 
    return true; 
}