#ifndef WEBSERVER_H
#define WEBSERVER_H

// The website: a thin HTTP layer over the engine.
// Every action goes browser -> WebServer -> AccessControlProxy -> WorkflowEngineFacade.

#include <memory>
#include <string>

class WorkflowEngineFacade;

class WebServer {
public:
  /** @param facade The real engine not owned I believe @param webRoot Folder with index.html. */
  WebServer(WorkflowEngineFacade *facade, const std::string &webRoot);
  ~WebServer();

  /** @brief Serves the page and the /api routes until stopped. @return false if it could not start. */
  bool run(const std::string &host, int port);

private:
  struct Site; // all state lives in WebServer.cpp
  std::unique_ptr<Site> site;
};

#endif