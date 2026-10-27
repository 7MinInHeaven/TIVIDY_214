#include "Decorator.h"

using namespace std;

//--------------------------------construction and rules ------------------------------

ValidatedTask::ValidatedTask(WorkComponent* taskToWrap, const string& statusToGuard)
    : TaskDecorator(taskToWrap), guardedStatus(statusToGuard) {
    if (statusToGuard.empty()) {
        throw invalid_argument("ValidatedTask needs the status it guards, e.g. \"InProgress\"");
    }
}

void ValidatedTask::addRule(shared_ptr<const ValidationRule> rule) {
    if (!rule) {
        throw invalid_argument("ValidatedTask cannot add a null rule");
    }
    validationRules.push_back(rule);
}

const string& ValidatedTask::getGuardedStatus() const { return guardedStatus; }

size_t ValidatedTask::getRuleCount() const { return validationRules.size(); }

bool ValidatedTask::validate(vector<string>& failureMessages) {
    bool allPassed = true;
    for (size_t i = 0; i < validationRules.size(); ++i) {
        if (!validationRules[i]->check(*this)) {
            failureMessages.push_back(validationRules[i]->getMessage());
            allPassed = false;
        }
    }
    return allPassed;
}

void ValidatedTask::nextState() {
    if (getState()->getStatus() == "In Progress") {
        vector<string> failures;
        if (!validate(failures)) {
            throw ValidationError(getName(), failures);
        }
    }
    TaskDecorator::nextState();
}

WorkComponent* ValidatedTask::clone() const {
    ValidatedTask* copy = new ValidatedTask(getWrappedTask()->clone(), guardedStatus);
    copy->validationRules = validationRules;
    return copy;
}
