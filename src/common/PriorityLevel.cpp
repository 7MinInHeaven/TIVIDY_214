#include "Common.h"

std::string priorityLevelToString(PriorityLevel level) {
    static const char* const names[] = { "Low", "Normal", "High", "Urgent" };
    int index = static_cast<int>(level);
    if (index < 0 || index > 3) {
        return "Unknown";
    }
    return names[index];
}
