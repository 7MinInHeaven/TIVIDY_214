#ifndef INTAKECHECK_H
#define INTAKECHECK_H

#include <map>
#include <string>
#include <vector>

/**
 * @brief Checks a new request's form data with Decorator rules (ValidatedTask + rules).
 *
 * Every required key must be filled; keys containing "email" must look like an email address;
 * an "amount" key must be a number. Returns one message per problem (empty = valid).
 */
std::vector<std::string> checkIntake(const std::map<std::string, std::string> &payload,
                                     const std::vector<std::string> &requiredKeys);

#endif