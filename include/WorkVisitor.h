#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

class WorkComponent;
class WorkUnit;
class WorkGroup;
class PrioritisedTask;
class AuditedTask;
class ValidatedTask;
class ValidationRule;
#include "Common.h"



class WorkVisitor {
public:
    virtual ~WorkVisitor() = default;

    virtual void visit(WorkUnit& item) = 0;
    virtual void visit(WorkGroup& item) = 0;

    virtual void visit(PrioritisedTask& item) = 0;
    virtual void visit(AuditedTask& item) = 0;
    virtual void visit(ValidatedTask& item) = 0;
};

class ProgressReportVisitor : public WorkVisitor{
    public:
    ProgressReportVisitor();
    ~ProgressReportVisitor() override;
    void visit(WorkUnit& item) override;
    void visit(WorkGroup& item) override;

    void visit(PrioritisedTask& item) override;
    void visit(AuditedTask& item) override;
    void visit(ValidatedTask& item) override;

    void createReport(WorkComponent* root);

    private:

    //html writers
    std::string writeProgressCircle(int done, int total);
    std::string writeDoneCheck(bool complete);

    void writeChildren(std::vector<WorkComponent*>& children); 
    std::string writeLog(const AuditLog* log);
    std::string writeRules(const std::vector<std::shared_ptr<const ValidationRule>>& rules);

    //formats
    std::string getPriorityString(PriorityLevel priority);
    std::string formatTime(std::time_t timestamp);
    std::string formatSeconds(long totalSeconds);

    //file
    std::ofstream output;
};

class PriorityCheckerVisitor : public WorkVisitor{
    public:
    ~PriorityCheckerVisitor() override;
    void visit(WorkUnit& item) override;
    void visit(WorkGroup& item) override;

    void visit(PrioritisedTask& item) override;

    std::vector<PrioritisedTask*> found;
};