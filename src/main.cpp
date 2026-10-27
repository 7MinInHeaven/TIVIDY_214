// #include "MailjetEmailAdapter.h"
// #include "NotificationBridge.h"
// #include "WorkItem.h"
// #include "WorkflowEngineFacade.h"
// #include <cstdlib>
// #include <curl/curl.h>
// #include <iostream>
// #include <memory>

// using namespace std;

// int main_2() {
//   cout << "========================================\n";
//   cout << "   TIVIDY: Generic Workflow Engine      \n";
//   cout << "========================================\n\n";

//   WorkflowEngineFacade facade;
//   WorkItem item;
//   facade.submitRequest(item);

//   return 0;
// }

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "Composite.h"
#include "WorkItem.h"
#include "ChainBuilder.h"
#include "WorkflowBuilder.h"
#include "Director.h"

using json = nlohmann::json;

int main() {
    std::string path = "src/blueprint.json";
    std::ifstream file(path);
    if (!file) {
        std::cerr << "couldn't find json file: " << path << "\n";
        return 1;
    }

    json data;
    file >> data;

    //Create a WorkItem
    WorkItem* item = new WorkItem();

    WorkflowBuilder builder;
    Director director(&builder, item, false);
    director.construct(data);
    Handler* chainHead = builder.getResult();

    std::cout << "\n=== Chain of Responsibility Execution (handleRequest across chained WorkGroups) ===\n";
    if (chainHead != nullptr) {
        chainHead->handleRequest(*item);
    }

    std::cout << "\n=== Work Item Payload ===\n";
    for (const auto& pair : item->getPayloadData()) {
        std::cout << pair.first << " -> " << pair.second << "\n";
    }

    delete item;

    return 0;
}