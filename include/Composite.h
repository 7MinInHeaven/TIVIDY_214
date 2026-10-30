#ifndef COMPOSITE_H
#define COMPOSITE_H

#include "Common.h" // mo for PriorityLevel and Clock
#include "CompletedState.h"
#include "Handler.h"
#include "InProgressState.h"
#include "Memento.h"
#include "PendingState.h"
#include "WorkItem.h"
#include "WorkState.h"
#include "WorkflowSubject.h"
#include "WorkVisitor.h"
#include <iostream>
#include <string>
#include <vector>

class WorkMemento;
class WorkComponent : public Handler, public WorkflowSubject {

    
public:
  WorkComponent(std::string name);
  WorkComponent(const WorkComponent &other);
  virtual ~WorkComponent();

  // getters
  virtual const WorkState* getState() const; // mo
  virtual std::string getName() const; // mo
  virtual WorkItem* getItem() const;
  virtual std::string getID() const;
  virtual std::string getAssignedWorkerId() const;
  virtual std::string getData(const std::string &key) const;
  virtual PriorityLevel getPriority() const;

  // setters
  void setState(WorkState *state);
  virtual void setData(const std::string &key, const std::string &value); // mo
  virtual bool assignTask(const std::string& workerID);

  // mo added virtual functions for decorator
  virtual void nextState();
  virtual bool isComplete() const;
  virtual bool startTask();
  virtual bool completeTask();

  virtual void accept(WorkVisitor& visitor) const = 0;
  virtual WorkComponent* clone() const override = 0;

  virtual bool add(WorkComponent* child);
  virtual void remove(WorkComponent* child);
  virtual void remove(std::string name);
  virtual const std::vector<WorkComponent*> getChildren() const;

  //changed commands to be connected to each task
  virtual WorkMemento *createMemento() =0;
  virtual void setMemento(WorkMemento *memento) =0;
  virtual bool startTask() =0;
  virtual bool completeTask() =0;
  virtual bool assignTask(const std::string &workerId) =0;


protected:
  WorkState *state;
  WorkItem *item;
  std::string name;
  std::string taskID;
  std::string assignedWorkerID;

  std::string generateRandomID() const;
};

class WorkGroup : public WorkComponent {
public:
  WorkGroup(std::string name);
  WorkGroup(const WorkGroup &other);
  ~WorkGroup() override;

  bool add(WorkComponent* child) override;
  void remove(WorkComponent *child) override;
  void remove(std::string name) override;
  const std::vector<WorkComponent *> getChildren() const override;

  virtual void accept(WorkVisitor &visitor) const override;
  virtual WorkGroup *clone() const;

  void handleRequest(WorkItem item);
private:

  //WorkMemento *createMemento() override;
  //void setMemento(WorkMemento *memento) override;
  bool startTask() override;
  bool completeTask() override;
  bool assignTask(const std::string &workerId) override;
  std::vector<WorkComponent*> children;
    void printTasks(WorkComponent* component);
    friend class WorkflowBuilder;

  // ensure proper division of composite classes
protected:

    
};

class Stage : public WorkGroup {
public:
  Stage(std::string name);

  void accept(WorkVisitor &visitor) const override;
  Stage *clone() const override;
};

class SubProcess : public WorkGroup {
public:
  SubProcess(std::string name);

  void accept(WorkVisitor &visitor) const override;
  SubProcess *clone() const override;
};

class WorkUnit : public WorkComponent {
public:
  WorkMemento *createMemento();
  void setMemento(WorkMemento *memento);
  WorkUnit(const std::string &taskId);
  std::string getTaskId();
  std::string getAssignedWorkerId();
  void accept(WorkVisitor &visitor) const override;
  WorkUnit *clone() const override;
  WorkMemento *createMemento() override;
  void setMemento(WorkMemento *memento) override;
  bool startTask() override;
  bool completeTask() override;
  bool assignTask(const std::string &workerId) override;
};

#endif