#ifndef TEMP_H
#define TEMP_H

#include <map>
#include <string>

class WorkComponent;
class WorkUnit;
class SubProcess;
class Stage;

class Handler {
public:
    virtual ~Handler() = default;
};

class WorkMemento {};

class WorkVisitor {
public:
    virtual ~WorkVisitor() = default;
    virtual void visit(WorkUnit&) {}
    virtual void visit(SubProcess&) {}
    virtual void visit(Stage&) {}
};

class WorkState {
public:
    virtual ~WorkState() = default;
    virtual WorkState* clone() = 0;
};

class PendingState : public WorkState {
public:
    WorkState* clone() override { return new PendingState(*this); }
};

class WorkItem {
    std::map<std::string, std::string> data;
public:
    WorkItem() = default;
    WorkItem(const WorkItem& other) = default;
    std::string getData(const std::string& key) { return data[key]; }
    void setData(const std::string& key, const std::string& value) { data[key] = value; }
};

#endif /* TEMP_H */

