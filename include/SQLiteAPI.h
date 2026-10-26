#ifndef SQLITEAPI_H
#define SQLITEAPI_H

#include <string>
#include <vector>

class SQLiteAPI {
private:
    std::string dbFilePath;
    bool connected;

public:
    SQLiteAPI(std::string filePath = "tividy_local.db");
    virtual ~SQLiteAPI();

    virtual bool openConnection();
    virtual void closeConnection();
    virtual int runStatement(std::string sqlQuery);
    virtual std::vector<std::string> queryRows(std::string sqlQuery);
    virtual bool isOpen() const;
    std::string getDbFilePath() const;
};

#endif