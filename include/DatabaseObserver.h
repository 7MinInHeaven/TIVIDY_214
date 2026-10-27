#ifndef DATABASEOBSERVER_H
#define DATABASEOBSERVER_H

#include "WorkflowObserver.h"
#include "ExternalDatabase.h"
#include "Composite.h"
#include <string>

class DatabaseObserver : public WorkflowObserver {
private:
    WorkComponent* subject;
    ExternalDatabase* database;
    ExternalDatabase* fallbackDatabase;
    std::string lastSyncedState;

public:
    DatabaseObserver(WorkComponent* subj, ExternalDatabase* db, ExternalDatabase* fallbackDb = nullptr);
    virtual ~DatabaseObserver();

    virtual void update() override;
    std::string getLastSyncedState() const;
    void setSubject(WorkComponent* s);
    void setDatabaseStrategy(ExternalDatabase* db);
    ExternalDatabase* selectBestStrategy() const;
    int syncDatabases(ExternalDatabase* fromDb, ExternalDatabase* toDb);
};

#endif