#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "ChainBuilder.h"
#include "Composite.h"
#include "Director.h"
#include "Handler.h"
#include "WorkItem.h"
#include "WorkflowBuilder.h"

using json = nlohmann::json;

// 1. Test WorkflowBuilder building a single WorkGroup with nested Stage, SubProcess, and WorkUnit
TEST(BuilderTest, WorkflowBuilderBuildsNestedCompositeHierarchy) {
    WorkflowBuilder builder;

    builder.buildWorkGroup("Root WorkGroup");
    builder.buildWorkUnit("Initial Triage Unit");

    builder.buildStage("Stage One");
    builder.buildWorkUnit("Stage One Unit");

    builder.buildSubProcess("SubProcess One");
    builder.buildWorkUnit("SubProcess Leaf Unit 1");
    builder.buildWorkUnit("SubProcess Leaf Unit 2");
    builder.endGroup(); // Close SubProcess One

    builder.endGroup(); // Close Stage One
    builder.endGroup(); // Close Root WorkGroup

    Handler* result = builder.getResult();
    EXPECT_NE(result, nullptr);

    WorkGroup* rootGroup = dynamic_cast<WorkGroup*>(result);
    EXPECT_NE(rootGroup, nullptr);
    EXPECT_EQ(rootGroup->getName(), "Root WorkGroup");
    EXPECT_EQ(rootGroup->getChildren().size(), 2u);

    // Child 0 of Root WorkGroup should be "Initial Triage Unit" (WorkUnit)
    WorkUnit* triageUnit = dynamic_cast<WorkUnit*>(rootGroup->getChildren()[0]);
    EXPECT_NE(triageUnit, nullptr);
    EXPECT_EQ(triageUnit->getName(), "Initial Triage Unit");

    // Child 1 of Root WorkGroup should be "Stage One" (Stage)
    Stage* stageOne = dynamic_cast<Stage*>(rootGroup->getChildren()[1]);
    EXPECT_NE(stageOne, nullptr);
    EXPECT_EQ(stageOne->getName(), "Stage One");
    EXPECT_EQ(stageOne->getChildren().size(), 2u);

    // Child 0 of Stage One should be "Stage One Unit" (WorkUnit)
    WorkUnit* stageUnit = dynamic_cast<WorkUnit*>(stageOne->getChildren()[0]);
    EXPECT_NE(stageUnit, nullptr);
    EXPECT_EQ(stageUnit->getName(), "Stage One Unit");

    // Child 1 of Stage One should be "SubProcess One" (SubProcess)
    SubProcess* subOne = dynamic_cast<SubProcess*>(stageOne->getChildren()[1]);
    EXPECT_NE(subOne, nullptr);
    EXPECT_EQ(subOne->getName(), "SubProcess One");
    EXPECT_EQ(subOne->getChildren().size(), 2u);
    EXPECT_EQ(subOne->getChildren()[0]->getName(), "SubProcess Leaf Unit 1");
    EXPECT_EQ(subOne->getChildren()[1]->getName(), "SubProcess Leaf Unit 2");

    WorkItem item;
    EXPECT_NO_THROW(rootGroup->handleRequest(item));

    delete rootGroup;
}

// 2. Test WorkflowBuilder reset() clears head/tail state cleanly
TEST(BuilderTest, WorkflowBuilderResetClearsResult) {
    WorkflowBuilder builder;
    EXPECT_EQ(builder.getResult(), nullptr);

    builder.buildWorkGroup("Temporary Group");
    builder.endGroup();

    Handler* firstResult = builder.getResult();
    EXPECT_NE(firstResult, nullptr);

    WorkGroup* firstGroup = dynamic_cast<WorkGroup*>(firstResult);
    EXPECT_NE(firstGroup, nullptr);

    builder.reset();
    EXPECT_EQ(builder.getResult(), nullptr);

    delete firstGroup;
}

/**
 * @brief Verifies that Builder clones roots and restores their chain links.
 */
