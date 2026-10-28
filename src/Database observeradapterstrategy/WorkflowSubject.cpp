#include "WorkflowSubject.h"
#include <algorithm>

WorkflowSubject::WorkflowSubject() {}

WorkflowSubject::~WorkflowSubject() {
    observerList.clear();
}

void WorkflowSubject::attach(WorkflowObserver* observer) {
    if (observer == nullptr) {
        return;
    }
    // duplicate check
    if (std::find(observerList.begin(), observerList.end(), observer) == observerList.end()) {
        observerList.push_back(observer);
    }
}

void WorkflowSubject::detach(WorkflowObserver* observer) {
    auto it = std::find(observerList.begin(), observerList.end(), observer);
    if (it != observerList.end()) {
        observerList.erase(it);
    }
}

void WorkflowSubject::notify() {
    WorkComponent* comp = dynamic_cast<WorkComponent*>(this);
    for (size_t i = 0; i < observers.size(); ++i) {
        if (observers[i] != nullptr) {
            if (comp != nullptr) {
                observers[i]->setSubject(comp);
            }
            observers[i]->update();
        }
    }
}
