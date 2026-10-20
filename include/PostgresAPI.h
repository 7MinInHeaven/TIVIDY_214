#ifndef POSTGRESAPI_H
#define POSTGRESAPI_H

#include <string>
#include <vector>

class PostgresAPI {
private:
    std::string connString;
    bool connected;

public:
    PostgresAPI(std::string connStr = "");
    virtual ~PostgresAPI();

    virtual bool connect();
    virtual void disconnect();
    virtual int executeTransaction(std::string sqlQuery);
    virtual std::vector<std::string> executeSelect(std::string sqlQuery);
    virtual bool getConnectionStatus() const;
    std::string getConnString() const;
};

#endif