#include <gtest/gtest.h>
#include "Composite.h"
#include "Handler.h"
#include "WorkItem.h"

TEST(HandlerTest, SetNextAndHandleRequest) {
    WorkGroup* groupOne = new WorkGroup("Test Group One");
    WorkGroup* groupTwo = new WorkGroup("Test Group Two");
    SubProcess* process = new SubProcess("Test Process");

    groupOne->add(process);
    groupOne->setNext(groupTwo);
    WorkItem item; 
    EXPECT_NO_THROW(groupOne->handleRequest(item));

    delete groupOne;
    delete groupTwo;
}

TEST(HandlerTest, HandleRequestWithoutSuccessor) {
    WorkGroup* loneGroup = new WorkGroup("Lone Group");
    WorkItem item;

    EXPECT_NO_THROW(loneGroup->handleRequest(item));

    delete loneGroup;
}