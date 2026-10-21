#include "InProgressState.h"
#include "CompletedState.h"
#include "Composite.h"

void InProgressState::handleUpdate(WorkComponent* context)
{
    context->setState(new CompletedState());
}

std::string InProgressState::getStatus()
{
    return "In Progress";
}

WorkState* InProgressState::clone() { return new InProgressState(*this); }