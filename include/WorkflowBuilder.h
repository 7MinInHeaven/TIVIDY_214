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

    /**
     * @brief Returns the group that nodes currently being processed will be attached to
     */
    WorkGroup* currentGroup() const;

    /**
     * @brief Links WorkGroups together either in a Chain of Responsibility or Links subtasks to a WorkGroup
     * @param[in] group The WorkGroup that will either be attached to the list or will be attached to another Workgroup
     */
    void attach(WorkGroup* group);

public:
    /**
     * @brief Constructs a new WorkflowBuilder instance
     */
    WorkflowBuilder();

    /**
     * @brief Destroys the WorkflowBuilder instance.
     */
    ~WorkflowBuilder();

    
    /**
     * @brief Clears the current list of WorkGroups and currently processed sub-tasks
     */
    void reset() override;

    /**
     * @brief Create a new WorkGroup instance and attach it to either the main list of steps or to another WorkGroup
     * @param[in] name The name of the WorkGroup to be created
     */
    void buildWorkGroup(const std::string& name) override;

        
    /**
     * @brief Create a Stage instance and links it to a WorkGroup 
     * @param[in] name The name of the Stage to be created
     */
    void buildStage(const std::string& name) override;

        
    /**
     * @brief Create a SubProcess instance and links it to a WorkGroup 
     * @param[in] name The name of the SubProcess to be created
     */
    void buildSubProcess(const std::string& name) override;

    /**
     * @brief Pops sub-tasks off a stack to manage which sub-tasks are linked to which parents
     */
    void endGroup() override;

    /**
     * @brief Create a WorkUnit instance and links it to a WorkGroup 
     * @param[in] name The name of the WorkUnit to be created
     */
    void buildWorkUnit(const std::string& name) override;

    /**
     * @brief Wraps the current composite's latest child with a decorator.
     * @param type Decorator type: ValidatedTask, AuditedTask, or PrioritisedTask.
     */
    void addDecorator(const std::string& type) override;

    
    /**
     * @brief Wraps the most recently added child in the active composite.
     * @param[in] type The type of Decorator that needs to be made
     * @param[in] requiredKeys The keys needed to instantiate the Decorator
     */
    void addDecorator(
        const std::string& type,
        const std::vector<std::string>& requiredKeys) override;

    /**
     * @brief Returns the start of the chain of objects 
     */
    Handler* getResult() override;

    /**
     * @brief Returns the high level of steps (WorkGroups)
     */
    std::vector<WorkGroup*> getWorkGroups() override;

    /**
     * @brief Creates a deep-cloned workflow tree with a rebuilt top-level chain.
     * @return Independently owned cloned workflow roots.
     */
    std::vector<WorkGroup*> cloneWorkGroups() const;
};

#endif /* WORKFLOWBUILDER_H */