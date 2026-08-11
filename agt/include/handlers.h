#ifndef HANDLERS_H
#define HANDLERS_H

#include <sys/types.h>
#include <sys/ipc.h>
#include "protocol.h"
typedef void (*reconnect_fn)(void);

typedef struct 
{
    int epfd;               // epoll fd
	int lfd;                // listen fd 
	int cmd_fd;           
	int rpl_fd;
    int tfd;
	int msg_id;

	time_t last_client_ts;
	time_t last_live_ts;
	int  data_seq;
	char recv_yn;
} AgentCtx;

int  handle_int( AgentCtx *ctx);

void disarm_and_close( AgentCtx *ctx, int fd);
int handle_listen_fd( AgentCtx *ctx, int lfd);
int handle_client_fd( AgentCtx *ctx, int cfd);
int handle_cmd_fd   ( AgentCtx *ctx, int fd);
int handle_rpl_fd   ( AgentCtx *ctx, int fd);
int handle_cid_timeouts( AgentCtx *ctx) ;
//void send_timeout_response_to_client( int client_fd, char *cid);
int  mq_setup_sender(void);

int handle_timer_fd(AgentCtx *ctx, int tfd);

#endif /* HANDERS_H */

