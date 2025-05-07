#include "rx.h"

void Subject::notifyObservers() {
  for (Observer *observer : observers) {
    observer->notify();
  }
}

void Subject::attach(Observer &observer) {
  observers.push_back(&observer);
}

void Subject::detach(Observer &observer) {
  for (auto i = observers.begin(); i != observers.end(); ++i) {
    if (*i == &observer) {
      observers.erase(i);
      break;
    }
  }
}

Subject::~Subject() {}
