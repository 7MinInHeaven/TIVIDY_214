#include "MailjetEmailAdapter.h"
#include "NotificationBridge.h"
#include "WorkItem.h"
#include "WorkflowEngineFacade.h"
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <memory>

using namespace std;

int main() {
  cout << "========================================\n";
  cout << "   TIVIDY: Generic Workflow Engine      \n";
  cout << "========================================\n\n";

  WorkflowEngineFacade facade;
  WorkItem item;
  facade.submitRequest(item);

  return 0;
}

// #include <iostream>
// #include <fstream>
// #include <string>
// #include <vector>
// #include <nlohmann/json.hpp>
// #include "Composite.h"
// #include "WorkItem.h"
// #include "ChainBuilder.h"
// #include "WorkflowBuilder.h"
// #include "Director.h"

// using json = nlohmann::json;

// std::string getComponentTypeTag(WorkComponent* comp) {
//     if (dynamic_cast<Stage*>(comp) != nullptr) {
//         return "[Stage]";
//     }
//     if (dynamic_cast<SubProcess*>(comp) != nullptr) {
//         return "[SubProcess]";
//     }
//     if (dynamic_cast<WorkGroup*>(comp) != nullptr) {
//         return "[WorkGroup]";
//     }
//     return "[WorkUnit]";
// }


// int main() {
//     std::string path = "src/blueprint.json";
//     std::ifstream file(path);
//     if (!file) {
//         std::cerr << "couldn't find json file: " << path << "\n";
//         return 1;
//     }

//     json data;
//     file >> data;

//     // 0. Create a WorkItem
//     WorkItem* item = new WorkItem();

//     WorkflowBuilder builder;
//     Director director(&builder, item, false);
//     director.construct(data);
//     Handler* chainHead = builder.getResult();

//     std::cout << "\n=== Chain of Responsibility Execution (handleRequest across chained WorkGroups) ===\n";
//     if (chainHead != nullptr) {
//         chainHead->handleRequest(*item);
//     }

//     std::cout << "\n=== Work Item Payload ===\n";
//     for (const auto& pair : item->getPayloadData()) {
//         std::cout << pair.first << " -> " << pair.second << "\n";
//     }

//     delete item;

//     return 0;
// }