/**
 * @file Memento.h
 * @brief Memento pattern: snapshots of a task's state, used to undo actions.
 */
#ifndef MEMENTO_H
#define MEMENTO_H

#include <string>
#include <vector>
class WorkState;
class WorkUnit;

/**
 * @brief Snapshot of a task: its id, state and assigned worker.
 * 
 * The constructor and snapshot data are private; only workflow components
 * participating in the Memento protocol may create or restore a snapshot.
 */

class WorkMemento{
    friend class WorkComponent;
    friend class WorkUnit;
    private:
        std::string taskId;
        WorkState* currentState;
        std::string assignedWorkerId;

        /**
         * @param taskId Id of the task being saved.
         * @param currentState State of the task being saved.
         * @param assignedWorkerId Worker assigned to the task, empty if none.
         */
        WorkMemento(const std::string& taskId, WorkState* currentState, const std::string& assignedWorkerId);
        WorkMemento(const WorkMemento& other) = delete;
        WorkMemento& operator=(const WorkMemento& other) = delete;

    public:
        ~WorkMemento();
};

/**
 * @brief Caretaker: keeps snapshots in the order they were taken.
 * 
 * The latest snapshot is the one used by the nect undo. Not copyable.
 */

class WorkHistory {
    private:
        std::vector<WorkMemento*> mementos;
        WorkHistory(const WorkHistory&) = delete;
        WorkHistory& operator=(const WorkHistory&) = delete;


    public:
        /**
         * @brief Created an empty history.
         */
        WorkHistory() {}

        ~WorkHistory();

        /**
         * @brief Stores a snapshot as teh latest entry.
         * @param memento The snapshot to keep. The history takes ownership.
         */
        void addMemento(WorkMemento* memento);

        /**
         * @brief Tells whether any snapshot are stored.
         * @return true if the history is empty.
         */
        bool isEmpty() const;

        /**
         * @brief Gives the most recent snapshot without removing it.
         * @return The latest snapshot. Check isEmpty() first.
         */
        WorkMemento* getLatestMemento() const;

        /**
         * @brief Removes and deletes the most recent snapshot
         */
        void popLatest();
};



#endif