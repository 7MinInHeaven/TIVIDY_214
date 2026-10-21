#include <string>
#include <stack>
#include <functional>
#include "Composite.h"
#include "WorkState.h"
#include "Decorator.h"

class WorkIterator {
    public:
    void virtual first() = 0;
    void virtual next() = 0;
    bool virtual isDone() = 0;
    WorkComponent* virtual getCurrent() = 0;
};

class DepthFirstIterator : public WorkIterator {
    public:
    DepthFirstIterator(WorkComponent* root);
    virtual void first() override;
    virtual void next() override;
    virtual bool isDone() override;
    virtual WorkComponent* getCurrent() override;
    
    private:
    WorkComponent* root;
    size_t index = 0;
    std::vector<WorkComponent*> elements;

    void Recursive(WorkComponent* parent);
}

class FilterIterator() : public DepthFirstIterator{
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
}

class IncompleteIterator : public FilterIterator {
    public:
    IncompleteIterator(WorkComponent* root);
}



