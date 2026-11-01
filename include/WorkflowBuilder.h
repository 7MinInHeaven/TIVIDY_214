#ifndef WORKFLOWBUILDER_H
#define WORKFLOWBUILDER_H

#include <string>
#include <vector>
#include "ChainBuilder.h"
#include "Decorator.h"

class WorkflowBuilder : public ChainBuilder {
private:
    Handler* head = nullptr;
    Handler* tail = nullptr;

    std::vector<WorkGroup*> workGroups; 
    std::vector<WorkGroup*> stack; //stack used to determine linking order
    AuditLog auditLog;

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
    /**
     * @brief Wraps the current composite's latest child with a decorator.
     * @param type Decorator type: ValidatedTask, AuditedTask, or PrioritisedTask.
     * @param requiredKeys Input keys added as validation rules for ValidatedTask.
     */
    void addDecorator(const std::string& type) override;
    void addDecorator(
        const std::string& type,
        const std::vector<std::string>& requiredKeys) override;
    Handler* getResult() override;
    std::vector<WorkGroup*> getWorkGroups() override;

    /**
     * @brief Creates a deep-cloned workflow tree with a rebuilt top-level chain.
     * @return Independently owned cloned workflow roots.
     */
    std::vector<WorkGroup*> cloneWorkGroups() const;
};

#endif /* WORKFLOWBUILDER_H */