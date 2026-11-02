#include "WebServer.h"
#include "WorkItem.h"
#include "WorkflowEngineFacade.h"
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char **argv) {
  cout << "========================================\n";
  cout << "   TIVIDY: Generic Workflow Engine      \n";
  cout << "========================================\n\n";

  WorkflowEngineFacade facade;

  //the siteOfWeb  ./build/bin/tividy_app --web   (run from the repository root so ./web and ./src are found)
  if (argc > 1 && string(argv[1]) == "--web") {
    const char *host = getenv("TIVIDY_HOST");   // EVERYONE LOOK HERE set to 0.0.0.0 inside Docker
    WebServer server(&facade, "web");
    return server.run(host != nullptr ? host : "127.0.0.1", 8080) ? 0 : 1;
  }

  //original lame version
  WorkItem item;
  facade.submitRequest(item);
  return 0;
}