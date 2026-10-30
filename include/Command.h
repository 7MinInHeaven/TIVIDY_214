#ifndef  COMMAND_H
#define  COMMAND_H
#include "Composite.h"
#include <vector>
#include <string>
#include "Memento.h"

class WorkUnit;
class WorkHistory;
class WorkState;
class WorkMemento;
class WorkComponent;

class Action {
    public:
        virtual bool execute() = 0;
        virtual void undo() {};
        virtual bool isUndoable() const { return true;}
        virtual WorkComponent* getReceiver() const =0;
        virtual ~Action() {}
};

class ActionExecutor
{
    private:
        WorkHistory history;
        std::vector<Action*> actions;
        ActionExecutor(const ActionExecutor&) = delete;
        ActionExecutor& operator=(const ActionExecutor&) = delete;

    public:
        ActionExecutor();
        ~ActionExecutor();
        bool executeCommand(Action* action);
        bool undoLastAction();
};


class StartTaskCommand : public Action
{
    private:
        WorkComponent* receiver;

    public:
        StartTaskCommand(WorkComponent* receiver);
        ~StartTaskCommand() = default;
        bool execute() override;
        WorkComponent* getReceiver() const override;
        
};

class CompleteTaskCommand : public Action
{
    private:
        WorkComponent* receiver;

    public:
        CompleteTaskCommand(WorkComponent* receiver);
        ~CompleteTaskCommand() = default;
        bool execute() override;
        WorkComponent* getReceiver() const override;
        
};


class AssignTaskCommand : public Action
{
    private:
        WorkComponent* receiver;
        std::string workerId;

    public:
        AssignTaskCommand(WorkComponent* receiver, const std::string& workerId);
        ~AssignTaskCommand() = default;
        bool execute() override;
        WorkComponent* getReceiver() const override;
        
};


#endif