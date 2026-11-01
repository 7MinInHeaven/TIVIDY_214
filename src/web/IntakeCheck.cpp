#include "IntakeCheck.h"

#include <memory>

#include "Decorator.h"

using namespace std;

vector<string> checkIntake(const map<string, string> &payload, const vector<string> &requiredKeys) {
  ValidatedTask intake(new WorkUnit("Intake"), "Pending");   // Decorator: a task that validates its data
  for (map<string, string>::const_iterator it = payload.begin(); it != payload.end(); ++it) {
    intake.setData(it->first, it->second);
  }
  for (size_t i = 0; i < requiredKeys.size(); ++i) {
    intake.addRule(shared_ptr<const ValidationRule>(new RequiredKeyRule(requiredKeys[i])));
    if (requiredKeys[i].find("email") != string::npos) {
      intake.addRule(shared_ptr<const ValidationRule>(new EmailFormatRule(requiredKeys[i])));
    }
  }
  if (payload.count("amount") > 0) {
    intake.addRule(shared_ptr<const ValidationRule>(new NumericRangeRule("amount", 1, 1000000000)));
  }
  vector<string> failures;
  intake.validate(failures);
  return failures;
}