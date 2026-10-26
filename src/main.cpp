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
    
    // With no work item context (compose will say "unknown"):
ResultNotice   emailResult(&emailChannel, nullptr, customerEmail);
ResultNotice   appResult(&appChannel, nullptr, supportAgentId);
DeadlineNotice deadlineAlert(&appChannel, nullptr, supportAgentId);

emailResult.send();
appResult.send();
deadlineAlert.send();

    cout << "========================================\n";
    cout << "   All Notifications Sent Successfully  \n";
    cout << "========================================\n";

    return 0;
}
