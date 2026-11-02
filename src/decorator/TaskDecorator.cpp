#include "Decorator.h"
#include <stdexcept>

using namespace std;

//-------- Construction --------

// The WorkComponent base needs a name, so it is taken from the wrapped task. nameOf() also rejects a null task before anything is dereferenced.
string TaskDecorator::nameOf(const WorkComponent* task) {
    if (task == nullptr) {
        throw invalid_argument("TaskDecorator needs a task to wrap");
    }
    return task->getName();
}

TaskDecorator::TaskDecorator(WorkComponent* taskToWrap)
    : WorkComponent(nameOf(taskToWrap)), wrappedTask(taskToWrap) {}

TaskDecorator::~TaskDecorator() {}

//-------- Forwarded operations --------

const WorkState* TaskDecorator::getState() const { return wrappedTask->getState(); } //gabriel added const

string TaskDecorator::getName() const { return wrappedTask->getName(); }

void TaskDecorator::setData(const string& key, const string& value) { wrappedTask->setData(key, value); }

string TaskDecorator::getData(const string& key) const { return wrappedTask->getData(key); }

//gabriel added getters
WorkItem* TaskDecorator::getItem() const{return wrappedTask->getItem();}
std::string TaskDecorator::getID() const{return wrappedTask->getID();}
std::string TaskDecorator::getAssignedWorkerId() const{return wrappedTask->getAssignedWorkerId();}
const std::vector<WorkComponent *> TaskDecorator::getChildren() const {return wrappedTask->getChildren();}

//gabriel added assignTask
bool TaskDecorator::assignTask(const std::string& workerID){return wrappedTask->assignTask(workerID);}
bool TaskDecorator::startTask(){return wrappedTask->startTask();}
bool TaskDecorator::completeTask(){return wrappedTask->completeTask();}

// lowkey after the wrapped task changes state, tell THIS decorator's observers too for outer
void TaskDecorator::nextState() {
    if (wrappedTask->isComplete()) {
        throw std::logic_error("Cannot advance a completed task");
    }
    
    wrappedTask->nextState();
    notify();
}

bool TaskDecorator::isComplete() const { return wrappedTask->isComplete(); }

PriorityLevel TaskDecorator::getPriority() const { return wrappedTask->getPriority(); }

//acces for subclasses
WorkComponent* TaskDecorator::getWrappedTask() const { return wrappedTask.get(); }
WorkComponent& TaskDecorator::inner() const { return *wrappedTask; }
