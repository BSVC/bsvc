///////////////////////////////////////////////////////////////////////////////
//
// Event.cxx
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
// $Id: Event.cxx,v 1.1 1996/08/02 14:51:57 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#include "Event.hxx"
#include "Time.hxx"

///////////////////////////////////////////////////////////////////////////////
// EventBase Destructor
///////////////////////////////////////////////////////////////////////////////
EventBase::~EventBase()
{ 
  // Remove any events in the EventHandler for this object
  myEventHandler.Remove(this);
}

///////////////////////////////////////////////////////////////////////////////
// EventHandler Constructor
///////////////////////////////////////////////////////////////////////////////
EventHandler::EventHandler()
    : myEvents(0),
      myIterations(0),
      myMicrosecondsPerCheck(20)
{
  Time time;

  myOldTime = time.seconds();
}


///////////////////////////////////////////////////////////////////////////////
// Check for any expired events
///////////////////////////////////////////////////////////////////////////////
void EventHandler::Check()
{
  long new_time;
  Time time;                         

  ++myIterations;                    // This is another iteration
  new_time = time.seconds();         // Get the current time in seconds

  // Check to see if it's time to update myMicrosecondsPerCheck
  if(new_time != myOldTime)
  {
    myMicrosecondsPerCheck = ((new_time - myOldTime) * 1000000) / myIterations;
    myOldTime = new_time;
    myIterations = 0;
  }

  // If the list isn't empty    
  if(myEvents != 0)
  {
    // Decrement delta_time of the earliest event
    if((myEvents->delta_time -= myMicrosecondsPerCheck) <= 0)
    {
      Event* tmp = myEvents;

      myEvents = myEvents->next;
      if(myEvents != 0)
        myEvents->delta_time += tmp->delta_time;  // In case of some overrun

      // This has to be done last because the callback might add new events :-)
      tmp->Dispatch();
      delete tmp;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Add an event to the event list
///////////////////////////////////////////////////////////////////////////////
void EventHandler::Add(EventBase *object, long data, void* pointer, long time)
{
  Event* event = new Event(object, data, pointer, time);

  if(myEvents == 0)
  {
    myEvents = event;
    event->delta_time = time;
  }
  else if(time < myEvents->delta_time)
  {
    event->next = myEvents;
    event->delta_time = time;
    myEvents->delta_time -= time;
    myEvents = event;
  }
  else
  {
    Event* p = myEvents;
    long tmp_delta = myEvents->delta_time;

    while(p->next != 0)
    {
      if(time < (tmp_delta + p->next->delta_time))
      {
        event->delta_time = time-tmp_delta;
        p->next->delta_time -= event->delta_time;
        event->next = p->next;
        p->next = event;
        break;
      }
      else
      {
        p = p->next;
        tmp_delta += p->delta_time;
      }
    }

    if(p->next == 0)
    {
      p->next = event;
      event->delta_time = time - tmp_delta;
    }
  } 
} 

///////////////////////////////////////////////////////////////////////////////
// Remove events for the given object
///////////////////////////////////////////////////////////////////////////////
void EventHandler::Remove(EventBase* object)
{
  bool done = false;
  Event* p;
  Event* q;

  // Keep looping until no events are deleted
  while(!done)
  {
    done = true;

    // Loop through all of the events removing the first one for this object
    for(q = 0, p = myEvents; p != 0; q = p, p = p->next)
    {
      // Check to see if it belongs to object
      if(p->Owner() == object)
      {
        // Delete the event from the event myEvents
        if(q == 0) 
          myEvents = myEvents->next;
        else
          q->next = p->next;
        delete p;
        done = false;
        break;
      }
    }
  }
}

