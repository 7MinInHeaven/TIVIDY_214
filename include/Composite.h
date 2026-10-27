#ifndef COMPOSITE_H
#define COMPOSITE_H

#include <string>
#include <vector>
#include <iostream>
#include "TEMP.h"
#include "WorkItem.h"
#include "Handler.h"
#include "WorkState.h"
#include "PendingState.h"
#include "InProgressState.h"
#include "CompletedState.h"
#include "WorkflowSubject.h"

class WorkComponent : public Handler, public WorkflowSubject
{
public:
    WorkComponent(std::string name);
    WorkComponent(const WorkComponent& other);
    virtual ~WorkComponent();

    //getters
    WorkState* getState() const;
    std::string getName() const;
    WorkItem* getItem() const;

    //setters
    void setState(WorkState* state);
    void setData(std::string key, std::string value);

    virtual void accept(WorkVisitor& visitor) = 0;
    virtual WorkComponent* clone() = 0;

private:
    WorkState *state;
    WorkItem *item;
    std::string name;
};

class WorkGroup : public WorkComponent{
    public:
    WorkGroup(std::string name);
    WorkGroup(const WorkGroup& other);
    ~WorkGroup() override;

    void add(Stage* child);
    void add(SubProcess* child);
    void add(WorkUnit* child);

    void remove(WorkComponent* child);
    void remove(std::string name);

    std::vector<WorkComponent*> getChildren() const;

    virtual void accept(WorkVisitor& visitor);
    virtual WorkGroup* clone();

    void handleRequest(WorkItem item);

    //ensure proper division of composite classes
    protected:
    void add(WorkComponent* child);

    private:
    std::vector<WorkComponent*> children;
};

class Stage : public WorkGroup {
    public:
    Stage(std::string name);

    void add(SubProcess* child);
    void add(WorkUnit* child);
    void accept(WorkVisitor& visitor) override;
    Stage* clone() override;

};

class SubProcess : public WorkGroup{
    public:
    SubProcess(std::string name);

    void add(WorkUnit* child);
    void accept(WorkVisitor& visitor) override;
    SubProcess* clone() override;
};

class WorkUnit : public WorkComponent {
    public:
    WorkUnit(std::string name);

    WorkMemento* createMemento();
    void setMemento(WorkMemento* memento);
    void accept(WorkVisitor& visitor);
    WorkUnit* clone() override;
};

#endif
