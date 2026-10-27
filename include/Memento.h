#ifndef MEMENTO_H
#define MEMENTO_H

#include <string>
#include <vector>
class WorkState;

class WorkMemento{
    friend class WorkUnit;
    private:
        std::string taskId;
        WorkState* currentState;
        std::string assignedWorkerId;
        WorkMemento(const std::string& taskId, WorkState* currentState, const std::string& assignedWorkerId);
        WorkMemento(const WorkMemento& other) = delete;
        WorkMemento& operator=(const WorkMemento& other) = delete;

    public:
        ~WorkMemento();
};

class WorkHistory {
    private:
        std::vector<WorkMemento*> mementos;
        WorkHistory(const WorkHistory&) = delete;
        WorkHistory& operator=(const WorkHistory&) = delete;


    public:
        WorkHistory() {}
        ~WorkHistory();
        void addMemento(WorkMemento* memento);
        bool isEmpty() const;
        WorkMemento* getLatestMemento() const;
        void popLatest();
};



#endif