#include "MailjetEmailAdapter.h"
#include "NotificationBridge.h"
#include "WorkItem.h"
#include "WorkflowEngineFacade.h"
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

/**
 * @brief Runs the console-based runtime workflow selector.
 */
int main() {
  cout << "========================================\n";
  cout << "   TIVIDY: Generic Workflow Engine      \n";
  cout << "========================================\n\n";

  WorkflowEngineFacade facade;
  while (true) {
    const vector<string> discoveryErrors = facade.refreshScenarios();
    for (const string& error : discoveryErrors) {
      cerr << "[Scenario warning] " << error << "\n";
    }

    const vector<WorkflowScenario>& scenarios = facade.listScenarios();
    if (scenarios.empty()) {
      cerr << "No valid workflow scenarios are available.\n";
      return 1;
    }

    cout << "\nAvailable workflow scenarios:\n";
    for (size_t index = 0; index < scenarios.size(); ++index) {
      cout << index + 1 << ". " << scenarios[index].label << "\n";
    }
    cout << "Select a scenario (1-" << scenarios.size()
         << ", 0 to exit): ";

    size_t selection = 0;
    if (!(cin >> selection)) {
      cerr << "Invalid scenario selection.\n";
      return 1;
    }
    if (selection == 0) {
      break;
    }
    if (selection > scenarios.size()) {
      cerr << "Invalid scenario selection.\n";
      continue;
    }

    WorkItem item;
    facade.submitRequest(item, scenarios[selection - 1].path);

    cout << "\nRun another workflow? (y/n): ";
    char again = 'n';
    cin >> again;
    if (again != 'y' && again != 'Y') {
      break;
    }
  }

  return 0;
}
