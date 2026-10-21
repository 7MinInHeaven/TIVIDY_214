#include "CompletedState.h"
#include "Composite.h"

void CompletedState::handleUpdate(WorkComponent* context)
{
    //dont do anything
}

std::string CompletedState::getStatus()
{
    return "Completed";
}