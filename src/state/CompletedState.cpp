#include "CompletedState.h"
#include "WorkComponent.h"

void CompletedState::handleUpdate(WorkComponent* context)
{
    //dont do anything
}

std::string CompletedState::getStatus()
{
    return "Completed";
}