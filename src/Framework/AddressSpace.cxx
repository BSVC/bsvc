///////////////////////////////////////////////////////////////////////////////
//
// AddressSpace.cxx
//
//   This class maintains a list of devices and provides methods to 
// peek and poke into them.
//
//
// BSVC "A Microprocessor Simulation Framework"
// Copyright (c) 1993
// By: Bradford W. Mott
// October 31,1993 
//
// Adapted to 68360 - DJ - 1996/11
// - device cache added (cache size is 3 for read and same for write)
// - Peek and Poke with size as parameter
//
///////////////////////////////////////////////////////////////////////////////
// $Id: AddressSpace.cxx,v 1.1 1996/08/02 14:48:23 bwmott Exp $
///////////////////////////////////////////////////////////////////////////////

#include "AddressSpace.hxx"
#include "BasicCPU.hxx"
#include "BasicDevice.hxx"

///////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////
AddressSpace::AddressSpace(unsigned long maximumAddress)
    : myMaximumAddress(maximumAddress)
{
  read_device_cache1 = 0;
  read_device_cache2 = 0;
  read_device_cache3 = 0;
  write_device_cache1 = 0;
  write_device_cache2 = 0;
  write_device_cache3 = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////
AddressSpace::~AddressSpace()
{
  // Destroy all of the attached devices
  while(DetachDevice(0));
}

///////////////////////////////////////////////////////////////////////////////
// Attach the given device. Answers true iff successful
///////////////////////////////////////////////////////////////////////////////
bool AddressSpace::AttachDevice(BasicDevice* device)
{
  unsigned long maximum_in_bytes;

  maximum_in_bytes = (MaximumAddress() + 1) * device->CPU().Granularity() - 1;

  if ((device->HighestAddress() >= device->LowestAddress()) &&
      (device->HighestAddress() <= maximum_in_bytes) &&
      (device->LowestAddress() <= maximum_in_bytes))
  {
    // Add device to list of attached devices
    myDevices.push_back(device);

    return true;
  }
  else
  {
    // Couldn't attach device so delete it!
    delete device; 

    return false;
  } 
}

///////////////////////////////////////////////////////////////////////////////
// Detach the indexed device and destroy it. Answers true iff successful
///////////////////////////////////////////////////////////////////////////////
bool AddressSpace::DetachDevice(unsigned int index)
{
  // Make sure it is a valid index
  if(index < NumberOfAttachedDevices()) 
  {
    list<BasicDevice*>::iterator i;
    unsigned int t;

    // Scan to the devices we want to detach
    for(t = 0, i = myDevices.begin(); t < index; ++t, ++i);

    // Update the cache, if necessary
    if(read_device_cache1==(*i))
    {
      read_device_cache1 = read_device_cache2;
      read_device_cache2 = read_device_cache3;
      read_device_cache3 = 0;
    }	  
    else if(read_device_cache2==(*i))
    {		  
      read_device_cache2 = read_device_cache3;
      read_device_cache3 = 0;
    }
    else if(read_device_cache3==(*i))
    {		  
      read_device_cache3 = 0;
    }
    if(write_device_cache1==(*i))
    {
      write_device_cache1 = write_device_cache2;
      write_device_cache2 = write_device_cache3;
      write_device_cache3 = 0;
    }	  
    else if(write_device_cache2==(*i))
    {		  
      write_device_cache2 = write_device_cache3;
      write_device_cache3 = 0;
    }
    else if(write_device_cache3==(*i))
    {		  
      write_device_cache3 = 0;
    }

    // Free the device
    delete *i;

    // Remove the device pointer from the list of attached devices
    myDevices.erase(i);

    return true;
  }
  else
  {
    return false;
  }
} 

///////////////////////////////////////////////////////////////////////////////
// Reset all the attached devices
///////////////////////////////////////////////////////////////////////////////
void AddressSpace::Reset()
{
  list<BasicDevice*>::iterator i;

  for(i = myDevices.begin(); i != myDevices.end(); ++i)
  {
    (*i)->Reset();
  }
}

///////////////////////////////////////////////////////////////////////////////
// Answers the number of attached devices
///////////////////////////////////////////////////////////////////////////////
unsigned int AddressSpace::NumberOfAttachedDevices() const
{
  return myDevices.size();
}

///////////////////////////////////////////////////////////////////////////////
// Get information about the indexed device. Answer true iff successful
///////////////////////////////////////////////////////////////////////////////
bool AddressSpace::GetDeviceInformation(unsigned int index,
    AddressSpace::DeviceInformation& info) const
{
  // Make sure it is a valid index
  if(index < NumberOfAttachedDevices()) 
  {
    list<BasicDevice*>::const_iterator i;
    unsigned int t;

    // Scan to the device
    for(t = 0, i = myDevices.begin(); t < index; ++t, ++i);

    info.name = (*i)->Name();
    info.arguments = (*i)->Arguments();
    info.index = index;

    return true;
  }
  else
  {
    return false;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Find device for read operation (device cache).
///////////////////////////////////////////////////////////////////////////////
BasicDevice* AddressSpace::FindReadDevice(unsigned long addr)
{
  list<BasicDevice*>::iterator i;
  BasicDevice* d = 0;

  if((read_device_cache1 != 0) && (read_device_cache1->CheckMapped(addr)))
  {
    d = read_device_cache1;		
  }
  else if((read_device_cache2 != 0) && (read_device_cache2->CheckMapped(addr)))
  {
    d = read_device_cache2;
    read_device_cache2 = read_device_cache1;
    read_device_cache1 = d;
  }
  else if((read_device_cache3 != 0) && (read_device_cache3->CheckMapped(addr)))
  {
    d = read_device_cache3;
    read_device_cache3 = read_device_cache2;
    read_device_cache2 = read_device_cache1;
    read_device_cache1 = d;				
  }

  if(d == 0)
  {
    // Find the correct device
    for(i = myDevices.begin(); i != myDevices.end(); ++i)
    {
      if((*i)->CheckMapped(addr))
        break;
    }

    // Did we find a device
    if(i != myDevices.end())
    {
      d = (*i);
      read_device_cache3 = read_device_cache2;
      read_device_cache2 = read_device_cache1;
      read_device_cache1 = d;							
    }
  }

  return d;
}

///////////////////////////////////////////////////////////////////////////////
// Find device for write operation (device cache).
///////////////////////////////////////////////////////////////////////////////
BasicDevice* AddressSpace::FindWriteDevice(unsigned long addr)
{
  list<BasicDevice*>::iterator i;
  BasicDevice* d = 0;

  if((write_device_cache1 != 0) && (write_device_cache1->CheckMapped(addr)))
  {
    d = write_device_cache1;
  }
  else if((write_device_cache2 != 0) && 
      (write_device_cache2->CheckMapped(addr)))
  {
    d = write_device_cache2;
    write_device_cache2 = write_device_cache1;
    write_device_cache1 = d;
  }
  else if((write_device_cache3 != 0) && 
      (write_device_cache3->CheckMapped(addr)))
  {
    d = write_device_cache3;
    write_device_cache3 = write_device_cache2;
    write_device_cache2 = write_device_cache1;
    write_device_cache1 = d;
  }

  if(d == 0)
  {
    // Find the correct device
    for(i = myDevices.begin(); i != myDevices.end(); ++i)
    {
      if((*i)->CheckMapped(addr))
        break;
    }

    // Did we find a device
    if(i != myDevices.end())
    {
      d = (*i);
      write_device_cache3 = write_device_cache2;
      write_device_cache2 = write_device_cache1;
      write_device_cache1 = d;
    }
  }

  return d;
}

///////////////////////////////////////////////////////////////////////////////
// Peek the given location.  Answers true iff successful
///////////////////////////////////////////////////////////////////////////////
bool AddressSpace::Peek(unsigned long addr, unsigned char& c)
{
  BasicDevice* d = FindReadDevice(addr);
	
  // Did we find a device
  if(d != 0)
  {
    // Get the byte
    c = d->Peek(addr);
    return true;
  }
  else
  {
    // No device found BUS ERROR, dude!!!
    return false;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Poke the given location.  Answers true iff successful
///////////////////////////////////////////////////////////////////////////////
bool AddressSpace::Poke(unsigned long addr, unsigned char c)
{
  BasicDevice *d = FindWriteDevice(addr);
	
  // Did we find a device
  if(d != 0)
  {
    // Get the byte
    d->Poke(addr, c);
    return true;
  }
  else
  {
    // No device found BUS ERROR, dude!!!
    return false;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Peek a location in the address space with size parameter. Answers true iff
// successful.
///////////////////////////////////////////////////////////////////////////////
bool AddressSpace::Peek(unsigned long addr,unsigned long& data, int size)
{
  BasicDevice* d;
  bool result;
  unsigned char c;

  switch(size)
  {
    case BYTE:
      // Find the correct device
      d = FindReadDevice(addr);

      // Did we find a device
      if(d != 0)
      {
        // Get the byte
        return d->Peek(addr,data,size);
      }
      else
      {
        return false;
      }
      break;

    case WORD:
      // Find the correct device
      d = FindReadDevice(addr);

      // Did we find a device
      if((d != 0) && (d->CheckMapped(addr+1)))
      {
        // Get the word
        return d->Peek(addr,data,size);
      }
      else
      {			
        // try per byte
        result = Peek(addr,c);
        if(!result)
          return result;
        data = c << 8;
        result = Peek(addr+1,c);
        if(!result)
          return result;
        data += c;
        return true;
      }
      break;

      case LONG:
        // Find the correct device
        d = FindReadDevice(addr);

        // Did we find a device
        if((d != 0) && (d->CheckMapped(addr+1))
            && (d->CheckMapped(addr+2)) && (d->CheckMapped(addr+3)))
        {
          // Get the long
          return d->Peek(addr,data,size);
        }
        else
        {
          // try per byte
          result = Peek(addr,c);
          if(!result)
            return result;
          data = c << 24;
          result = Peek(addr+1,c);
          if(!result)
            return result;
          data += c << 16;
          result = Peek(addr+2,c);
          if(!result)
            return result;
          data += c << 8;
          result = Peek(addr+3,c);
          if(!result)
            return result;
          data += c;
          return true;
        }
        break;

      default:
        break;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Poke a location in the address space with size parameter. Answers true
// iff successful.
///////////////////////////////////////////////////////////////////////////////
bool AddressSpace::Poke(unsigned long addr,unsigned long data, int size)
{
  BasicDevice* d;
  bool result;

  switch(size)
  {
    case BYTE:
      // Find the correct device
      d = FindWriteDevice(addr);

      // Did we find a device
      if(d != 0)
      {
        // Poke the byte
        return d->Poke(addr,data,size);
      }
      else
      {
        // No device found BUS ERROR, dude!!!
        return false;
      }
      break;

    case WORD:
      // Find the correct device
      d = FindWriteDevice(addr);

      // Did we find a device
      if((d != 0) && (d->CheckMapped(addr+1)))
      {
        // Poke the word
        return d->Poke(addr,data,size);
      }
      else
      {
        // try per byte
        result = Poke(addr,(unsigned char)(data>>8));
        if(!result)
          return result;
        return Poke(addr+1,(unsigned char)data);
      }
      break;

    case LONG:
      // Find the correct device
      d = FindWriteDevice(addr);
  
      // Did we find a device
      if((d != 0) && (d->CheckMapped(addr+1))
          && (d->CheckMapped(addr+2)) && (d->CheckMapped(addr+3)))
      {
        // Poke the long
        return d->Poke(addr,data,size);
      }
      else
      {
        // try per byte
        result = Poke(addr,(unsigned char)(data>>24));
        if(!result)
          return result;
        result = Poke(addr+1,(unsigned char)(data>>16));
        if(!result)
          return result;
        result = Poke(addr+2,(unsigned char)(data>>8));
        if(!result)
          return result;
        return Poke(addr+3,(unsigned char)(data));
      }
      break;

    default:
      break;
  }
  return false;
}


