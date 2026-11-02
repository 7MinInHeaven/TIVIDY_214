#include "Common.h"


Participant::Participant(int participantId, const std::string& participantName, const std::string& roleName)
    : id(participantId), name(participantName), role(roleName) {}


int Participant::getId() const { return id; }
const std::string& Participant::getName() const { return name; }
const std::string& Participant::getRoleName() const { return role; }
bool Participant::hasRole(const std::string& roleName) const { return role == roleName; }
