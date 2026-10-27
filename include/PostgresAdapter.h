#ifndef POSTGRESADAPTER_H
#define POSTGRESADAPTER_H

#include "ExternalDatabase.h"
#include "PostgresAPI.h"
#include <string>
#include <vector>

class PostgresAdapter : public ExternalDatabase {
private:
    PostgresAPI* pgApi;//
    std::string tableName;
    bool ownsAPI;

    std::string rmvQuotes(const std::string& input) const;

public:
    PostgresAdapter(PostgresAPI* pgApi, std::string table = "workflow_audit_log", bool takeOwnership = true);
    virtual ~PostgresAdapter();

    virtual bool saveRecord(int taskID, std::string taskName, std::string state, std::string payload) override;
    virtual std::string getRecordState(std::string taskName) override;
    virtual std::vector<std::string> fetchAllRecords() override;
    virtual bool isConnected() override;
};

#endif