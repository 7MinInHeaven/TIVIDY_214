#include <string>
#include <stack>
#include <functional>
#include <algorithm>
#include "Composite.h"
#include "WorkState.h"
#include "TEMP.h"

class WorkIterator {
    public:
    virtual void first() = 0;
    virtual void next() = 0;
    virtual bool isDone() = 0;
    virtual WorkComponent* getCurrent() = 0;
};

class DepthFirstIterator : public WorkIterator {
    public:
    DepthFirstIterator(WorkComponent* root);
    virtual void first() override;
    virtual void next() override;
    virtual bool isDone() override;
    virtual WorkComponent* getCurrent() override;
    
    protected:
    WorkComponent* root;
    size_t index = 0;
    std::vector<WorkComponent*> elements;

    private:
    virtual void recursive(WorkComponent* node);
};

class FilterIterator : public DepthFirstIterator{
    public:
    FilterIterator(WorkComponent* root, std::function<bool(WorkComponent*)> filter);
    virtual void first() override;
    virtual void next() override;

    private:
    std::function<bool(WorkComponent*)> filter;
    void skipFilter();
};

class StateIterator : public FilterIterator {
    public:
    StateIterator(WorkComponent* root, std::string state);

    private:
    std::string state;
};

class IncompleteIterator : public FilterIterator {
    public:
    IncompleteIterator(WorkComponent* root);
};

class PriorityIterator : DepthFirstIterator{
    public:
    PriorityIterator(WorkComponent* root);


    private:
};