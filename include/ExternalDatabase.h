#ifndef EXTERNALDATABASE_H
#define EXTERNALDATABASE_H

#include <string>
#include <vector>

class ExternalDatabase{
    public:
    virtual bool saveRecord(int taskID, std::string taskName, std::string state, std::string payload)=0;
    virtual std::string getRecordState(std::string taskName) = 0;
    virtual std::vector<std::string> fetchAllRecords() = 0;
    virtual bool isConnected() =0;
    virtual ~ExternalDatabase(){};


};


#endif