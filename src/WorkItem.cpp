#include "WorkItem.h"

WorkItem::WorkItem() {}

WorkItem::WorkItem(const WorkItem& other) : payloadData(other.payloadData) {}

void WorkItem::setData(const std::string& key, const std::string& value) {
    payloadData[key] = value;
}

std::string WorkItem::getData(const std::string& key) const {
    auto it = payloadData.find(key);
    if (it != payloadData.end()) {
        return it->second;
    }
    return "";
}

bool WorkItem::hasData(const std::string& key) const {
    return payloadData.find(key) != payloadData.end();
}

const std::map<std::string, std::string>& WorkItem::getPayloadData() const {
    return payloadData;
}
