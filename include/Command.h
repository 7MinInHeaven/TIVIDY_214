/**
 * @file Command.h
 * @brief Command pattern: agent actions as objects, run and undone by an invoker
*/

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

/**
 * @brief Command interface: one action that can be run on a task.
 */

class Action {
    public:
       /**
        * @brief Executes the action on the receiver (WorkComponent).
        * @return True if the action was successful, false otherwise.
        */
        virtual bool execute() = 0;

        /**
         * @brief Indicates whether the action can be undone.
         * @return true by default
         */
        virtual bool isUndoable() const { return true;}

        /**
         * @brief Undoes the action, restoring the receiver to its previous state.
         */
        virtual void undo() {};

        /**
         * @brief Gives the task this action is applied to.
         * @return The receiver. The command does not own it.
         */
        virtual WorkComponent* getReceiver() const =0;
        virtual ~Action() {}
};

/**
 * @brief Invoker: runs commands, keeps their history and undoes them.
 * 
 * Before a command runs, the executor asks the receiver for a snapshot
 * (WorkMemento) keeps it in a WorkHistory. Undo restores the receiver from the latest snapshot
 * 
 */

class ActionExecutor
{
    private:
        WorkHistory history;
        std::vector<Action*> actions;
        ActionExecutor(const ActionExecutor&) = delete;
        ActionExecutor& operator=(const ActionExecutor&) = delete;

    public:
        /**
         * @brief Creates an executor with an empty history.
         */
        ActionExecutor();

        /**
         * @brief Deletes the actions still held in the history.
         */
        ~ActionExecutor();

        /**
         * @brief Saves a snapshot of the receiver, then runs the action.
         * @param action The command to run. Must not be null
         * @return true if the action succeeded and was recorded, false otherwise
         */
        bool executeCommand(Action* action);

        /**
         * @brief Undoes the most recently executed action.
         * @return false if there is nothing to undo or the last action is not undoable, true otherwise
         */
        bool undoLastAction();
};


class StartTaskCommand : public Action
{
    private:
        WorkComponent* receiver;

    public:
        /**
         * @param receiver The task to start.
         */
        StartTaskCommand(WorkComponent* receiver);
        ~StartTaskCommand() = default;

        /**
         * @brief Starts the receiver task.
         * @return true if the task started, false if it was refused
         */
        bool execute() override;

         /**
         * @brief Gives the task this command acts on.
         * @return The receiver.
         */
        WorkComponent* getReceiver() const override;
        
};

class CompleteTaskCommand : public Action
{
    private:
        WorkComponent* receiver;

    public:
        /**
         * @param receiver The task to start.
         */
        CompleteTaskCommand(WorkComponent* receiver);
        ~CompleteTaskCommand() = default;

        /**
         * @brief Starts the receiver task.
         * @return true if the task started, false if it was refused
         */
        bool execute() override;

        /**
         * @brief Gives the task this command acts on.
         * @return The receiver.
         */
        WorkComponent* getReceiver() const override;
        

};


class AssignTaskCommand : public Action
{
    private:
        WorkComponent* receiver;
        std::string workerId;

    public:
        /**
         * @param receiver The task to start.
         * @param workerId The ID of the worker to assign the task to.
         */
        AssignTaskCommand(WorkComponent* receiver, const std::string& workerId);
        ~AssignTaskCommand() = default;

        /**
         * @brief Starts the receiver task.
         * @return true if the task started, false if it was refused
         */
        bool execute() override;

        /**
         * @brief Gives the task this command acts on.
         * @return The receiver.
         */
        WorkComponent* getReceiver() const override;
        
};


#endif