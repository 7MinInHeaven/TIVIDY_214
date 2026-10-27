#ifndef TEMP_H
#define TEMP_H

#include <map>
#include <string>

class WorkComponent;
class WorkGroup;
class WorkUnit;
class SubProcess;
class Stage;

class WorkMemento {};

class WorkVisitor {
public:
    virtual ~WorkVisitor() = default;
    virtual void visit(WorkUnit&) {}
    virtual void visit(SubProcess&) {}
    virtual void visit(Stage&) {}
    virtual void visit(WorkGroup&){}
};

enum class PriorityLevel{
    low,
    normal,
    high,
    urgent
};

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

