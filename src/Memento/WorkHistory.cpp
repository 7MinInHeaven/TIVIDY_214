#include "Memento.h"

WorkHistory::~WorkHistory() {
  for (WorkMemento *memento : mementos) {
    delete memento;
  }
}

void WorkHistory::addMemento(WorkMemento *memento) {
  mementos.push_back(memento);
}

bool WorkHistory::isEmpty() const { return mementos.empty(); }

WorkMemento *WorkHistory::getLatestMemento() const {
  if (mementos.empty()) {
    return nullptr;
  }
  return mementos.back();
}

void WorkHistory::popLatest() {
  if (!mementos.empty()) {
    delete mementos.back();
    mementos.pop_back();
  }
}
