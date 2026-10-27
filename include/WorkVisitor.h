#include "Composite.h"
#include "TEMP.h"

class WorkVisitor {
public:
    virtual ~WorkVisitor() = default;

    virtual void visit(WorkUnit& item) = 0;
    virtual void visit(WorkGroup& item) = 0;

    virtual void visit(PrioritisedTask& item) = 0;
};

class ProgressReportVisitor : public WorkVisitor{
    public:
    void visit(WorkUnit& item);
    void visit(WorkGroup& item);

    void visit(PrioritisedTask& item);
};

class PriorityCheckerVisitor : public WorkVisitor{
    public:
    void visit(WorkUnit& item) override;
    void visit(WorkGroup& item) override;

    void visit(PrioritisedTask& item) override;

    std::vector<PrioritisedTask*> found;
};