#include "Decorator.h"

#include <cmath>
#include <cstdlib>
#include <sstream>

using namespace std;

namespace {

// this is basically so it prints 250000 rather than 250000.000000 or 2.5e+05, after tests its lowkey necssary
string formatNumber(double value) {
    ostringstream text;
    if (value == floor(value) && fabs(value) < 1e15) {
        text << static_cast<long long>(value);
    } else {
        text << value;
    }
    return text.str();
}

}

//-------- RequiredKeyRule --------

RequiredKeyRule::RequiredKeyRule(const string& dataKey) : key(dataKey) {}

bool RequiredKeyRule::check(WorkComponent& task) const {
    return !readValue(task, key).empty();
}

string RequiredKeyRule::getMessage() const {
    return "'" + key + "' is required and must not be empty";
}

//-------- NumericRangeRule --------

NumericRangeRule::NumericRangeRule(const string& dataKey, double minimum, double maximum)
    : key(dataKey), minimumValue(minimum), maximumValue(maximum) {}

bool NumericRangeRule::check(WorkComponent& task) const {
    string text = readValue(task, key);
    if (text.empty()) {
        return false;
    }
    const char* start = text.c_str();
    char* parseEnd = nullptr;
    double value = strtod(start, &parseEnd);
    if (parseEnd == start || *parseEnd != '\0') {
        return false; // not a number, or trailing junk such as "100abc"
    }
    if (!isfinite(value)) {
        return false;
    }
    return value >= minimumValue && value <= maximumValue;
}

string NumericRangeRule::getMessage() const {
    return "'" + key + "' must be a number between " + formatNumber(minimumValue) +
           " and " + formatNumber(maximumValue);
}

//-------- EmailFormatRule --------

EmailFormatRule::EmailFormatRule(const string& dataKey) : key(dataKey) {}

bool EmailFormatRule::check(WorkComponent& task) const {
    string text = readValue(task, key);
    if (text.empty() || text.find(' ') != string::npos) {
        return false;
    }
    size_t at = text.find('@');
    if (at == string::npos || at == 0 || text.find('@', at + 1) != string::npos) {
        return false;
    }
    size_t dot = text.find('.', at + 2); // at least one character between '@' and '.'
    return dot != string::npos && dot + 1 < text.size();
}

string EmailFormatRule::getMessage() const {
    return "'" + key + "' must be a valid email address";
}
