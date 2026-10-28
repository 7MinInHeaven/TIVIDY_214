#ifndef SCENARIOACCESSPOLICIES_H
#define SCENARIOACCESSPOLICIES_H

// SCENARIO LAYER (not part of the generic engine).
// Each industry describes who may do what by filling the SAME AccessPolicy class with its own
// role names. The engine is never edited to support a new industry.

#include "Proxy.h"

/** @brief Life-insurance roles: Customer, ClaimsSpecialist, TeamLead, SeniorManager. */
void configureInsuranceAccess(AccessPolicy& policy);

/** @brief Service-request roles: Citizen, Dispatcher, Technician, Supervisor. */
void configureServiceRequestAccess(AccessPolicy& policy);

#endif // SCENARIOACCESSPOLICIES_H