TEST(BuilderTest, WorkflowBuilderClonesIndependentWorkflowChain) {
    WorkflowBuilder builder;
    builder.buildWorkGroup("First Group");
    builder.endGroup();
    builder.buildWorkGroup("Second Group");
    builder.endGroup();

    std::vector<WorkGroup*> clones = builder.cloneWorkGroups();

    ASSERT_EQ(clones.size(), 2u);
    EXPECT_NE(clones[0], builder.getWorkGroups()[0]);
    EXPECT_NE(clones[1], builder.getWorkGroups()[1]);
    EXPECT_EQ(clones[0]->getName(), "First Group");
    EXPECT_EQ(clones[1]->getName(), "Second Group");

    WorkItem item;
    EXPECT_NO_THROW(clones[0]->handleRequest(item));

    for (WorkGroup* clone : clones) {
        delete clone;
    }
    for (WorkGroup* group : builder.getWorkGroups()) {
        delete group;
    }
}

// 3. Test Director constructing a workflow from JSON via WorkflowBuilder
TEST(BuilderTest, DirectorConstructsCompositeTreeFromJson) {
    json blueprint = json::parse(R"({
        "workflow_name": "Unit Test Workflow",
        "chain": [
            {
                "type": "WorkGroup",
                "name": "Intake Group",
                "children": [
                    { "type": "WorkUnit", "name": "Log Claim" },
                    {
                        "type": "Stage",
                        "name": "Verification Stage",
                        "children": [
                            {
                                "type": "SubProcess",
                                "name": "Identity SubProcess",
                                "children": [
                                    { "type": "WorkUnit", "name": "Check ID" }
                                ]
                            }
                        ]
                    }
                ]
            }
        ]
    })");

    WorkflowBuilder builder;
    Director director(&builder);
    director.construct(blueprint);

    Handler* result = builder.getResult();
    EXPECT_NE(result, nullptr);

    WorkGroup* intakeGroup = dynamic_cast<WorkGroup*>(result);
    EXPECT_NE(intakeGroup, nullptr);
    EXPECT_EQ(intakeGroup->getName(), "Intake Group");
    EXPECT_EQ(intakeGroup->getChildren().size(), 2u);
    EXPECT_EQ(intakeGroup->getChildren()[0]->getName(), "Log Claim");

    Stage* verStage = dynamic_cast<Stage*>(intakeGroup->getChildren()[1]);
    EXPECT_NE(verStage, nullptr);
    EXPECT_EQ(verStage->getName(), "Verification Stage");
    EXPECT_EQ(verStage->getChildren().size(), 1u);

    SubProcess* idSub = dynamic_cast<SubProcess*>(verStage->getChildren()[0]);
    EXPECT_NE(idSub, nullptr);
    EXPECT_EQ(idSub->getName(), "Identity SubProcess");
    EXPECT_EQ(idSub->getChildren().size(), 1u);
    EXPECT_EQ(idSub->getChildren()[0]->getName(), "Check ID");

    WorkItem item;
    EXPECT_NO_THROW(intakeGroup->handleRequest(item));

    delete intakeGroup;
}

// 4. Test Director populating WorkItem from required_inputs via std::cin
TEST(BuilderTest, DirectorPopulatesRequiredInputsOnWorkItem) {
    json blueprint = json::parse(R"({
        "required_inputs": [
            { "key": "policy_number", "prompt": "Enter Policy Number" },
            { "key": "category", "prompt": "Enter Category" }
        ],
        "chain": []
    })");

    // Redirect std::cin to provide simulated user inputs
    std::istringstream simulatedInput("POL-777\nDeathBenefit\n");
    std::streambuf* oldCin = std::cin.rdbuf(simulatedInput.rdbuf());

    WorkItem item;
    WorkflowBuilder builder;
    Director director(&builder, &item, true);
    director.construct(blueprint);

    // Restore std::cin
    std::cin.rdbuf(oldCin);

    EXPECT_TRUE(item.hasData("policy_number"));
    EXPECT_EQ(item.getData("policy_number"), "POL-777");
    EXPECT_TRUE(item.hasData("category"));
    EXPECT_EQ(item.getData("category"), "DeathBenefit");
    EXPECT_EQ(builder.getResult(), nullptr);
}

/**
 * @brief Verifies that empty required input is retried before acceptance.
 */
