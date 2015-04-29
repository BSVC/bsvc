//
// Fake device, connecting a TCP socket in sim68000,
// in order to use (remote) gdb. The goal is to provide
// PutChar and GetChar functions, as required by GDB stub.
// (see m68k_stub.c in gdb sources).
//

#ifndef M68K_DEVICES_GDBSOCK_HPP_
#define M68K_DEVICES_GDBSOCK_HPP_

#include "Framework/BasicDevice.hpp"

class GdbSocket : public BasicDevice {
public:
  GdbSocket(const std::string &args, BasicCPU &cpu);
  ~GdbSocket();

  // Returns true iff the address maps into the device.
  virtual bool CheckMapped(Address addr) const;

  // Returns the lowest address used by the device.
  virtual Address LowestAddress() const { return (m_base_address); }

  // Returns the highest address used by the device.
  virtual Address HighestAddress() const {
    return (m_base_address + 3); // need 4 bytes: status,command,read,write
  }

  // Gets a byte from the device.
  virtual Byte Peek(Address addr);

  // Puts a byte into the device.
  virtual void Poke(Address addr, Byte c);

  // Resets the device.
  virtual void Reset();

  // Handles socket events.
  void EventCallback(int type, void *pointer);

private:
  Address m_base_address;       // Base address of the device
  int m_port;                   // socket port number
  int m_server_socket;          // server socket identifier
  int m_client_socket;          // client socket identifier
  int m_status;                 // 1 if connected, 0 otherwise

  Byte *m_received_buffer;
  int m_received_first, m_received_last, m_received_length;

  Byte *m_send_buffer;
  int m_send_length;

  void SetupServer();
};

#endif  // M68K_DEVICES_GDBSOCK_HPP_
