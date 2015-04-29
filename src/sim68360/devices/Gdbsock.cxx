///////////////////////////////////////////////////////////////////////////////
//
// Gdbsock.cxx
//
// Fake device, connecting a TCP socket in Sim68360,
// in order to use (remote) gdb.
// (c) DJ, July 1996
//
// Adapted from the M68681 device.
//
// Description: this device use 4 bytes (from 'base_address'):
// 0 is status byte (read only)
// 1 is command byte (write only)
// 2 is receive byte (read only)
// 3 is send byte (write only)
//
// status is:
// 0: not running,
// bit0=1: server started
// bit1=1: client connected (requires server started!)
// bit2=1: ready to send
// bit3=1: there are pending characters
//
// command is: currently, there is no need of commands.
//
// * USUAL DISCLAIMER... NO WARRANTY FOR ANY PURPOSE *
//
///////////////////////////////////////////////////////////////////////////////
// $Id: $
///////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
#include <iostream.h>
#include <strstrea.h>
#else
#include <iostream.h>
#include <strstream.h>
#endif

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "Tools.hxx"
#include "BasicCPU.hxx"
#include "Gdbsock.hxx"

// The duration of the default select events
#define DEFAULT_CALLBACK_DURATION  100000

// Size of send buffer and receive buffer
#define BUFFER_LENGTH 1024

///////////////////////////////////////////////////////////////////////////////
// The Constructor
///////////////////////////////////////////////////////////////////////////////
GdbSocket::GdbSocket(const string& args, BasicCPU& cpu)
  : BasicDevice("GdbSocket", args, cpu)
{
	istrstream in((char*)args.data(), args.length());
	string keyword, equals;
	unsigned long base;
	
	m_port=0;
	m_server_socket=-1;
	m_client_socket=-1;
	m_status=0;
	
	m_send_buffer=new unsigned char[BUFFER_LENGTH];
	m_received_buffer=new unsigned char[BUFFER_LENGTH];

	// Scan "BaseAddress = nnnn" (in hexadecimal)
	in >> keyword >> equals >> hex >> base;
	if((!in) || (keyword != "BaseAddress") || (equals != "="))
	{
		ErrorMessage("Invalid initialization arguments!");
		return;
	}

	// Scan "PortNumber = nnnn" (in decimal)
	in >> keyword >> equals >> dec >> m_port;
	if((!in) || (keyword != "PortNumber") || (equals != "="))
	{
		ErrorMessage("Invalid initialization arguments!");
		return;
	}
	
	if(m_port==0)
	{
		ErrorMessage("Bad socket port number!");
		return;
	}
	m_base_address = base * cpu.Granularity();
	
	// Reset the socket to its startup state
	Reset();
	
	// Schedule an event to start reading the socket
	CPU().eventHandler().Add(this, 0,(void*)0, DEFAULT_CALLBACK_DURATION);
}


///////////////////////////////////////////////////////////////////////////////
// The Destructor
///////////////////////////////////////////////////////////////////////////////
GdbSocket::~GdbSocket()
{
	delete[] m_received_buffer;
	delete[] m_send_buffer;

	// stop the client
	if(m_client_socket!=-1)
	{
		close(m_client_socket);
	}
	// stop the server
	if(m_server_socket!=-1)
	{
		shutdown(m_server_socket, 2);
		close(m_server_socket);
	}	
}

