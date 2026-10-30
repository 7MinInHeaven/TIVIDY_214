#include "InProgressState.h"
#include "CompletedState.h"
#include "Composite.h"

InProgressState::InProgressState() {}

void InProgressState::handleUpdate(WorkComponent* context)
{
    context->setState(new CompletedState());
}

std::string InProgressState::getStatus() const
{
    return "In Progress";
}

WorkState* InProgressState::clone() { return new InProgressState(*this); }

InProgressState::~InProgressState() {}