#include "WorkItem.h"
#include "WorkflowEngineFacade.h"
#include <gtest/gtest.h>

/**
 * @brief Verifies that the existing default facade entry point still executes.
 */
TEST(FacadeTest, SubmitsRequestWithoutErrors) {
  WorkflowEngineFacade facade;
  WorkItem item;
  item.setData("id", "TEST-REQ-01");
  item.setData("category", "MotorVehicleClaim");
  item.setData("email", "client@tividy.co.za");

  // Verifies that facade executes without throwing exceptions or crashing
  EXPECT_NO_THROW(facade.submitRequest(item));
}

/**
 * @brief Verifies that a selected runtime blueprint can execute through Facade.
 */
TEST(FacadeTest, SubmitsSelectedBlueprintWithoutErrors) {
  WorkflowEngineFacade facade;
  WorkItem item;
  item.setData("id", "TEST-RUNTIME-01");
  item.setData("email", "client@tividy.co.za");

  EXPECT_NO_THROW(facade.submitRequest(item, "src/life_blueprint.json"));
}

/**
 * @brief Verifies that the facade exposes validated runtime scenarios.
 */
TEST(FacadeTest, ListsRuntimeScenarios) {
  WorkflowEngineFacade facade;

  facade.refreshScenarios();

  bool lifeScenarioFound = false;
  bool serviceScenarioFound = false;
  for (const WorkflowScenario& scenario : facade.listScenarios()) {
    lifeScenarioFound =
        lifeScenarioFound || scenario.path == "src/life_blueprint.json";
    serviceScenarioFound =
        serviceScenarioFound ||
        scenario.path == "src/service_blueprint.json";
  }
  EXPECT_TRUE(lifeScenarioFound);
  EXPECT_TRUE(serviceScenarioFound);
}
