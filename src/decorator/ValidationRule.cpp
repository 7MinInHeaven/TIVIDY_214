#include "Decorator.h"

#include <exception>

using namespace std;

//-------- ValidationRule --------

ValidationRule::~ValidationRule() {}

string ValidationRule::trim(const string& text) {
    const char* whitespace = " \t\r\n";
    size_t first = text.find_first_not_of(whitespace);
    if (first == string::npos) {
        return "";
    }
    size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

string ValidationRule::readValue(WorkComponent& task, const string& key) {
    try {
        return trim(task.getData(key));
    } catch (const exception&) {
        return "";
    }
}

//-------- ValidationError --------

string ValidationError::buildMessage(const string& taskName, const vector<string>& failureMessages) {
    string text = "Cannot continue '" + taskName + "': ";
    for (size_t i = 0; i < failureMessages.size(); ++i) {
        if (i > 0) {
            text += "; ";
        }
        text += failureMessages[i];
    }
    return text;
}

ValidationError::ValidationError(const string& taskName, const vector<string>& failureMessages)
    : runtime_error(buildMessage(taskName, failureMessages)), failures(failureMessages) {}

const vector<string>& ValidationError::getFailures() const { return failures; }
