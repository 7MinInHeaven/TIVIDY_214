#ifndef RULEDECORATOR_H
#define RULEDECORATOR_H

#include <string>
#include <vector>
#include "Composite.h"

class RuleDecorator : public WorkGroup {
protected:
    WorkGroup* group;
    std::string key;
    std::string op;

public:
    RuleDecorator(WorkGroup* group, const std::string& key, const std::string& op);
    ~RuleDecorator() override;
    virtual bool evaluate(const WorkItem& item) const = 0;
    void handleRequest(WorkItem item) override;
    void add(WorkGroup* child);
    void add(Stage* child);
    void add(SubProcess* child);
    void add(WorkUnit* child);
    std::vector<WorkComponent*> getChildren() const;

    WorkGroup* getWorkGroup() const { return group; }
};

#endif