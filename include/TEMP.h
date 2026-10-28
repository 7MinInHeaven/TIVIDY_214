#ifndef TEMP_H
#define TEMP_H

#include <map>
#include <string>

class WorkComponent;
class WorkGroup;
class WorkUnit;
class SubProcess;
class Stage;

class TaskDecorator {
    public:
    virtual PriorityLevel getPriority() {return PriorityLevel::normal}
};

class PrioritisedTask : public TaskDecorator {
    public:
    PriorityLevel getPriority() {return level;}

    private: 
    PriorityLevel level = PriorityLevel::normal;
};

#endif /* TEMP_H */

