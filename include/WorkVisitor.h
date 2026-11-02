/**
 * @file WorkVisitor.h
 * @brief Visitor pattern, execute methods on Composite without modifying classes
 */

#ifndef WORKVISITOR_H
#define WORKVISITOR_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <optional>

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

/**
 * @class WorkVisitor
 * @brief   Abstract Visitor interface, visit() method per concrete WorkComponent type
 *          Each concrete type implements accept(WorkVisitor&) and provides itself as a parameter, 
 *             so that the visitor is aware of its type
 */
class WorkVisitor {
public:
    virtual ~WorkVisitor() = default;

    /** 
     * @brief Visits WorkUnit 
     * @param item WorkUnit being visited
     */
    virtual void visit(const WorkUnit& item) = 0;
    /**
     * @brief Visits WorkGroup
     * @param item WorkGroup being visited
     */
    virtual void visit(const WorkGroup& item) = 0;
    /**
     * @brief Visits Stage
     * @param item Stage being visited
     */
    virtual void visit(const Stage& item) = 0;
    /**
     * @brief Visits SubProcess
     * @param item SubProcess being visited
     */
    virtual void visit(const SubProcess& item) = 0;
    /**
     * @brief Visits PrioritisedTask
     * @param item PrioritisedTask being visited
     */
    virtual void visit(const PrioritisedTask& item) = 0;
    /**
     * @brief Visits AuditedTask
     * @param item AuditedTask being visited
     */
    virtual void visit(const AuditedTask& item) = 0;
    /**
     * @brief Visits ValidatedTask
     * @param item ValidatedTask being visited
     */
    virtual void visit(const ValidatedTask& item) = 0;
};


/**
 * @class ProgressReportVisitor
 * @brief  Concrete Visitor, displays tree as a HTML progress Report
 *         Groups are collapsable <details> blocks with progress circle to show no. children done out of the total
 *         Decorates add details about their specific functionality that they add to a component
 */

class ProgressReportVisitor : public WorkVisitor{
    public:
    /**
     * @brief Opens <filename>.html and writes HTML document header
     * @param filename name of report file, can add directory information to output file to specific directory 
     */
    ProgressReportVisitor(std::string filename = "report");
    /** @brief clouses the output file if it is still open */
    ~ProgressReportVisitor() override;

    /** @brief Writes leaf: name, priority and then any decorator badges and details */
    void visit(const WorkUnit& item) override;
    /** @brief Writes Group using visitGroup(item) */
    void visit(const WorkGroup& item) override;
    /** @brief Writes Stage using visitGroup(item) */
    void visit(const Stage& item) override;
    /** @brief Writes SubProcess using visitGroup(item) */
    void visit(const SubProcess& item) override;

    /**
     * @brief Records priority and deadline or OVERDUE badge, then visits wrapped task
     */
    void visit(const PrioritisedTask& item) override;
    /**
     * @brief Records audit log table details, then visits wrapped task
     */
    void visit(const AuditedTask& item) override;
    /**
     * @brief Records Validation rule list details, then visits wrapped task
     */
    void visit(const ValidatedTask& item) override;


    /**
     * @brief   Generates full report for tree and closes the file
     * @param root Root of the tree to report on
     */
    void createReport(WorkComponent* root);

    void writeCSS();
    
    private:
    //html writers
    /**
     * @brief Builds Group (WorkGroup, Stage, Subprocess)
     *      counts completed children for progress circle
     *      writes group summary with any badges and details form decorators, then writes children
     * @param item Group being written
     */
    void visitGroup(const WorkGroup& item);

    /**
     * @brief builds HTML for circular progress indicator
     * @param done No. completed children
     * @param total Total n. children (0 == 0%)
     * @return HTML fragment as sting
     */
    std::string writeProgressCircle(int done, int total);

    /**
     * @brief builds HTML for completion tick mark
     * @param complete whether item complete or not
     * @return HTML fragment as string if complete, empty if not
     */
    std::string writeDoneCheck(bool complete);

    /**
     * @brief Writes children of a group  as HTML list and visits each child
     * @param children Children to write
     */
    void writeChildren(std::vector<WorkComponent*>& children); 
    /**
     * @brief Builds HTML coloured priority label
     * @param level Priority to display
     * @return HTML fragment as string
     */
    std::string writePriority(PriorityLevel level);
    /**
     * @brief Builds HTML collapsable table of audit log
     * @param log Audit log to build
     * @return HTML fragment as string
     */
    std::string writeLog(const AuditLog* log);
    /**
     * @brief Builds HTML collapsable list of validation rules
     * @param log list of validation Rules to build
     * @return HTML fragment as string
     */
    std::string writeRules(const std::vector<std::shared_ptr<const ValidationRule>>& rules);

    //formats
    /**
     * @brief Converts PriorityLevel to display string
     * @return priorityLevel as string
     */
    std::string getPriorityString(PriorityLevel priority);
    /**
     * @brief formats timestamp in local time
     * @param timestamp seconds since epoch
     * @return formatted local time
     */
    std::string formatTime(std::time_t timestamp);
    /**
     * @brief formats duration as compact string
     * @param totalSeconds duration in seconds
     * @return formatted duration
     */
    std::string formatSeconds(long totalSeconds);

    //file
    std::ofstream outputFile; /// Destination HMTL file

    //decorator shenanigans
    std::string badges; ///Badges collected from decorates
    std::string details; ///Details collected from decorators

    PriorityLevel pendingPriority = PriorityLevel::Normal; ///Priority carried from PrioritisedTTask to node it wraps
};

/**
 * @class PriorityCheckerVisitor
 * @brief Calculates rank of Components Priority for PrioritisedIterator
 */

class PriorityCheckerVisitor : public WorkVisitor{
    public:
    void visit(const WorkUnit& item) override;
    void visit(const WorkGroup& item) override;
    void visit(const Stage& item) override;
    void visit(const SubProcess& item) override;
    /** @brief marks task as found, computes level (with overdue Penalty) */
    void visit(const PrioritisedTask& item) override;
    void visit(const AuditedTask& item) override;
    void visit(const ValidatedTask& item) override;

    /** @brief clears #found and #level */
    void reset();

    
    bool found = false; ///<True if last visited node was PrioritisedTask
    int level = 0; ///<Rank of last prioritised task

    private:
    static const int overduePenalty = 10; //Added to rank of overdue items
    /**
     * @brief Computes sorting rank for task
     * @param item priority decorator
     * @return Priority value, plus #overduePenalty if overdue
     */
    int computeRank(const PrioritisedTask& item); 
};


/**
 * @class WorkComponentValidator
 * @brief Validates whether Components can be nested inside other component
 */
class WorkComponentValidator {
    public:
    /**
     * @brief Checks whether child can be nested inside parent
     * @param parent Parent of child
     * @param child possible child of parent
     * @return if Child's kind is lower in hierarchy than parents
     */
    static bool validate(WorkComponent* parent, WorkComponent* child);

    private:
    void unwrap(const WorkComponent* wrapped);

    /**
     * @brief Level of component in hierarchy
     *      Declared in order of nesting
     */
    enum class Kind{
        WorkGroup,
        Stage,
        SubProcess,
        WorkUnit
    };

    /**
     * @class KindFinder
     * @brief Visitor that records Kind of component it visits
     */
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

        /** @brief returns Kind found by visit */
        Kind getKind();

        private:
        void unwrap(WorkComponent* wrapped);

        Kind kind; ///< Result of the visit
    };
};

#endif /* WORKVISITOR_H */
