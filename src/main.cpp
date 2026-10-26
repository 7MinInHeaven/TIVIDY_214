#include <iostream>
#include <memory>
#include <cstdlib>
#include <curl/curl.h>
#include "NotificationBridge.h"
#include "MailjetEmailAdapter.h"

using namespace std;

int main() {
    // 1. Initialize curl globally
    curl_global_init(CURL_GLOBAL_DEFAULT);

    cout << "========================================\n";
    cout << "   TIVIDY: Generic Workflow Engine      \n";
    cout << "========================================\n\n";

    // 2. Setup the Adapter Factory
    unique_ptr<EmailTransport> transport;
    
    const char* key    = getenv("MJ_APIKEY_PUBLIC");
    const char* secret = getenv("MJ_APIKEY_PRIVATE");
    const char* from   = getenv("MJ_FROM_EMAIL");
    
    if (key && secret && from) {
        bool sandbox = (getenv("MJ_SANDBOX") != nullptr);
        cout << "[Init] Using Mailjet Adapter " << (sandbox ? "(SANDBOX)" : "(LIVE)") << "\n";
        transport.reset(new MailjetEmailAdapter(key, secret, from, "TIVIDY Support", sandbox));
    } else {
        cout << "[Init] Missing Mailjet credentials. Using Console Adapter.\n";
        transport.reset(new ConsoleEmailAdapter(cout));
    }

    // 3. Setup the Channels
    EmailChannel emailChannel(transport.get());
    AppChannel appChannel;

    // 4. Create Notifications
    string customerEmail = "your-personal-email@gmail.com"; // Change this later for a real test!
    string supportAgentId = "AGENT-442";

    ResultNotice   emailResult(&emailChannel, nullptr, customerEmail);
    ResultNotice   appResult(&appChannel, nullptr, supportAgentId);
    DeadlineNotice deadlineAlert(&appChannel, nullptr, supportAgentId);

    // 5. Send!
    emailResult.send();
    appResult.send();
    deadlineAlert.send();

    // 6. Check for Mailjet Errors
    if (auto* mj = dynamic_cast<MailjetEmailAdapter*>(transport.get())) {
        if (!mj->lastError().empty()) {
            cerr << "\n[MAILJET ERROR] " << mj->lastError() << "\n";
        }
    }

    cout << "\n========================================\n";
    cout << "   All Notifications Processed          \n";
    cout << "========================================\n";

    // 7. Cleanup curl
    curl_global_cleanup();
    return 0;
}