///////////////////////////////////////////////////////////////////////////////
// Start the socket server
///////////////////////////////////////////////////////////////////////////////
int GdbSocket::SetupServer()
{
	struct sockaddr_in addr;
	
	m_server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(m_server_socket<0)
	{
		return m_server_socket;
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr=INADDR_ANY;
	addr.sin_port=htons(m_port);
	
	if(bind(m_server_socket, (struct sockaddr*)&addr, sizeof(addr))<0)
	{
		close(m_server_socket);
		m_server_socket=-1;
		return 1;
	}
	
	// listen for the connection
	if(listen(m_server_socket,1)<0)
	{		
		close(m_server_socket);
		m_server_socket=-1;
		return 2;
	}
	
	// the 'accept' and other operations will be performed 
	// through select calls, periodically (see EventCallback).	
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Perform a reset of the socket.
///////////////////////////////////////////////////////////////////////////////
void GdbSocket::Reset()
{
	int code;
	
	BasicDevice::Reset();              // Do the BasicDevice reset

	// reset buffers
	m_send_length=0;
	m_received_length=0;
	m_received_last=0;
	m_received_first=0;

	// stop the client
	if(m_client_socket!=-1)
	{
		code=close(m_client_socket);
		m_client_socket=-1;
		m_status=0;
	}	
	// start the server if necessary
	if(m_server_socket==-1)
	{
		(void)SetupServer();
	}	
}

///////////////////////////////////////////////////////////////////////////////
// Check to see if the address maps into this device
///////////////////////////////////////////////////////////////////////////////
bool GdbSocket::CheckMapped(unsigned long addr) const
{
	if((addr>=LowestAddress()) && (addr<=HighestAddress()))
	{
		return(1);
	}
	else
	{
		return(0);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Read one of the register of the device
///////////////////////////////////////////////////////////////////////////////
unsigned char GdbSocket::Peek(unsigned long addr)
{
	unsigned char c=0;
	
	addr -= m_base_address;

	if(addr==0)
	{
		// read status byte
		if(m_server_socket==-1)
		{
			// server not started.
			return c;
		}
		c|=1;
		if(m_client_socket==-1)
		{
			// no connected client.
			return c;
		}
		c|=2;
		if(m_send_length<BUFFER_LENGTH)
		{
			// can send.
			c|=4;
		}
		if(m_received_length>0)
		{
			// there are pending characters.
			c|=8;
		}	  		
	}
	else if(addr==2)
	{
		// read received byte
		if(m_received_length>0)
		{
			c = (unsigned char)m_received_buffer[m_received_first];
			
			m_received_length--;
			m_received_first++;
			if(m_received_first>=BUFFER_LENGTH)
			{
				m_received_first=0;
			}
			return c;
		}
	}

	return c;
}

///////////////////////////////////////////////////////////////////////////////
// Write to one of the registers of the device
///////////////////////////////////////////////////////////////////////////////
void GdbSocket::Poke(unsigned long addr, unsigned char c)
{
	addr -= m_base_address;

	if(addr==1)
	{
		// command byte. Not needed currently.
	}	
	else if(addr==3)
	{
		// send byte
		if(m_send_length<BUFFER_LENGTH)
		{
			m_send_buffer[m_send_length++]=c;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// Handle event callbacks
///////////////////////////////////////////////////////////////////////////////
void GdbSocket::EventCallback(long type, void* pointer)
{
	if(m_status)
	{
		// try to write or read from the socket
		fd_set r,w,e;
		struct timeval delay;
		int result;
		int busy;
		
		do
		{
			busy=0;
			FD_ZERO(&r);
			FD_ZERO(&w);
			FD_ZERO(&e);
			delay.tv_sec=0;
			delay.tv_usec=0;
			FD_SET(m_client_socket, &r);			
			FD_SET(m_client_socket, &w);
		
			result=select(m_client_socket+1, &r,&w,&e,&delay);
			if((result>=1) && (m_received_length<BUFFER_LENGTH) && (FD_ISSET(m_client_socket,&r)))
			{
				char c;
				result=recv(m_client_socket, &c, 1, 0);
				if(result>0)
				{
					if(m_received_last>=BUFFER_LENGTH)
					{
						m_received_last=0;
					}
					m_received_buffer[m_received_last]=(unsigned char)c;
					m_received_last++;
					m_received_length++;
					busy++;
				}				
			}
			if((result>=1) && (m_send_length>0) && (FD_ISSET(m_client_socket,&w)))
			{
				result=send(m_client_socket,(char*)m_send_buffer,m_send_length,0);
				if(result != m_send_length)
				{
					break;
				}
				m_send_length=0;
				busy++;
			}
		}
		while(busy>=1);		
	}
	else
	{
		// try to connect the socket
		fd_set r,w,e;
		struct timeval delay;
		int result;
		
		FD_ZERO(&r);
		FD_ZERO(&w);
		FD_ZERO(&e);
		delay.tv_sec=0;
		delay.tv_usec=0;
		FD_SET(m_server_socket, &r);
		
		result=select(m_server_socket+1, &r,&w,&e,&delay);
		if((result==1) && (FD_ISSET(m_server_socket,&r)))
		{
			m_client_socket=accept(m_server_socket,NULL,NULL);
			if(m_client_socket!=-1)
			{
				m_status=1;
			}
		}
	}	
	
	// Reschedule another event callback to check for more characters
	CPU().eventHandler().Add(this, 0, (void*)0, DEFAULT_CALLBACK_DURATION);
	
	// unused: this remove annoying warnings.
	(void)type;
	(void)pointer;
}

//
// End of file
//

