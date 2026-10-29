#include "MailjetEmailAdapter.h"
#include "NotificationBridge.h"
#include "WorkItem.h"
#include "WorkflowEngineFacade.h"
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>

using namespace std;

namespace {

/**
 * @brief Limits retries for invalid console scenario selections.
 */
const int kMaxSelectionAttempts = 3;

} // namespace

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
      cout << "\nNo valid workflow scenarios are available.\n";
    } else {
      cout << "\nAvailable workflow scenarios:\n";
      for (size_t index = 0; index < scenarios.size(); ++index) {
        cout << index + 1 << ". " << scenarios[index].label << "\n";
      }
    }
    cout << "Select a scenario (1-" << scenarios.size()
         << ", r to refresh, 0 to exit): ";

    size_t selection = 0;
    bool validSelection = false;
    bool refreshRequested = false;
    for (int attempt = 1; attempt <= kMaxSelectionAttempts; ++attempt) {
      string command;
      if (!(cin >> command)) {
        return 1;
      }

      /**
       * @brief Rescans src/ without restarting the running application.
       */
      if (command == "r" || command == "R") {
        refreshRequested = true;
        break;
      }

      char trailingCharacter = '\0';
      istringstream parser(command);
      if (parser >> selection && !(parser >> trailingCharacter) &&
          selection <= scenarios.size()) {
        validSelection = true;
        break;
      }

      cerr << "Invalid scenario selection. Attempt " << attempt << " of "
           << kMaxSelectionAttempts << ".\n";
    }

    if (refreshRequested) {
      continue;
    }

    if (!cin) {
      return 1;
    }

    if (!validSelection) {
      cerr << "Too many invalid scenario selections. Exiting.\n";
      return 1;
    }

    if (selection == 0) {
      break;
    }

    /**
     * @brief Removes the menu choice newline before Director reads line input.
     */
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

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
