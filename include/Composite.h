#include <string>
#include <vector>
#include <iostream>
#include "TEMP.h"
#include "WorkItem.h"


class WorkComponent : public Handler
{
public:
    WorkComponent(std::string name);
    WorkComponent(const WorkComponent& other);
    virtual ~WorkComponent();

    //getters
    virtual WorkState* getState() const; //mo
    virtual std::string getName() const; //mo
    WorkItem* getItem() const;

    //setters
    void setState(WorkState* state);
    virtual void setData(const std::string& key,const std::string& value); //mo

    // mo added virtual functions for decorator
    virtual std::string getData(const std::string& key) const;
    virtual void nextState();
    virtual bool isComplete() const;
    virtual PriorityLevel getPriority() const;

    virtual void accept(WorkVisitor& visitor) = 0;
    virtual WorkComponent* clone() const = 0;

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
    virtual WorkGroup* clone() const;

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
    Stage* clone() const override;

};

class SubProcess : public WorkGroup{
    public:
    SubProcess(std::string name);

    void add(WorkUnit* child);
    void accept(WorkVisitor& visitor) override;
    SubProcess* clone() const override;
};

class WorkUnit : public WorkComponent {
    public:
    WorkUnit(std::string name);

    WorkMemento* createMemento();
    void setMemento(WorkMemento* memento);
    void accept(WorkVisitor& visitor);
    WorkUnit* clone() const override;
};
