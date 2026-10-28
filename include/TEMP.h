#ifndef TEMP_H
#define TEMP_H

#include <map>
#include <string>

class WorkComponent;
class WorkGroup;
class WorkUnit;
class SubProcess;
class Stage;

class WorkIterator {
    public:
    WorkIterator() = default;

    void first();
    void next();
    bool isDone();
    WorkComponent* getCurrent();
};

class DepthFirstIterator : public WorkIterator {
    private:
    WorkComponent* root;

    public:
    DepthFirstIterator(WorkComponent* root);
};

#endif /* TEMP_H */