TEST(BuilderTest, DirectorRetriesEmptyRequiredInput) {
    json blueprint = json::parse(R"({
        "required_inputs": [
            { "key": "policy_number", "prompt": "Enter Policy Number" }
        ],
        "chain": []
    })");

    std::istringstream simulatedInput("\nPOL-777\n");
    std::streambuf* oldCin = std::cin.rdbuf(simulatedInput.rdbuf());

    WorkItem item;
    WorkflowBuilder builder;
    Director director(&builder, &item, true);
    EXPECT_NO_THROW(director.construct(blueprint));

    std::cin.rdbuf(oldCin);
    EXPECT_EQ(item.getData("policy_number"), "POL-777");
}

/**
 * @brief Verifies that repeated empty required input fails clearly.
 */
TEST(BuilderTest, DirectorRejectsRepeatedEmptyRequiredInput) {
    json blueprint = json::parse(R"({
        "required_inputs": [
            { "key": "policy_number", "prompt": "Enter Policy Number" }
        ],
        "chain": []
    })");

    std::istringstream simulatedInput("\n\n\n");
    std::streambuf* oldCin = std::cin.rdbuf(simulatedInput.rdbuf());

    WorkItem item;
    WorkflowBuilder builder;
    Director director(&builder, &item, true);
    EXPECT_THROW(director.construct(blueprint), std::invalid_argument);

    std::cin.rdbuf(oldCin);
}

// 5. Test Director with null builder and multi-WorkGroup chain traversal using a spy ChainBuilder
namespace {
class RecordingChainBuilder : public ChainBuilder {
public:
    std::vector<std::string> events;

    void reset() override {
        events.push_back("reset");
    }

    void buildWorkGroup(const std::string& name) override {
        events.push_back("WorkGroup:" + name);
    }

    void buildStage(const std::string& name) override {
        events.push_back("Stage:" + name);
    }

    void buildSubProcess(const std::string& name) override {
        events.push_back("SubProcess:" + name);
    }

    void buildWorkUnit(const std::string& name) override {
        events.push_back("WorkUnit:" + name);
    }

    void endGroup() override {
        events.push_back("endGroup");
    }

    Handler* getResult() override {
        return nullptr;
    }

    std::vector<WorkGroup*> getWorkGroups() override {
        return std::vector<WorkGroup*>();   // the spy builds nothing, so there are no groups
    }
};
} // namespace

TEST(BuilderTest, DirectorHandlesNullBuilderAndMultiGroupChainSequence) {
    json blueprint = json::parse(R"({
        "chain": [
            {
                "type": "WorkGroup",
                "name": "Group A",
                "children": [
                    { "type": "WorkUnit", "name": "Unit A1" }
                ]
            },
            {
                "type": "Stage",
                "name": "Stage B",
                "children": [
                    {
                        "type": "SubProcess",
                        "name": "Sub B1",
                        "children": [
                            { "type": "WorkUnit", "name": "Unit B1a" }
                        ]
                    }
                ]
            }
        ]
    })");

    // Null builder should not crash
    Director nullDirector(nullptr);
    EXPECT_NO_THROW(nullDirector.construct(blueprint));

    // Verify exact builder call sequence on multi-node chain
    RecordingChainBuilder spyBuilder;
    Director director(&spyBuilder);
    director.construct(blueprint);

    ASSERT_EQ(spyBuilder.events.size(), 9u);
    EXPECT_EQ(spyBuilder.events[0], "reset");
    EXPECT_EQ(spyBuilder.events[1], "WorkGroup:Group A");
    EXPECT_EQ(spyBuilder.events[2], "WorkUnit:Unit A1");
    EXPECT_EQ(spyBuilder.events[3], "endGroup");
    EXPECT_EQ(spyBuilder.events[4], "Stage:Stage B");
    EXPECT_EQ(spyBuilder.events[5], "SubProcess:Sub B1");
    EXPECT_EQ(spyBuilder.events[6], "WorkUnit:Unit B1a");
    EXPECT_EQ(spyBuilder.events[7], "endGroup");
    EXPECT_EQ(spyBuilder.events[8], "endGroup");
}