#include "WorkflowBuilder.h"
#include "Composite.h"
#include <memory>
#include <stdexcept>

/**
 * @brief Constructs a new WorkflowBuilder instance
 */
WorkflowBuilder::WorkflowBuilder() {}

/**
 * @brief Destroys the WorkflowBuilder instance.
 */
WorkflowBuilder::~WorkflowBuilder() {}

/**
 * @brief Returns the group that nodes currently being processed will be attached to
 */
WorkGroup* WorkflowBuilder::currentGroup() const 
{
    if (stack.empty()) {
        return nullptr;
    }
    return stack.back();
}

/**
 * @brief Links WorkGroups together either in a Chain of Responsibility or Links subtasks to a WorkGroup
 * @param[in] group The WorkGroup that will either be attached to the list or will be attached to another Workgroup
 */
void WorkflowBuilder::attach(WorkGroup* group)
{
    if (stack.empty()) 
    {
        workGroups.push_back(group);
        if (head == nullptr) {
            head = group;
            tail = group;
        } else {
            tail->setNext(group);
            tail = group;
        }
    } 
    else 
    {
        //attach nested processes to current group
        WorkGroup* parent = currentGroup();
        parent->add(group);
    }

    //place workgroup on stack so other processes can be added to it 
    stack.push_back(group);
}

/**
 * @brief Clears the current list of WorkGroups and currently processed sub-tasks
 */
void WorkflowBuilder::reset()
{
    head = nullptr;
    tail = nullptr;
    workGroups.clear();
    stack.clear();
}

/**
 * @brief Create a new WorkGroup instance and attach it to either the main list of steps or to another WorkGroup
 * @param[in] name The name of the WorkGroup to be created
 */
void WorkflowBuilder::buildWorkGroup(const std::string& name)
{
    WorkGroup* group = new WorkGroup(name);
    attach(group);
}
    
/**
 * @brief Create a Stage instance and links it to a WorkGroup 
 * @param[in] name The name of the Stage to be created
 */
void WorkflowBuilder::buildStage(const std::string& name)
{
    Stage* stage = new Stage(name);
    attach(stage);
}
    
/**
 * @brief Create a SubProcess instance and links it to a WorkGroup 
 * @param[in] name The name of the SubProcess to be created
 */
void WorkflowBuilder::buildSubProcess(const std::string& name) 
{
    SubProcess* sub = new SubProcess(name);
    attach(sub);
}
    
/**
 * @brief Create a WorkUnit instance and links it to a WorkGroup 
 * @param[in] name The name of the WorkUnit to be created
 */
void WorkflowBuilder::buildWorkUnit(const std::string& name)
{
    WorkGroup* parent = currentGroup();
    if (parent == nullptr) {
        return;
    }

    WorkUnit* unit = new WorkUnit(name);
    parent->add(unit);
}
    
/**
 * @brief Wraps the most recently added child in the active composite.
 * @param[in] type The type of Decorator that needs to be made
 */
void WorkflowBuilder::addDecorator(const std::string& type)
{
    const std::vector<std::string> noRequiredKeys;
    addDecorator(type, noRequiredKeys);
}

/**
 * @brief Wraps the most recently added child in the active composite.
 * @param[in] type The type of Decorator that needs to be made
 * @param[in] requiredKeys The keys needed to instantiate the Decorator
 */
void WorkflowBuilder::addDecorator(
    const std::string& type, const std::vector<std::string>& requiredKeys)
{
    WorkGroup* parent = currentGroup();
    if (parent == nullptr || parent->children.empty()) {
        throw std::logic_error(
            "cannot add a decorator without a current component");
    }

    WorkComponent* wrapped = parent->children.back();
    std::unique_ptr<WorkComponent> decorated;

    if (type == "ValidatedTask") {
        for (const std::string& key : requiredKeys) {
            if (key.empty()) {
                throw std::invalid_argument(
                    "ValidatedTask required keys cannot be empty");
            }
        }
        std::unique_ptr<ValidatedTask> validated(
            new ValidatedTask(wrapped, "In Progress"));
        for (const std::string& key : requiredKeys) {
            validated->addRule(
                std::shared_ptr<const ValidationRule>(new RequiredKeyRule(key)));
        }
        decorated.reset(validated.release());
    } else if (type == "AuditedTask") {
        decorated.reset(new AuditedTask(wrapped, &auditLog));
    } else if (type == "PrioritisedTask") {
        decorated.reset(
            new PrioritisedTask(wrapped, PriorityLevel::Normal, 0));
    } else {
        throw std::invalid_argument("unsupported decorator type: " + type);
    }

    parent->children.back() = decorated.release();
}

/**
 * @brief Pops sub-tasks off a stack to manage which sub-tasks are linked to which parents
 */
void WorkflowBuilder::endGroup() 
{
    if (!stack.empty()) 
    {
        stack.pop_back();
    }
}
    
/**
 * @brief Returns the start of the chain of objects 
 */
Handler* WorkflowBuilder::getResult()
{
    return head;
}

/**
 * @brief Returns the high level of steps (WorkGroups)
 */
std::vector<WorkGroup*> WorkflowBuilder::getWorkGroups()
{
    return workGroups;
}

/**
 * @brief Clones workflow roots and reconnects their Chain of Responsibility.
 */
std::vector<WorkGroup*> WorkflowBuilder::cloneWorkGroups() const
{
    std::vector<WorkGroup*> clones;

    for (WorkGroup* group : workGroups) {
        WorkGroup* clone = dynamic_cast<WorkGroup*>(group->clone());
        if (clone != nullptr) {
            clones.push_back(clone);
        }
    }

    for (size_t index = 1; index < clones.size(); ++index) {
        clones[index - 1]->setNext(clones[index]);
    }

    return clones;
}