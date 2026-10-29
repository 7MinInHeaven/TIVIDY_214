#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "WorkflowScenarioCatalog.h"

/**
 * @brief Verifies discovery of valid JSON files in the source directory.
 */
TEST(WorkflowScenarioCatalogTest, DiscoversExistingBlueprints) {
    WorkflowScenarioCatalog catalog("src");

    const std::vector<std::string> errors = catalog.refresh();

    EXPECT_TRUE(errors.empty());
    ASSERT_EQ(catalog.scenarios().size(), 2u);
    EXPECT_EQ(catalog.scenarios()[0].path, "src/life_blueprint.json");
    EXPECT_EQ(catalog.scenarios()[1].path, "src/service_blueprint.json");
}

/**
 * @brief Verifies that a console selection returns the requested scenario.
 */
TEST(WorkflowScenarioCatalogTest, SelectsScenarioFromConsoleInput) {
    WorkflowScenarioCatalog catalog("src");
    ASSERT_TRUE(catalog.refresh().empty());

    std::istringstream input("2\n");
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
    ASSERT_TRUE(catalog.refresh().empty());

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
