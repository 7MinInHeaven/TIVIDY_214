#include "RequestChain.h"
#include "WorkflowBuilder.h"

RequestChain::RequestChain(const std::map<std::string, std::string>& payload)
{
    for (std::map<std::string, std::string>::const_iterator it = payload.begin(); it != payload.end(); ++it)
    {
        item.setData(it->first, it->second);
    }
}

RequestChain::~RequestChain()
{
    for (size_t i = 0; i < chain.size(); i++)
    {
        delete chain[i];
    }
}

bool RequestChain::build(const json& blueprint)
{
    WorkflowBuilder builder;
    Director director(&builder, nullptr, false);   
    try
    {
        director.construct(blueprint);
    }
    catch (...)
    {
        std::vector<WorkGroup*> partial = builder.getWorkGroups();
        for (size_t i = 0; i < partial.size(); i++)
        {
            delete partial[i];
        }
        return false; 
    }
    
    std::vector<WorkGroup*> templates = builder.getWorkGroups();
    chain = builder.cloneWorkGroups();
    for (size_t i = 0; i < templates.size(); i++)
    {
        delete templates[i];
    }

    for (size_t i = 0; i < chain.size(); i++)
    {
        chain[i]->setInteractive(false);            
    }
    indexTasks();
    return true;
}

void RequestChain::indexTasks()
{
    tasks.clear();
    taskGroup.clear();
    for (size_t i = 0; i < chain.size(); i++)
    {
        collectTasks(chain[i], i);
    }
}

void RequestChain::collectTasks(WorkComponent* component, size_t groupIndex)
{
    WorkGroup* group = dynamic_cast<WorkGroup*>(component);
    if (group == nullptr)
    {
        tasks.push_back(component);
        taskGroup.push_back(groupIndex);
        return;
    }

    std::vector<WorkComponent*> kids = group->getChildren();
    for (size_t i = 0; i < kids.size(); i++)
    {
        collectTasks(kids[i], groupIndex);
    }
}

const std::vector<WorkGroup*>& RequestChain::groups() const
{
    return chain;
}

int RequestChain::taskCount() const
{
    return static_cast<int>(tasks.size());
}

std::vector<std::string> RequestChain::describeTasks() const
{
    std::vector<std::string> lines;
    for (size_t i = 0; i < tasks.size(); i++)
    {
        lines.push_back(std::to_string(i + 1) + ". " + tasks[i]->getName() + " [" + tasks[i]->getStatus() +"] in " + chain[taskGroup[i]]->getName());
    }
    return lines;
}

bool RequestChain::isFinished() const
{
    if (chain.empty())
    {
        return false;
    }
    return unfinishedGroup() < 0;
}

int RequestChain::unfinishedGroup() const
{
    for (size_t i = 0; i < chain.size(); i++)
    {
        if (!chain[i]->isComplete() && !chain[i]->getChildren().empty())
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void RequestChain::resume()
{
    if (!chain.empty())
    {
        chain.front()->handleRequest(item);
    }
}

bool RequestChain::run(Action* action, int& executed)
{
    if (!executor.executeCommand(action))    
    {
        return false;
    }
    executed++;
    return true;
}

void RequestChain::rollback(int count)
{
    for (int i = 0; i < count; i++)
    {
        executor.undoLastAction();
    }
}

bool RequestChain::completeTask(const std::string& workerId, int taskId)
{
    if (taskId < 1 || taskId > taskCount())
    {
        return false;
    }
    
    size_t index = static_cast<size_t>(taskId - 1);
    WorkComponent* task = tasks[index];

    if (task->isComplete())
    {
        return false; 
    }

    int current = unfinishedGroup();
    if (static_cast<int>(taskGroup[index]) != current)
    {
        return false; 
    }

    int executed = 0;
    if (task->getStatus() == "Pending")
    {
        if (!run(new AssignTaskCommand(task, workerId), executed))
        {
            rollback(executed);
            return false;
        }
        
        if (!run(new StartTaskCommand(task), executed))
        {
            rollback(executed);
            return false;
        }
    }
    
    if (!run(new CompleteTaskCommand(task), executed))
    {
        rollback(executed);
        return false;
    }

    Step step;
    step.taskId = taskId;
    step.commands = executed;
    steps.push_back(step);
    resume();   
    
    return true;
}

int RequestChain::undoLastCompletion()
{
    if (steps.empty())
    {
        return -1; 
    }

    Step step = steps.back();
    steps.pop_back();
    rollback(step.commands);     
    return step.taskId;
}