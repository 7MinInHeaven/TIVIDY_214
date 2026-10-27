#include <gtest/gtest.h>
#include "WorkflowSubject.h"
#include "WorkflowObserver.h"

// Mock Observer 
class MockWorkflowObserver : public WorkflowObserver {
public:
    int updateCallCount = 0;

    void update() override {
        updateCallCount++;
    }
};

TEST(ObserverPatternTest, AttachAndNotifySingleObserver) {
    
    WorkflowSubject subject;
    MockWorkflowObserver obs1;

    
    subject.attach(&obs1);
    subject.notify();

    
    EXPECT_EQ(obs1.updateCallCount, 1);
}

TEST(ObserverPatternTest, NotifyTriggersAllAttachedObservers) {
    
    WorkflowSubject subject;
    MockWorkflowObserver obs1;
    MockWorkflowObserver obs2;
    subject.attach(&obs1);
    subject.attach(&obs2);

    
    subject.notify();

    
    EXPECT_EQ(obs1.updateCallCount, 1);
    EXPECT_EQ(obs2.updateCallCount, 1);

    
    subject.notify();
    EXPECT_EQ(obs1.updateCallCount, 2);
    EXPECT_EQ(obs2.updateCallCount, 2);
}

TEST(ObserverPatternTest, DetachRemovesObserverFromBroadcast) {
    
    WorkflowSubject subject;
    MockWorkflowObserver obs1;
    MockWorkflowObserver obs2;
    subject.attach(&obs1);
    subject.attach(&obs2);

    
    subject.detach(&obs1);
    subject.notify();

    
    EXPECT_EQ(obs1.updateCallCount, 0);
    EXPECT_EQ(obs2.updateCallCount, 1);
}

TEST(ObserverPatternTest, HandlesNullptrSafely) {
    
    WorkflowSubject subject;
    MockWorkflowObserver obs1;

    subject.attach(nullptr);
    subject.attach(&obs1);
    subject.detach(nullptr);

    subject.notify();
    EXPECT_EQ(obs1.updateCallCount, 1);
}