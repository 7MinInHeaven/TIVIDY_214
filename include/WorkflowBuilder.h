/**
 * @file WorkflowBuilder.h
 * @brief Concrete Builder declaration for constructing chained composite workflows.
 */

#ifndef WORKFLOWBUILDER_H
#define WORKFLOWBUILDER_H

#include <string>
#include <vector>
#include "ChainBuilder.h"
#include "Decorator.h"

/**
 * @class WorkflowBuilder
 * @brief Concrete Builder that assembles top-level WorkGroup chains and nested Composite hierarchies.
 * 
 */
class WorkflowBuilder : public ChainBuilder {
private:
    /** @brief Pointer to the first top-level WorkGroup in the Chain of Responsibility. */
    Handler* head = nullptr;

    /** @brief Pointer to the most recently added top-level WorkGroup in the chain. */
    Handler* tail = nullptr;

    /** @brief Collection of all top-level WorkGroup roots owned by the workflow. */
    std::vector<WorkGroup*> workGroups;

    /** @brief Active parent hierarchy stack used to determine composite linking order. */
    std::vector<WorkGroup*> stack;

    /** @brief Shared audit log instance passed to AuditedTask decorators. */
    AuditLog auditLog;

    /**
     * @brief Retrieves the currently active parent composite container from the top of the stack.
     * @return Pointer to the WorkGroup at the top of the stack, or nullptr if the stack is empty.
     */
    WorkGroup* currentGroup() const;

    /**
     * @brief Links a newly created composite container into the workflow and pushes it onto the stack.
     * 
     * @param[in] group Pointer to the WorkGroup, Stage, or SubProcess to attach.
     */
    void attach(WorkGroup* group);

public:
    /**
     * @brief Constructs a new WorkflowBuilder with an empty chain and stack.
     */
    WorkflowBuilder();

    /**
     * @brief Destroys the WorkflowBuilder instance.
     */
    ~WorkflowBuilder();

    /**
     * @brief Resets the builder's internal chain pointers, top-level group list, and scope stack.
     */
    void reset() override;

    /**
     * @brief Creates a new WorkGroup, attaches it to the chain or active parent, and opens its scope.
     * @param[in] name Display name of the WorkGroup to construct.
     */
    void buildWorkGroup(const std::string& name) override;

    /**
     * @brief Creates a new Stage, attaches it to the active parent (or chain), and opens its scope.
     * @param[in] name Display name of the Stage to construct.
     */
    void buildStage(const std::string& name) override;

    /**
     * @brief Creates a new SubProcess, attaches it to the active parent (or chain), and opens its scope.
     * @param[in] name Display name of the SubProcess to construct.
     */
    void buildSubProcess(const std::string& name) override;

    /**
     * @brief Closes the currently active composite container scope by popping the top of the stack.
     */
    void endGroup() override;

    /**
     * @brief Creates a leaf WorkUnit and attaches it to the currently active parent container.
     * @param[in] name Display name of the leaf WorkUnit to construct.
     */
    void buildWorkUnit(const std::string& name) override;

    /**
     * @brief Wraps the current composite's latest child with a decorator using default keys.
     * @param[in] type Decorator type: ValidatedTask, AuditedTask, or PrioritisedTask.
     */
    void addDecorator(const std::string& type) override;

    /**
     * @brief Wraps the current composite's latest child with a decorator.
     * @param[in] type Decorator type: ValidatedTask, AuditedTask, or PrioritisedTask.
     * @param[in] requiredKeys Input keys added as validation rules for ValidatedTask.
     */
    void addDecorator(
        const std::string& type,
        const std::vector<std::string>& requiredKeys) override;

    /**
     * @brief Returns the head Handler of the constructed Chain of Responsibility.
     * @return Pointer to the first Handler in the chain, or nullptr if no groups were built.
     */
    Handler* getResult() override;

    /**
     * @brief Returns the list of top-level chained WorkGroups constructed by the builder.
     * @return Vector of pointers to the top-level WorkGroup instances.
     */
    std::vector<WorkGroup*> getWorkGroups() override;

    /**
     * @brief Creates a deep-cloned workflow tree with a rebuilt top-level chain.
     * @return Independently owned cloned workflow roots.
     */
    std::vector<WorkGroup*> cloneWorkGroups() const;
};

#endif /* WORKFLOWBUILDER_H */