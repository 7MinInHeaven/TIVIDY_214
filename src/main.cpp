#include <iostream>
#include "NotificationBridge.h"

using namespace std;

int main() {


    cout << "========================================\n";
    cout << "   TIVIDY: Generic Workflow Engine      \n";
    cout << "   COS 214 Project Scaffolding          \n";
    cout << "========================================\n\n";

    // ---------------------------------------------------------
    // 1. Setup the Adapter (The Target)
    // We pass std::cout so the email is printed to the terminal
    // ---------------------------------------------------------
    ConsoleEmailAdapter consoleEmail(cout);

    // ---------------------------------------------------------
    // 2. Setup the Bridge Implementors (The Channels)
    // ---------------------------------------------------------
    EmailChannel emailChannel(&consoleEmail);
    AppChannel appChannel;

    // ---------------------------------------------------------
    // 3. Simulate the WorkItem Payload (For the Recipients)
    // ---------------------------------------------------------
    string customerEmail = "bob@example.com";
    string supportAgentId = "AGENT-442";

    // ---------------------------------------------------------
    // 4. Test the Bridge Refined Abstractions (The Notices)
    // ---------------------------------------------------------
    
    // Scenario A: Send an email to the customer about their claim
    cout << "--- Triggering: Customer Email (Result Notice) ---\n";
    ResultNotice emailResult(&emailChannel, customerEmail);
    emailResult.send();
    cout << "\n";

    // Scenario B: Send an in-app TUI alert to the Support Agent
    cout << "--- Triggering: Support Agent App Alert (Result Notice) ---\n";
    ResultNotice appResult(&appChannel, supportAgentId);
    appResult.send();
    cout << "\n";

    // Scenario C: Send an urgent deadline alert to the Support Agent
    cout << "--- Triggering: Support Agent App Alert (Deadline Notice) ---\n";
    DeadlineNotice deadlineAlert(&appChannel, supportAgentId);
    deadlineAlert.send();
    cout << "\n";

    cout << "========================================\n";
    cout << "   All Notifications Sent Successfully  \n";
    cout << "========================================\n";

    return 0;
}
