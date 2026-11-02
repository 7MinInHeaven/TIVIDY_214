#include "Proxy.h"

std::string accessActionToString(AccessAction action) {
    static const char* const names[] = {
        "Submit", "View", "ViewHistory", "TakeOwnership", "CompleteTask",
        "Approve", "Reject", "Cancel", "Undo", "SaveProgress"
    };
    int index = static_cast<int>(action);
    if (index < 0 || index > 9) {
        return "Unknown";
    }
    return names[index];
}

std::string accessScopeToString(AccessScope scope) {
    static const char* const names[] = { "Any", "OwnRequest", "AssignedRequest" };
    int index = static_cast<int>(scope);
    if (index < 0 || index > 2) {
        return "Unknown";
    }
    return names[index];
}
