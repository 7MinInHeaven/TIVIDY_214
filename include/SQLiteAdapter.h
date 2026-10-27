#ifndef SQLITEADAPTER_H
#define SQLITEADAPTER_H

#include "ExternalDatabase.h"
#include "SQLiteAPI.h"
#include <string>
#include <vector>

class SQLiteAdapter : public ExternalDatabase {
private:
    SQLiteAPI* api;
    std::string tableName;
    bool ownsAPI;

    std::string rmvQuotes(const std::string& input) const;

public:
    SQLiteAdapter(SQLiteAPI* sqApi, std::string table = "workflow_audit_log", bool takeOwnership = true);
    virtual ~SQLiteAdapter();

    virtual bool saveRecord(int taskID, std::string taskName, std::string state, std::string payload) override;
    virtual std::string getRecordState(std::string taskName) override;
    virtual std::vector<std::string> fetchAllRecords() override;
    virtual bool isConnected() override;
};

#endif