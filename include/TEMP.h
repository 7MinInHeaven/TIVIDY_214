#ifndef TEMP_H
#define TEMP_H

#include <map>
#include <string>

class WorkComponent;
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
};


#endif /* TEMP_H */