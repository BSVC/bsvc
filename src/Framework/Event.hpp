//
// Maintains a queue of events requested by EventBase derived objects.
//

#ifndef FRAMEWORK_EVENT_HPP_
#define FRAMEWORK_EVENT_HPP_

#include <cstdint>
#include <ctime>
#include <vector>

#include "Framework/Types.hpp"

class EventHandler;

// The base class for any class that is going to register
// events with the event handler.
class EventBase {
public:
  EventBase(EventHandler &handler) : myEventHandler(handler) { }
  virtual ~EventBase();

  // Called when a registered event is dispatched.
  virtual void EventCallback(int data, void *pointer) = 0;

private:
  EventHandler &myEventHandler;
};

class EventHandler {
public:
  // Constructor
  EventHandler();

  // Checks for expired events.
  void Check();

  // Adds an event to the event list.
  void Add(EventBase *object, int data, void *pointer, USeconds time);

  // Removes events for the given object.
  void Remove(EventBase *object);

private:
  class Event {
  public:
    Event(EventBase *o, int d, void *p, USeconds t)
        : total_time(t), object(o), pointer(p), data(d) { }

    // Dispatches the event by calling the object's callback routine.
    void Dispatch() { object->EventCallback(data, pointer); }

    // Returns the owning object.
    EventBase *Owner() { return object; }

    // Total amount of time to wait before the event.
    NanoSeconds total_time;

    // Time left before the event occurs.
    NanoSeconds delta_time;

  private:
    // The object that owns this event.
    EventBase *object;

    // Data to be passed to the callback method.
    void *pointer;
    int data;
  };

  // Linked list of events.
  std::vector<Event> myEvents;

  // Number of calls since last second.
  std::uint64_t myIterations;

  // Last second Check() was called.
  std::time_t myOldTime;

  // Average nanoseconds per call to Check.
  NanoSeconds myNSPerCheck;
};

#endif  // FRAMEWORK_EVENT_HPP_
