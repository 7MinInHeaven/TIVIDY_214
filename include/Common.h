#ifndef COMMON_H
#define COMMON_H

#include <cstddef>
#include <ctime>
#include <string>
#include <vector>

//------------PriorityLevel----------

enum class PriorityLevel { Low, Normal, High, Urgent };

std::string priorityLevelToString(PriorityLevel level);

//-------- Clock---------

class Clock {
public:
    virtual ~Clock() {}
    virtual std::time_t now() const = 0;
};

class SystemClock : public Clock {
public:
    std::time_t now() const override;
};

class ManualClock : public Clock {
public:
    explicit ManualClock(std::time_t startTime = 0);
    std::time_t now() const override;
    void setTime(std::time_t newTime);
    void advanceSeconds(long seconds);
    void advanceDays(int days);

private:
    std::time_t currentTime;
};

const Clock& defaultClock();

//---------------------Participant----------------------

class Participant {
public:
    Participant(int participantId, const std::string& participantName, const std::string& roleName);

    int getId() const;
    const std::string& getName() const;
    const std::string& getRoleName() const;
    bool hasRole(const std::string& roleName) const;

private:
    int id;
    std::string name;
    std::string role;
};

//-------- -------------------AuditLog --------

struct AuditEntry {
    std::size_t sequenceNumber;
    std::time_t timestamp;
    std::string message;
};

class AuditLog {
public:
    explicit AuditLog(const Clock* clock = nullptr);
    virtual ~AuditLog();

    virtual void record(const std::string& message);

    std::size_t size() const;
    const std::vector<AuditEntry>& getEntries() const;
    bool containsText(const std::string& text) const;
    static std::string formatEntry(const AuditEntry& entry);

private:
    const Clock* clock;
    std::vector<AuditEntry> entries;
};

#endif // COMMON_H
