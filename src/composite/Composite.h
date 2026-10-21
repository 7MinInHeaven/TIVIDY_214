#include <string>
#include "WorkFlowSubject.h"
#include "WorkState.h"
#include "WorkItem.h"
#include "WorkVisitor.h"
#include "Handler.h"

class WorkComponent : public WorkFlowSubject
{
public:
    WorkComponent(std::string name);
    WorkComponent(const WorkComponent& other);

    //getters
    WorkState* getState();
    std::string getName();
    std::string getData(std::string key);

    //setters
    void setData(std::string key, std::string value);

    //states
    void virtual start();
    void virtual review();
    void virtual completeReview();
    void virtual complete();

    void virtual accept(WorkVisitor& visitor) = 0;
    Handler* virtual clone();

private:
    WorkState *state;
    WorkItem *item;
    std::string name;
};

class WorkGroup : public WorkComponent{
    public:
    WorkGroup(const WorkGroup& other);

    void remove(WorkComponent* child);
    vector<WorkComponent*> getChildren();

    void start() override;
    void review() override;
    void completeReview() override;
    void complete() override;

    void virtual accept(WorkVisitor& visitor) override;
    Handler* clone() override;

    //ensure proper division of composite classes
    protected:
    void add(WorkComponent* child);

    private:
    vector<WorkComponent*> children;
};

class Stage : public WorkGroup {
    public:
    accept(WorkVisitor& visitor) override;
    void add(SubProcess* child);
    void add(WorkUnit* child);

};

class SubProcess : public WorkGroup{
    public:
    accept(WorkVisitor& visitor) override;
    void add(WorkUnit* child);
};

class WorkUnit : public WorkUnit {
    public:
    WorkMemento* createMemento();
    void setMemento(WorkMemento* memento);
    void accept(WorkVisitor& visitor);
};