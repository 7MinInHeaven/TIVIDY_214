#ifndef COMPOSITE_H
#define COMPOSITE_H

#include <string>
#include <vector>
#include "TEMP.h"

class WorkState; 

class WorkComponent : public Handler
{
public:
    WorkComponent(std::string name);
    WorkComponent(const WorkComponent& other);

    //getters
    WorkState* getState();
    std::string getName();
    std::string getData(std::string key);

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
    WorkGroup(const WorkGroup& other);

    void remove(WorkComponent* child);
    std::vector<WorkComponent*> getChildren();

    virtual void  accept(WorkVisitor& visitor) = 0;
    virtual WorkGroup* clone() = 0;

    //ensure proper division of composite classes
    protected:
    void add(WorkComponent* child);

    private:
    std::vector<WorkComponent*> children;
};

class Stage : public WorkGroup {
    public:
    void accept(WorkVisitor& visitor) override;
    void add(SubProcess* child);
    void add(WorkUnit* child);

};

class SubProcess : public WorkGroup{
    public:
    void accept(WorkVisitor& visitor) override;
    void add(WorkUnit* child);
};

class WorkUnit : public WorkComponent {
    public:
    WorkMemento* createMemento();
    void setMemento(WorkMemento* memento);
    void accept(WorkVisitor& visitor);
};

#endif