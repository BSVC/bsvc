///////////////////////////////////////////////////////////////////////////////
//
// Gdbsock.hxx
// Fake device, connecting a TCP socket in sim68000,
// in order to use (remote) gdb. The goal is to provide
// PutChar and GetChar functions, as required by GDB stub.
// (see m68k_stub.c in gdb sources).
//
// (c) DJ, July 1996
//
// Adapted from the M68681 device.
//
// * USUAL DISCLAIMER... NO WARRANTY FOR ANY PURPOSE *
//
///////////////////////////////////////////////////////////////////////////////
// $Id: $
///////////////////////////////////////////////////////////////////////////////

#ifndef GDBSOCK_HXX
#define GDBSOCK_HXX

#include "BasicDevice.hxx"

class GdbSocket : public BasicDevice {
  public:
    GdbSocket(const string& args, BasicCPU& cpu);

    ~GdbSocket();

    // See if the address maps into the device (1=Yes,0=No)
    virtual bool CheckMapped(unsigned long addr) const;

    // Return the lowest address used by the device
    virtual unsigned long LowestAddress() const
    { return(m_base_address); }

    // Return the highest address used by the device
    virtual unsigned long HighestAddress() const
    { 
		return(m_base_address+3); // need 4 bytes: status,command,read,write
    }

    // Get a byte from the device
    virtual unsigned char Peek(unsigned long addr);

    // Put a byte into the device
    virtual void Poke(unsigned long addr, unsigned char c);

    // Reset handler
    virtual void Reset();
   
    // Handle the socket events
    void EventCallback(long type, void* pointer);

 private:
	
	unsigned long m_base_address; // Base address of the device
	int m_port; // socket port number
	int m_server_socket; // server socket identifier
	int m_client_socket; // client socket identifier
	int m_status; // 1 if connected, 0 otherwise

	unsigned char *m_received_buffer;
	int m_received_first, m_received_last, m_received_length;
	
	unsigned char *m_send_buffer;
	int m_send_length;

	int SetupServer();			
};

#endif /* GDBSOCK_HXX */


