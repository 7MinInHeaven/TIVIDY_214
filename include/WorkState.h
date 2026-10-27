#ifndef WORKSTATE_H
#define WORKSTATE_H
// TEST STUB mirroring the team's diagram: handleUpdate(), getStatus(), clone()
#include <string>
class WorkComponent;
class WorkState {
public:
    virtual ~WorkState() {}
    virtual void handleUpdate(WorkComponent* context) = 0;
    virtual std::string getStatus() = 0;
    virtual WorkState* clone() = 0;
};
class PendingState : public WorkState {
public:
    void handleUpdate(WorkComponent* context);
    std::string getStatus() { return "Pending"; }
    WorkState* clone() { return new PendingState(*this); }
};
class InProgressState : public WorkState {
public:
    void handleUpdate(WorkComponent* context);
    std::string getStatus() { return "In Progress"; }
    WorkState* clone() { return new InProgressState(*this); }
};
class CompletedState : public WorkState {
public:
    void handleUpdate(WorkComponent*) {}
    std::string getStatus() { return "Completed"; }
    WorkState* clone() { return new CompletedState(*this); }
};
#endif
