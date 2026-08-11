#define _POSIX_C_SOURCE 200809L
#include "netutil.h"
#include "protocol.h"
#include "cid.h"
#include "log.h"
#include "handlers.h"
#include "config.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <time.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX_EVENTS  256
#define TIMEOUT_SEC 10


// #define FD_SETSIZE 128
// ---------------------------------
static int ep= -1;
static int cmd_fd = -1, rpl_fd = -1, lfd = -1;
static int backend_ready  =0; 
extern int g_level ;
static volatile sig_atomic_t stop_flag = 0;

char *cmd_host;
char *rpl_host;
int cmd_port = -1, rpl_port = -1 , cli_port = -1;


static int dispatch_event ( AgentCtx *ctx, int fd, uint32_t events);
static void on_sig(int s){ (void)s; stop_flag = 1; }

static uint32_t rand_u32(void){
    uint32_t a = (uint32_t)rand(); 
	uint32_t b = (uint32_t)rand();
    return (a << 16) ^ b;
}

static int getenv_int(const char *name, int def){
    const char *v = getenv(name);
    return v ? atoi(v) : def;
}

static const char *getenv_str(const char *name, const char *def){
    const char *v = getenv(name);
    return v ? v : def;
}

extern Config g_cfg;
char   *pname;
// ---------------------------------
// main 
// ---------------------------------
int main(int argc, char **argv)
{
    int  rc = 0;
    AgentCtx ctx;

	pname = basename(argv[0]);

    // config
    if (load_config("/fsfxwin/fep/agt/conf/config.ini") != 0) {
        fprintf(stderr, "Config load failed!\n");
        return 1;
    }
    /* --- 환경 변수 읽기 --- */
    cmd_host = "127.0.0.1";
    rpl_host = "127.0.0.1";
#if 0
    int cli_port   = getenv_int("AGENT_CLI_PORT",   4000);
    cmd_port       = getenv_int("AGENT_CMD_PORT",   32011);
    rpl_port       = getenv_int("AGENT_RPL_PORT",   32001);
#endif
    cli_port   = g_cfg.oms_cli_port;
    cmd_port   = g_cfg.oms_cmd_port;
    rpl_port   = g_cfg.oms_rpl_port;

     // 로그 초기화
    //log_init_default();
    log_set_level(g_cfg.oms_log_level);
	if( log_open_daily(g_cfg.oms_log_file, pname) != 0)
    {
        LOG_WARN("failed to open log file, keep stderr only");
    }
	LOG_INFO ( "START");
 
    signal(SIGINT,  on_sig);
    signal(SIGTERM, on_sig);

    // epoll 생성 
    ctx.epfd  = epoll_create1(0);
	if( ctx.epfd < 0)
	{
	   LOG_ERR("[agent] epoll_create  err!!!");
	   exit (1);
    }
    // server
    ctx.lfd = mk_server(cli_port, 128);
	if( ctx.lfd < 0)
	{
	   LOG_ERR("[agent] mk_server  err!!!");
	   exit (1);
	}
	register_fd( &ctx, ctx.lfd, EPOLLIN );

    // cid 초기화
	cid_table_init();
#if 0 
  // mq 연결
  //  rc = msg_queue_init( g_cfg.oms_msg_key);
  //  if( rc  <  0)
  //  {
  //      LOG_ERR("msg_queue_init fail.rc= [%d]",  rc );
  //      return -1;
  //  }
  //  ctx.msg_id  = rc;
#endif	
    // backend 연결
    if ( handlers_init(&ctx) < 0)
	{
	   LOG_ERR("[agent] handler_init err!!!");
	   exit (1);
    }

    struct epoll_event evs[MAX_EVENTS];

    // main 
    while (!stop_flag)
	{
        int n = epoll_wait(ctx.epfd, evs, MAX_EVENTS, 10000 ); // 1000 ms
        if (n < 0)
		{ 
		    if (errno == EINTR) 
			{
			    continue; 
            }
			break; 
		}
		if( n == 0)
		{
	        // LOG_INFO ( "waiting timeout.... ");
		    continue;
		}	

	    int i;	
        for (i=0;i<n;i++)
		{
		    int fd = evs[i].data.fd;
		    uint32_t ev = evs[i].events;

			if(ev &(EPOLLERR | EPOLLHUP ) )
			{
	            LOG_INFO ( "epoll signal STOP");
			    disarm_and_close( &ctx, fd );
				return 0;
				// continue;
            }
		    dispatch_event( &ctx, fd, ev );
		}
        
		handle_cid_timeouts( &ctx); 
     } 

	 LOG_INFO ( "signal STOP");
	 close(ctx.lfd);
	 close(ctx.epfd);

	 return 0;
}

static int dispatch_event ( AgentCtx *ctx, int fd, uint32_t events)
{
    int rc =0;
    if( events & (EPOLLERR | EPOLLHUP) )
    {
         disarm_and_close( ctx, fd);
         return ;
    }

    if( fd == ctx->lfd   )
    {
        rc = handle_listen_fd( ctx,fd);
    }
    else if( fd == ctx->cmd_fd)
    {
        rc = handle_cmd_fd   ( ctx,fd);
    }
    else if( fd == ctx->rpl_fd)
    {
        rc = handle_rpl_fd   ( ctx,fd);
    }
    else if( fd == ctx->tfd)
    {
        rc = handle_timer_fd ( ctx,fd);
    }
    else
    {
         if( ctx->recv_yn !='N')
         {
             rc = handle_client_fd( ctx,fd);
         }
         else
         {
             rc = handle_client_bat_fd( ctx,fd);
         }
    }
    if( rc == -1) return -1;

    return 0;

}

