#include <gtest/gtest.h>
#include "NotificationBridge.h"
#include "NotificationObserver.h"
#include "Composite.h"
#include "PendingState.h"
#include "InProgressState.h"
#include "CompletedState.h"

// =========================================================
// 1. THE MOCK ADAPTER (The fake Strategy)
// =========================================================
class MockEmailAdapter : public EmailTransport {
public:
    std::string lastTo;
    std::string lastSubject;
    std::string lastBody;
    int sendCount = 0;

    bool sendEmail(std::string to, std::string subject, std::string body) override {
        lastTo = to;
        lastSubject = subject;
        lastBody = body;
        sendCount++;
        return true; // Pretend it succeeded
    }
};

// =========================================================
// 2. TESTING THE BRIDGE PATTERN
// =========================================================
TEST(NotificationBridgeTest, EmailChannelDeliversToAdapter) {
    // Setup
    MockEmailAdapter mockAdapter;
    EmailChannel emailChannel(&mockAdapter);
    NotificationData data{"test@test.com", "Test Subject", "Test Body"};

    // Action
    emailChannel.deliver(data);

    // Assertions
    EXPECT_EQ(mockAdapter.sendCount, 1);
    EXPECT_EQ(mockAdapter.lastTo, "test@test.com");
    EXPECT_EQ(mockAdapter.lastSubject, "Test Subject");
    EXPECT_EQ(mockAdapter.lastBody, "Test Body");
}

// =========================================================
// 3. TESTING THE OBSERVER PATTERN
// =========================================================
TEST(NotificationObserverTest, TriggersNotificationOnStateChange) {
    // Setup
    MockEmailAdapter mockAdapter;
    EmailChannel emailChannel(&mockAdapter);
    AppChannel appChannel; // We won't test the cout in unit tests

    WorkUnit* task = new WorkUnit("Test Task");
    
    // Inject payload data so the notice has something to compose
    task->getItem()->setData("email", "student@tuks.co.za");
    task->getItem()->setData("id", "REQ-123");
    task->getItem()->setData("category", "Test");

    // Create and attach Observer
    NotificationObserver observer(task, &emailChannel, &appChannel);
    task->attach(&observer);

    // Action: Change state
    // NOTE: If your WorkComponent::setState() doesn't automatically call notify(), 
    // you will need to call task->notify() right after this line.
    task->setState(new InProgressState());

    // Assertions
    EXPECT_EQ(mockAdapter.sendCount, 1);
    EXPECT_EQ(mockAdapter.lastTo, "student@tuks.co.za");
    // Check that the body actually contains the new state
    EXPECT_NE(mockAdapter.lastBody.find("In Progress"), std::string::npos);

    // Action: Change state again
    task->setState(new CompletedState());

    // Assertions (Should trigger a second email)
    EXPECT_EQ(mockAdapter.sendCount, 2);
    EXPECT_NE(mockAdapter.lastBody.find("Completed"), std::string::npos);

    // Cleanup
    delete task;
}

// =========================================================
// 4. TESTING ANTI-SPAM (Duplicate State Check)
// =========================================================
TEST(NotificationObserverTest, DoesNotSendDuplicateNotifications) {
    MockEmailAdapter mockAdapter;
    EmailChannel emailChannel(&mockAdapter);
    AppChannel appChannel;

    WorkUnit* task = new WorkUnit("Test Task");
    NotificationObserver observer(task, &emailChannel, &appChannel);
    task->attach(&observer);

    // 1. Change to InProgress
    task->setState(new InProgressState());
    EXPECT_EQ(mockAdapter.sendCount, 1);

    // 2. Change to InProgress AGAIN (should NOT send another email)
    // You can simulate this by calling notify() manually if setState allows it
    task->notify(); 
    EXPECT_EQ(mockAdapter.sendCount, 1); // Still 1! The lastState check worked.

    delete task;
}