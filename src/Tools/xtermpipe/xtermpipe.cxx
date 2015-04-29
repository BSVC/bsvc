///////////////////////////////////////////////////////////////////////////////
// This simple program can be used with the Motorola M68681 DUART
// to pipe input and output to an xterm.
//
// By: Bradford W. Mott
///////////////////////////////////////////////////////////////////////////////

#include <sys/time.h>
#include <sys/types.h>
#include <sys/param.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

int WaitForIO(int pipe_id)
{
  fd_set readfds;
  fd_set writefds;
  fd_set exceptfds;

  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  FD_ZERO(&exceptfds);

  FD_SET(0, &readfds);
  FD_SET(pipe_id, &readfds);

# ifdef _HPUX_SOURCE
    select(pipe_id + 1, (int*)&readfds, (int*)&writefds, (int*)&exceptfds,
           (void*)0);
# else
    select(pipe_id + 1, &readfds, &writefds, &exceptfds, (void*)0);
# endif

  if(FD_ISSET(0, &readfds))
    return(0);
  else
    return(1);
}

main()
{
  int read_id, write_id;

  system("stty -echo -echoe -echonl raw");

  read_id=3;
  write_id=4;

  while(1)
  {
    if(WaitForIO(read_id))
    {
      char c;

      read(read_id, &c, 1);
      write(1,&c,1);
    }
    else
    {
      char c;

      read(0, &c, 1);
      if(write(write_id, &c, 1)<0)
        printf("Error on write!!!\n");
    }
  }
}
