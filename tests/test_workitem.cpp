#include <gtest/gtest.h>
#include "WorkItem.h"

// Test 1: Setting and retrieving basic fields
TEST(WorkItemTest, StoresAndRetrievesDataCorrectly) {
    WorkItem item;
    item.setData("policy_number", "POL-99182");
    item.setData("category", "AccidentalClaim");
    item.setData("amount", "300000");

    EXPECT_EQ(item.getData("policy_number"), "POL-99182");
    EXPECT_EQ(item.getData("category"), "AccidentalClaim");
    EXPECT_EQ(item.getData("amount"), "300000");
}

// Test 2: Querying non-existent key returns empty string safely
TEST(WorkItemTest, NonExistentKeyReturnsEmptyStringSafely) {
    WorkItem item;
    EXPECT_EQ(item.getData("unknown_key"), "");
    EXPECT_FALSE(item.hasData("unknown_key"));
}

// Test 3: Overwriting an existing key updates its value
TEST(WorkItemTest, OverwritingKeyUpdatesValue) {
    WorkItem item;
    item.setData("status", "PENDING");
    EXPECT_EQ(item.getData("status"), "PENDING");

    item.setData("status", "UNDER_REVIEW");
    EXPECT_EQ(item.getData("status"), "UNDER_REVIEW");
}

// Test 4: Returns the complete payload map
TEST(WorkItemTest, ReturnsCompletePayloadMap) {
    WorkItem item;
    item.setData("policy_number", "POL-101");
    item.setData("category", "DeathBenefit");
    item.setData("amount", "150000");

    const std::map<std::string, std::string>& payload = item.getPayloadData();

    EXPECT_EQ(payload.size(), 3);
    EXPECT_EQ(payload.at("policy_number"), "POL-101");
    EXPECT_EQ(payload.at("category"), "DeathBenefit");
    EXPECT_EQ(payload.at("amount"), "150000");
}

// Test 5: Copy constructor produces an independent copy of payloadData
TEST(WorkItemTest, CopyConstructorProducesIndependentDataCopy) {
    WorkItem original;
    original.setData("policy_number", "ORIGINAL-POL");
    original.setData("status", "PENDING");

    WorkItem copy(original);

    // Modifying the copy must NOT affect the original WorkItem
    copy.setData("policy_number", "MODIFIED-POL");
    copy.setData("status", "COMPLETED");

    EXPECT_EQ(original.getData("policy_number"), "ORIGINAL-POL");
    EXPECT_EQ(original.getData("status"), "PENDING");
    EXPECT_EQ(copy.getData("policy_number"), "MODIFIED-POL");
    EXPECT_EQ(copy.getData("status"), "COMPLETED");
}
