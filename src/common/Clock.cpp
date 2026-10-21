#include "Common.h"

//------------------SystemClock-----------------------------------------

std::time_t SystemClock::now() const {
    return std::time(nullptr);
}

//------------------------ManualClock------------------------

ManualClock::ManualClock(std::time_t startTime) : currentTime(startTime) {}

std::time_t ManualClock::now() const { return currentTime; }

void ManualClock::setTime(std::time_t newTime) { currentTime = newTime; }

void ManualClock::advanceSeconds(long seconds) { currentTime += static_cast<std::time_t>(seconds); }

void ManualClock::advanceDays(int days) { currentTime += static_cast<std::time_t>(days) * 86400; }

//-------------------------------------------Default-------------------------

const Clock& defaultClock() {
    static SystemClock systemClock;
    return systemClock;
}
