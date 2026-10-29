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
  virtual WorkState *getState() const; // mo
  virtual std::string getName() const; // mo
  WorkItem *getItem() const;

  // setters
  void setState(WorkState *state);
  virtual void setData(const std::string &key, const std::string &value); // mo

  // mo added virtual functions for decorator
  virtual std::string getData(const std::string &key) const;
  virtual void nextState();
  virtual bool isComplete() const;
  virtual PriorityLevel getPriority() const;

  virtual void accept(WorkVisitor& visitor) const = 0;
  virtual WorkComponent* clone() const override = 0;

  virtual bool add(WorkComponent* child);
  virtual void remove(WorkComponent* child);
  virtual void remove(std::string name);
  virtual std::vector<WorkComponent*> getChildren() const;


private:
  WorkState *state;
  WorkItem *item;
  std::string name;
};

class WorkGroup : public WorkComponent {
public:
  WorkGroup(std::string name);
  WorkGroup(const WorkGroup &other);
  ~WorkGroup() override;

  bool add(WorkComponent* child) override;

  void remove(WorkComponent *child) override;
  void remove(std::string name) override;

  std::vector<WorkComponent *> getChildren() const override;

  virtual void accept(WorkVisitor &visitor) const override;
  virtual WorkGroup *clone() const;

  void handleRequest(WorkItem item);

  // ensure proper division of composite classes
protected:

    private:
    std::vector<WorkComponent*> children;
    void printTasks(WorkComponent* component);
    friend class WorkflowBuilder;
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
private:
  std::string taskId;
  WorkState *currentState;
  std::string assignedWorkerId;

public:
  WorkUnit(const std::string &taskId);
  WorkMemento *createMemento();
  void setMemento(WorkMemento *memento);
  bool startTask();
  bool completeTask();
  bool assignTask(const std::string &workerId);
  std::string getTaskId();
  std::string getAssignedWorkerId();
  void accept(WorkVisitor &visitor) const override;
  WorkUnit *clone() const override;
};

#endif