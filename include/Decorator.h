#ifndef DECORATOR_H
#define DECORATOR_H

// Decorator pattern: add behaviour to a task by wrapping it.
//
// GoF roles:  Component          = WorkComponent   (src/composite)
//             ConcreteComponent  = WorkUnit        (src/composite)
//             Decorator          = TaskDecorator
//             ConcreteDecorators = ValidatedTask, AuditedTask, PrioritisedTask
// Helpers:    ValidationRule (+ RequiredKeyRule, NumericRangeRule, EmailFormatRule), ValidationError

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "Common.h"
#include "Composite.h"

//==================== Validation rules ====================

class ValidationRule {
    public:
        virtual ~ValidationRule();

        virtual bool check(WorkComponent& task) const = 0;

        virtual std::string getMessage() const = 0;

    protected:

        static std::string readValue(WorkComponent& task, const std::string& key);
        static std::string trim(const std::string& text);
};

class RequiredKeyRule : public ValidationRule {
    public:
        explicit RequiredKeyRule(const std::string& dataKey);
        bool check(WorkComponent& task) const override;
        std::string getMessage() const override;

    private:
        std::string key;
};

class NumericRangeRule : public ValidationRule {
    public:
        NumericRangeRule(const std::string& dataKey, double minimum, double maximum);
        bool check(WorkComponent& task) const override;
        std::string getMessage() const override;

    private:
        std::string key;
        double minimumValue;
        double maximumValue;
};

class EmailFormatRule : public ValidationRule {
    public:
        explicit EmailFormatRule(const std::string& dataKey);
        bool check(WorkComponent& task) const override;
        std::string getMessage() const override;

    private:
        std::string key;
};


class ValidationError : public std::runtime_error {
    public:
        ValidationError(const std::string& taskName, const std::vector<std::string>& failureMessages);
        const std::vector<std::string>& getFailures() const;

    private:
        static std::string buildMessage(const std::string& taskName, const std::vector<std::string>& failureMessages);
        std::vector<std::string> failures;
};

//----------------------------TaskDecorator----------------------------------------------

class TaskDecorator : public WorkComponent {
    public:

        explicit TaskDecorator(WorkComponent* taskToWrap);
        virtual ~TaskDecorator();

        WorkState* getState() const override;
        std::string getName() const override;
        void setData(const std::string& key, const std::string& value) override;
        std::string getData(const std::string& key) const override;
        void nextState() override;
        bool isComplete() const override;
        PriorityLevel getPriority() const override;
        void accept(WorkVisitor& visitor) override;

        WorkComponent* clone() const override = 0;

        WorkComponent* getWrappedTask() const;

    protected:
        WorkComponent& inner() const;

    private:
        TaskDecorator(const TaskDecorator&) = delete;
        TaskDecorator& operator=(const TaskDecorator&) = delete;

        static std::string nameOf(const WorkComponent* task);
        std::unique_ptr<WorkComponent> wrappedTask;
};

//----------------------------------ValidatedTask-----------------------------------------------


class ValidatedTask : public TaskDecorator {
    public:
    
        ValidatedTask(WorkComponent* taskToWrap, const std::string& guardedStatus);

        void addRule(std::shared_ptr<const ValidationRule> rule);
        bool validate(std::vector<std::string>& failureMessages);
        void nextState() override;

        WorkComponent* clone() const override;

        const std::string& getGuardedStatus() const;
        std::size_t getRuleCount() const;

    private:
        std::string guardedStatus;
        std::vector<std::shared_ptr<const ValidationRule> > validationRules;
};

//---------------------------------AuditedTask ------------------------------------

class AuditedTask : public TaskDecorator {
    public:

        AuditedTask(WorkComponent* taskToWrap, AuditLog* log);
        void nextState() override;
        WorkComponent* clone() const override;

    private:
        AuditLog* auditLog;
};

//--------------------- PrioritisedTask----------------------------------

class PrioritisedTask : public TaskDecorator {
    public:

        PrioritisedTask(WorkComponent* taskToWrap, PriorityLevel level, int slaDays = 0, const Clock* timeSource = nullptr);

        PriorityLevel getPriority() const override;
        void setPriority(PriorityLevel newPriority);

        void nextState() override;

        int getSlaDays() const;
        bool hasDeadline() const;
        std::time_t getDeadline() const;
        bool isOverdue() const;
        long getSecondsRemaining() const;

        WorkComponent* clone() const override;

    private:
        PriorityLevel priorityLevel;
        int slaDays;
        const Clock* clock;
        std::time_t deadline;
};

#endif
