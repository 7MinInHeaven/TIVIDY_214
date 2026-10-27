#ifndef WORKFLOWBUILDER_H
#define WORKFLOWBUILDER_H

#include <string>
#include <vector>
#include "ChainBuilder.h"

class WorkflowBuilder : public ChainBuilder {
private:
    Handler* head = nullptr;
    Handler* tail = nullptr;

    std::vector<WorkGroup*> workGroups; 
    std::vector<WorkGroup*> stack; //stack used to determine linking order

    WorkGroup* currentGroup() const;
    void attach(WorkGroup* group);

public:
    WorkflowBuilder();
    ~WorkflowBuilder();
    void reset() override;
    void buildWorkGroup(const std::string& name) override;
    void buildStage(const std::string& name) override;
    void buildSubProcess(const std::string& name) override;
    void endGroup() override;
    void buildWorkUnit(const std::string& name) override;
    // void addDecorator(const std::string& type,const std::vector<std::string>& requiredKeys = {}) override;
    // void addRule(const std::string& key, const std::string& op, const std::string& value) override;
    Handler* getResult() override;
    //std::vector<WorkGroup*> getWorkGroups() const override;
};

#endif /* WORKFLOWBUILDER_H */