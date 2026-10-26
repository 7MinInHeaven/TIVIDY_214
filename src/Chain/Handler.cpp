#include "Handler.h"

void Handler::setNext(Handler* nextPhase)
{
    successor = nextPhase;
}
void Handler::handleRequest(WorkItem item)
{
    if (successor != nullptr)
    {
        successor->handleRequest(item);
    }
}