#ifndef TEMP_H
#define TEMP_H

#include <map>
#include <string>

class WorkComponent;
class WorkGroup;
class WorkUnit;
class SubProcess;
class Stage;

class WorkMemento {};

class WorkVisitor {
public:
    virtual ~WorkVisitor() = default;
    virtual void visit(WorkUnit&) {}
    virtual void visit(SubProcess&) {}
    virtual void visit(Stage&) {}
    virtual void visit(WorkGroup&){}
};


class WorkIterator {
    public:
    WorkIterator() = default;

    void first();
    void next();
    bool isDone();
    WorkComponent* getCurrent();
};

class DepthFirstIterator : public WorkIterator {
    private:
    WorkComponent* root;

    public:
    DepthFirstIterator(WorkComponent* root);
};

#endif /* TEMP_H */

