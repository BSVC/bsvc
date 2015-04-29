///////////////////////////////////////////////////////////////////////////////
//
// Event.hxx
//
//   This class maintains a queue of events requested by EventBase derived
// objects.
//
//
// BSVC "A Microprocessor Simulation Framework"
// Copyright (c) 1993
// By: Bradford W. Mott
// August 11,1993
//
///////////////////////////////////////////////////////////////////////////////
// $Id: Event.hxx,v 1.1 1996/08/02 14:51:50 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#ifndef EVENT_HXX
#define EVENT_HXX

#include <string>

#ifdef USE_STD
  using namespace std;
#endif

class EventHandler;

///////////////////////////////////////////////////////////////////////////////
// Should be the base class for any class that is going to register
// events with the event handler
///////////////////////////////////////////////////////////////////////////////
class EventBase {
  public:
    // Constructor
    EventBase(EventHandler& handler)
        : myEventHandler(handler) 
    {}

    // Destructor
    virtual ~EventBase();

    // Called when a registered event is dispatched
    virtual void EventCallback(long data, void* pointer) = 0;

  private:
    EventHandler& myEventHandler;
};


class EventHandler {
  public:
    // Constructor
    EventHandler();

    // Check for any expired events
    void Check();

    // Add an event to the event list
    void Add(EventBase* object, long data, void* pointer, long time);

    // Remove events for the given object
    void Remove(EventBase* object); 

  private:
    // The event class
    class Event {
      public:
        Event(EventBase* object, long data, void* pointer, unsigned long t) 
            : total_time(t), next(0),
              myObject(object), myPointer(pointer), myData(data)
        {};

        // Dispatch the event by calling the object's callback routine
        void Dispatch() { myObject->EventCallback(myData, myPointer); }

        // Return the owning object
        EventBase* Owner() { return myObject; }

        // Total amount of time to elapse before the event
        const long total_time;

        // Time left before the event occurs
        long delta_time;

        // Pointer to the next event
        Event *next;

      private:
        // The object that owns this event
        EventBase* myObject;

        // Data to be passed to the callback method
        void* myPointer; 
        long  myData;
    };
 
    // Linked list of events
    Event* myEvents;

    // Number of calls since last time update
    long myIterations;

    // Last usec_per_check update time
    long myOldTime;

    // Average micro-seconds per call to Check
    long myMicrosecondsPerCheck;
};
#endif

