#include "PendingState.h"
#include "InProgressState.h"
#include "Composite.h"

void PendingState::handleUpdate(WorkComponent* context)
{
    context->setState(new InProgressState());
}

std::string PendingState::getStatus()
{
    return "Pending";
}

WorkState* PendingState::clone() { return new PendingState(*this); }