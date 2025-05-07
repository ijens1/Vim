#ifndef RX_H
#define RX_H
#include <vector>

class Observer {
  public:
    virtual void notify() = 0;
};

class Subject {
    std::vector<Observer*> observers;
  public:
    Subject() {}
    Subject(const Subject&) = default;
    Subject(Subject&&) = default;
    Subject& operator=(const Subject&) = default;
    Subject& operator=(Subject&&) = default;
    virtual ~Subject() = 0;

    void notifyObservers();

    void attach(Observer &observer);

    void detach(Observer &observer);
};

#endif
