/*------------------------------------------------------------------------
#   Module  : Shared utility functions
#   File    : poll_event.c
------------------------------------------------------------------------*/

#include    "fep_fepp.h"
#include    <poll.h>

/*------------------------------------------------------------------------
    Function    : detect_poll_event
    Description : Check poll array for POLLHUP and POLLIN events
    Parameters  : poll_arr         - poll file descriptor array
                  poll_cnt         - number of entries in poll_arr
                  socket_event_idx - index of socket fd in poll_arr
    Return      : >= 0: index of fd with POLLIN event
                  -1: socket disconnected (POLLHUP on socket_event_idx)
                  -2: no POLLIN event found
------------------------------------------------------------------------*/
int detect_poll_event(struct pollfd *poll_arr, int poll_cnt, int socket_event_idx) {
    int     i, first_pollin;

    first_pollin = -2;

    for (i = 0; i < poll_cnt; i++) {
        if (poll_arr[i].revents & POLLHUP) {
            if (i == socket_event_idx) {
                Log(TCP_ERROR, "socket disconnected[%#06x]",
                        poll_arr[i].revents);
                return -1;
            }

            Log(SYS_ERROR, "poll hangup[%d,%d]", i, poll_cnt);
        }

        if ((poll_arr[i].revents & POLLIN) && first_pollin == -2) {
            poll_arr[i].revents = 0;
            first_pollin = i;
        }
    }

    return first_pollin;
}
