#include <algorithm>
#include <ctime>

#include "Framework/Event.hpp"
#include "Framework/Time.hpp"

EventHandler::EventHandler() : myIterations(0), myNSPerCheck(1000) {
  myOldTime = Time::seconds();
}

EventBase::~EventBase() {
  // Remove any events in the EventHandler for this object
  myEventHandler.Remove(this);
}

// Checks for a expired events.
void EventHandler::Check() {
  myIterations = std::max<decltype(myIterations)>(myIterations + 1, 1);
  auto now = Time::seconds();
  if (now > myOldTime) {
    constexpr NanoSeconds NS_PER_SECOND{1000000000L};
    NanoSeconds delta_ns = (now - myOldTime) * NS_PER_SECOND;
    myNSPerCheck = std::max<NanoSeconds>(delta_ns / myIterations, 1);
    myOldTime = now;
    myIterations = 0;
  }
  if (myEvents.empty()) {
    return;
  }
  std::vector<Event> runnables;
  auto nsec = myNSPerCheck;
  do {
    runnables.clear();
    auto it = myEvents.begin();
    for (; it != myEvents.end() && it->delta_time <= nsec; ++it) {
      nsec -= it->delta_time;
      runnables.push_back(*it);
    }
    myEvents.erase(myEvents.begin(), it);
    if (!myEvents.empty()) {
      myEvents[0].delta_time -= nsec;
    }
    for (auto event : runnables) { event.Dispatch(); }
  } while (!runnables.empty());
}

// Adds an event to the event list.
void EventHandler::Add(EventBase *object, int data, void *pointer, USeconds etime) {
  Event event{object, data, pointer, etime};
  event.delta_time = etime * 1000;  // Time in nanoseconds.
  NanoSeconds adelta = 0;
  auto it = myEvents.begin();
  for (; it != myEvents.end(); ++it) {
    if (etime < (adelta + it->delta_time)) { break; }
    adelta += it->delta_time;
  }
  event.delta_time -= adelta;
  if (it != myEvents.end()) {
    it->delta_time -= event.delta_time;
  }
  myEvents.insert(it, event);
}

// Removes events for the given object.
void EventHandler::Remove(EventBase *object) {
  auto end = std::remove_if(myEvents.begin(), myEvents.end(),
                           [object](Event &event) {
                             return event.Owner() == object;
                           });
  myEvents.erase(end, myEvents.end());
}
