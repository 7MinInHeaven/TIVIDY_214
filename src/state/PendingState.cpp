#include "PendingState.h"
#include "InProgressState.h"
#include "WorkComponent.h"

void PendingState::handleUpdate(WorkComponent* context)
{
    context->setState(new InProgressState());
}

std::string PendingState::getStatus()
{
    return "Pending";
}