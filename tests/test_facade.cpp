#include "WorkItem.h"
#include "WorkflowEngineFacade.h"
#include <gtest/gtest.h>

TEST(FacadeTest, SubmitsRequestWithoutErrors) {
  WorkflowEngineFacade facade;
  WorkItem item;
  item.setData("id", "TEST-REQ-01");
  item.setData("category", "MotorVehicleClaim");
  item.setData("email", "client@tividy.co.za");

  // Verifies that facade executes without throwing exceptions or crashing
  EXPECT_NO_THROW(facade.submitRequest(item));
}
