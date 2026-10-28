#include "Decorator.h"

using namespace std;

PrioritisedTask::PrioritisedTask(WorkComponent* taskToWrap, PriorityLevel level, int sla, const Clock* timeSource)
    : TaskDecorator(taskToWrap),
      priorityLevel(level),
      slaDays(sla),
      clock(timeSource != nullptr ? timeSource : &defaultClock()),
      deadline(0) {
    if (sla < 0) {
        throw invalid_argument("PrioritisedTask: slaDays cannot be negative");
    }
}

PriorityLevel PrioritisedTask::getPriority() const { return priorityLevel; }

void PrioritisedTask::setPriority(PriorityLevel newPriority) { priorityLevel = newPriority; }

//------------------------------- SLA tracking ---------

void PrioritisedTask::nextState() {
    TaskDecorator::nextState();
    if (slaDays > 0 && deadline == 0) {
        deadline = clock->now() + static_cast<time_t>(slaDays) * 86400; // first state change starts the SLA
    }
}

int PrioritisedTask::getSlaDays() const { return slaDays; }

bool PrioritisedTask::hasDeadline() const { return deadline != 0; }

time_t PrioritisedTask::getDeadline() const { return deadline; }

bool PrioritisedTask::isOverdue() const {
    return hasDeadline() && !isComplete() && clock->now() > deadline;
}

long PrioritisedTask::getSecondsRemaining() const {
    if (!hasDeadline()) {
        return 0;
    }
    return static_cast<long>(deadline - clock->now());
}


WorkComponent* PrioritisedTask::clone() const {
    return new PrioritisedTask(getWrappedTask()->clone(), priorityLevel, slaDays, clock);
}
//pretty sure we dont copy deadline here