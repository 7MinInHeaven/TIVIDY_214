#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "WorkflowScenarioCatalog.h"

/**
 * @brief Verifies discovery of valid JSON files in the source directory.
 */
TEST(WorkflowScenarioCatalogTest, DiscoversExistingBlueprints) {
    WorkflowScenarioCatalog catalog("src");

    catalog.refresh();

    bool lifeScenarioFound = false;
    bool serviceScenarioFound = false;
    for (const WorkflowScenario& scenario : catalog.scenarios()) {
        lifeScenarioFound =
            lifeScenarioFound || scenario.path == "src/life_blueprint.json";
        serviceScenarioFound =
            serviceScenarioFound ||
            scenario.path == "src/service_blueprint.json";
    }
    EXPECT_TRUE(lifeScenarioFound);
    EXPECT_TRUE(serviceScenarioFound);
}

/**
 * @brief Verifies that a console selection returns the requested scenario.
 */
TEST(WorkflowScenarioCatalogTest, SelectsScenarioFromConsoleInput) {
    WorkflowScenarioCatalog catalog("src");
    catalog.refresh();

    std::size_t serviceIndex = 0;
    for (std::size_t index = 0; index < catalog.scenarios().size(); ++index) {
        if (catalog.scenarios()[index].path == "src/service_blueprint.json") {
            serviceIndex = index + 1;
            break;
        }
    }
    ASSERT_NE(serviceIndex, 0u);

    std::istringstream input(std::to_string(serviceIndex) + "\n");
    std::ostringstream output;
    WorkflowScenario selected;

    EXPECT_TRUE(catalog.selectFromConsole(input, output, selected));
    EXPECT_EQ(selected.id, "src/service_blueprint.json");
}

/**
 * @brief Verifies that invalid console selections are rejected.
 */
TEST(WorkflowScenarioCatalogTest, RejectsInvalidConsoleSelection) {
    WorkflowScenarioCatalog catalog("src");
    catalog.refresh();

    std::istringstream input("99\n");
    std::ostringstream output;
    WorkflowScenario selected;

    EXPECT_FALSE(catalog.selectFromConsole(input, output, selected));
    EXPECT_NE(output.str().find("Invalid scenario selection"), std::string::npos);
}

/**
 * @brief Verifies that missing directories produce an explicit discovery error.
 */
TEST(WorkflowScenarioCatalogTest, ReportsMissingDirectory) {
    WorkflowScenarioCatalog catalog("missing-scenarios");

    const std::vector<std::string> errors = catalog.refresh();

    ASSERT_EQ(errors.size(), 1u);
    EXPECT_NE(errors[0].find("cannot open scenario directory"),
              std::string::npos);
    EXPECT_TRUE(catalog.scenarios().empty());
}
