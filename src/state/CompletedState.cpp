#include "CompletedState.h"
#include "Composite.h"
#include <iostream>

void CompletedState::handleUpdate(WorkComponent* context)
{
    std::cout << context->getName() + "has already been completed\n";
}

std::string CompletedState::getStatus()
{
    return "Completed";
}

WorkState* CompletedState::clone() { return new CompletedState(*this); }