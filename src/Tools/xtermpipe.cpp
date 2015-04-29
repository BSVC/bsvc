// This simple program can be used with the Motorola M68681 DUART
// to pipe input and output to an xterm.

#include <sys/time.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

int WaitForIO(int pipe_id) {
  fd_set readfds;
  fd_set writefds;
  fd_set exceptfds;

  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  FD_ZERO(&exceptfds);

  FD_SET(0, &readfds);
  FD_SET(pipe_id, &readfds);

  select(pipe_id + 1, &readfds, &writefds, &exceptfds, NULL);

  if (FD_ISSET(0, &readfds))
    return (0);

  return (1);
}

int main() {
  int read_id, write_id;

  if (system("stty -echo -echoe -echonl raw") < 0) {
    fprintf(stderr, "Can't set terminal mode. Failing.\n");
    exit(1);
  }

  read_id = 3;
  write_id = 4;

  for (;;) {
    if (WaitForIO(read_id)) {
      char c;

      if (read(read_id, &c, 1) != 1) {
        fprintf(stderr, "Read failed.\n");
        exit(1);
      }
      if (write(1, &c, 1) != 1) {
        fprintf(stderr, "Write failed.\n");
        exit(1);
      }
    } else {
      char c;

      if (read(0, &c, 1) != 1) {
        fprintf(stderr, "Read failed.\n");
        exit(1);
      }
      if (write(write_id, &c, 1) < 0) {
        fprintf(stderr, "Error on write.\n");
        exit(1);
      }
    }
  }
}
