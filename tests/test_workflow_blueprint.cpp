#include <fstream>
#include <string>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "WorkflowBlueprint.h"

namespace {

/**
 * @brief Provides a valid nested blueprint for validation tests.
 */
const char* validBlueprint() {
    return R"({
        "workflow_name": "Test workflow",
        "required_inputs": [
            {"key": "request_id", "prompt": "Request ID"}
        ],
        "chain": [
            {
                "type": "WorkGroup",
                "name": "Intake",
                "children": [
                    {"type": "WorkUnit", "name": "Validate request"},
                    {
                        "type": "Stage",
                        "name": "Assessment",
                        "children": [
                            {
                                "type": "SubProcess",
                                "name": "Checks",
                                "children": [
                                    {"type": "WorkUnit", "name": "Check evidence"}
                                ]
                            }
                        ]
                    }
                ]
            }
        ]
    })";
}

/**
 * @brief Verifies that a valid blueprint is accepted.
 */
TEST(WorkflowBlueprintTest, AcceptsValidBlueprint) {
    const nlohmann::json blueprint = nlohmann::json::parse(validBlueprint());

    EXPECT_NO_THROW(WorkflowBlueprint::validate(blueprint));
}

/**
 * @brief Verifies that a missing workflow name is rejected.
 */
TEST(WorkflowBlueprintTest, RejectsMissingWorkflowName) {
    nlohmann::json blueprint = nlohmann::json::parse(validBlueprint());
    blueprint.erase("workflow_name");

    EXPECT_THROW(WorkflowBlueprint::validate(blueprint),
                 BlueprintValidationError);
}

/**
 * @brief Verifies that an empty chain is rejected.
 */
TEST(WorkflowBlueprintTest, RejectsEmptyChain) {
    nlohmann::json blueprint = nlohmann::json::parse(validBlueprint());
    blueprint["chain"] = nlohmann::json::array();

    EXPECT_THROW(WorkflowBlueprint::validate(blueprint),
                 BlueprintValidationError);
}

/**
 * @brief Verifies that root work units are rejected.
 */
TEST(WorkflowBlueprintTest, RejectsRootWorkUnit) {
    nlohmann::json blueprint = nlohmann::json::parse(validBlueprint());
    blueprint["chain"] = {{{"type", "WorkUnit"}, {"name", "Invalid root"}}};

    EXPECT_THROW(WorkflowBlueprint::validate(blueprint),
                 BlueprintValidationError);
}

/**
 * @brief Verifies that unsupported node types are rejected.
 */
TEST(WorkflowBlueprintTest, RejectsUnsupportedNodeType) {
    nlohmann::json blueprint = nlohmann::json::parse(validBlueprint());
    blueprint["chain"][0]["children"][0]["type"] = "CustomHandler";

    EXPECT_THROW(WorkflowBlueprint::validate(blueprint),
                 BlueprintValidationError);
}

/**
 * @brief Verifies that duplicate sibling names are rejected.
 */
TEST(WorkflowBlueprintTest, RejectsDuplicateSiblingNames) {
    nlohmann::json blueprint = nlohmann::json::parse(validBlueprint());
    blueprint["chain"][0]["children"].push_back(
        {{"type", "WorkUnit"}, {"name", "Validate request"}});

    EXPECT_THROW(WorkflowBlueprint::validate(blueprint),
                 BlueprintValidationError);
}

/**
 * @brief Verifies that duplicate input keys are rejected.
 */
TEST(WorkflowBlueprintTest, RejectsDuplicateInputKeys) {
    nlohmann::json blueprint = nlohmann::json::parse(validBlueprint());
    blueprint["required_inputs"].push_back(
        {{"key", "request_id"}, {"prompt", "Again"}});

    EXPECT_THROW(WorkflowBlueprint::validate(blueprint),
                 BlueprintValidationError);
}

/**
 * @brief Verifies that supported action and capability metadata is accepted.
 */
TEST(WorkflowBlueprintTest, AcceptsSupportedActionMetadata) {
    nlohmann::json blueprint = nlohmann::json::parse(validBlueprint());
    blueprint["chain"][0]["children"][0]["action"] = "workflow.log";
    blueprint["capabilities"] = {"sqlite_audit", "app_notification"};

    EXPECT_NO_THROW(WorkflowBlueprint::validate(blueprint));
}

/**
 * @brief Verifies that unsupported action metadata is rejected.
 */
TEST(WorkflowBlueprintTest, RejectsUnsupportedActionMetadata) {
    nlohmann::json blueprint = nlohmann::json::parse(validBlueprint());
    blueprint["chain"][0]["children"][0]["action"] =
        "job_application.run_code";

    EXPECT_THROW(WorkflowBlueprint::validate(blueprint),
                 BlueprintValidationError);
}

/**
 * @brief Verifies that unsupported capability metadata is rejected.
 */
TEST(WorkflowBlueprintTest, RejectsUnsupportedCapabilityMetadata) {
    nlohmann::json blueprint = nlohmann::json::parse(validBlueprint());
    blueprint["capabilities"] = {"unknown_service"};

    EXPECT_THROW(WorkflowBlueprint::validate(blueprint),
                 BlueprintValidationError);
}

/**
 * @brief Verifies that the existing blueprints are loadable.
 */
TEST(WorkflowBlueprintTest, LoadsAndValidatesExistingBlueprint) {
    EXPECT_NO_THROW(WorkflowBlueprint::loadAndValidate(
        "src/service_blueprint.json"));
    EXPECT_NO_THROW(WorkflowBlueprint::loadAndValidate(
        "src/life_blueprint.json"));
}

/**
 * @brief Verifies that missing blueprint files report an error.
 */
TEST(WorkflowBlueprintTest, ReportsMissingFile) {
    EXPECT_THROW(WorkflowBlueprint::loadAndValidate(
                     "src/does_not_exist.json"),
                 BlueprintValidationError);
}

} // namespace
