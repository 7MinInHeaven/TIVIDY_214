#ifndef WORKVISITOR_H
#define WORKVISITOR_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

class WorkComponent;
class WorkUnit;
class WorkGroup;
class Stage;
class SubProcess;
class ValidationRule;
class PrioritisedTask;
class ValidatedTask;
class AuditedTask;
#include "Common.h"

class WorkVisitor {
public:
    virtual ~WorkVisitor() = default;

    virtual void visit(const WorkUnit& item) = 0;
    virtual void visit(const WorkGroup& item) = 0;
    virtual void visit(const Stage& item) = 0;
    virtual void visit(const SubProcess& item) = 0;
    virtual void visit(const PrioritisedTask& item) = 0;
    virtual void visit(const AuditedTask& item) = 0;
    virtual void visit(const ValidatedTask& item) = 0;
};

class ProgressReportVisitor : public WorkVisitor{
    public:
    ProgressReportVisitor();
    ~ProgressReportVisitor() override;
    void visit(const WorkUnit& item) override;
    void visit(const WorkGroup& item) override;
    void visit(const Stage& item) override;
    void visit(const SubProcess& item) override;
    void visit(const PrioritisedTask& item) override;
    void visit(const AuditedTask& item) override;
    void visit(const ValidatedTask& item) override;

    void createReport(WorkComponent* root);

    private:

    //html writers
    void visitGroup(const WorkGroup& item);

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
    void visit(const WorkUnit& item) override;
    void visit(const WorkGroup& item) override;
    void visit(const Stage& item) override;
    void visit(const SubProcess& item) override;
    void visit(const PrioritisedTask& item) override;
    void visit(const AuditedTask& item) override;
    void visit(const ValidatedTask& item) override;

    void reset();

    bool found = false;
    int level = 0;

    private:
    static const int overduePenalty = 10;
    int computeRank(const PrioritisedTask& item);
};

class WorkComponentValidator {
    public:
    static bool validate(WorkComponent* parent, WorkComponent* child);

    private:
    void unwrap(const WorkComponent* wrapped);

    enum class Kind{
        WorkGroup,
        Stage,
        SubProcess,
        WorkUnit
    };

    class KindFinder : WorkVisitor {
    public:
        KindFinder(WorkComponent* item);
        void visit(const WorkUnit& item) override;
        void visit(const WorkGroup& item) override;
        void visit(const Stage& item) override;
        void visit(const SubProcess& item) override;
        void visit(const PrioritisedTask& item) override;
        void visit(const AuditedTask& item) override;
        void visit(const ValidatedTask& item) override;

        Kind getKind();

        private:
        void unwrap(WorkComponent* wrapped);

        Kind kind;
    };
};

#endif /* WORKVISITOR_H */
