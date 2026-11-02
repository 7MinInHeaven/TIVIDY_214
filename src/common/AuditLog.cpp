#include "Common.h"

#include <sstream>

using namespace std;

//-------- Construction --------

AuditLog::AuditLog(const Clock* timeSource)
    : clock(timeSource != nullptr ? timeSource : &defaultClock()) {}

AuditLog::~AuditLog() {}

//-------- Recording --------

void AuditLog::record(const string& message) {
    AuditEntry entry;
    entry.sequenceNumber = entries.size() + 1;
    entry.timestamp = clock->now();
    entry.message = message;
    entries.push_back(entry);
}

//-------- Queries --------

size_t AuditLog::size() const { return entries.size(); }

const vector<AuditEntry>& AuditLog::getEntries() const { return entries; }

bool AuditLog::containsText(const string& text) const {
    for (size_t i = 0; i < entries.size(); ++i) {
        if (entries[i].message.find(text) != string::npos) {
            return true;
        }
    }
    return false;
}

string AuditLog::formatEntry(const AuditEntry& entry) {
    char timeText[32];
    time_t when = entry.timestamp;
    struct tm* parts = localtime(&when);
    if (parts == nullptr || strftime(timeText, sizeof(timeText), "%Y-%m-%d %H:%M:%S", parts) == 0) {
        timeText[0] = '?';
        timeText[1] = '\0';
    }
    ostringstream line;
    line << "#" << entry.sequenceNumber << " [" << timeText << "] " << entry.message;
    return line.str();
}
