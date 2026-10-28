#ifndef WORKITEM_H
#define WORKITEM_H

#include <map>
#include <string>

/**
 * @brief Represents a generic payload data container moving through the
 * workflow.
 */
class WorkItem {
private:
  std::map<std::string, std::string> payloadData;

public:
  WorkItem();
  WorkItem(const WorkItem &other);
  WorkItem(const std::map<std::string, std::string> &initialData);
  virtual ~WorkItem() = default;

  void setData(const std::string &key, const std::string &value);
  std::string getData(const std::string &key) const;
  bool hasData(const std::string &key) const;

  const std::map<std::string, std::string> &getPayloadData() const;
};

#endif // WORKITEM_H
