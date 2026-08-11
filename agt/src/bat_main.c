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
#include <libgen.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/timerfd.h>

#define MAX_EVENTS  256
#define TIMEOUT_SEC 10

#define  IO_TIMEOUT_MS  10000 // 응답이 안오면 10초후 timeout을 내 보낸다.
#define  CID_DEADLINE_SEC  10
#define  LIVE_IDLE_SEC     30
#define  LIVE_PERIOD_SEC   5


typedef struct
{
    int epfd;               // epoll fd
    int lfd;                // listen fd
    int cmd_fd;
    int rfd;
    int tfd;
    int cfd;
    int msg_id;

    time_t last_client_ts;
    time_t last_live_ts;
    int  data_seq;
} Context;

// #define FD_SETSIZE 128
// ---------------------------------
static int ep= -1;
// static int cmd_fd = -1, rpl_fd = -1, lfd = -1;
static int backend_ready  =0; 
extern int g_level ;
static volatile sig_atomic_t stop_flag = 0;

char *cmd_host;
char *rpl_host;
int cmd_port = -1, rpl_port = -1 , cli_port = -1;


static void on_sig(int s){ (void)s; stop_flag = 1; }
static int bat_register_fd    ( Context *ctx, int fd, uint32_t events);
static int bat_dispatch_event  ( Context *ctx, int fd, uint32_t events);
static int bat_oms_connect     ( Context *ctx);
static int bat_send_link       ( Context *ctx);
static int bat_handle_timer_fd ( Context *ctx);
static int bat_handle_rpl_fd   ( Context *ctx);
static int bat_handle_client_fd( Context *ctx);
static int bat_handle_listen_fd( Context *ctx);
static void bat_close_fd( Context *ctx, int fd);

extern Config g_cfg;
char *pname;
// ---------------------------------
// main 
// ---------------------------------
int main(int argc, char **argv)
{
    int  rc = 0;
    Context ctx;

    pname = basename(argv[0]);
    // config
    if (load_config("/fsfxwin/fep/agt/conf/config.ini") != 0) {
        fprintf(stderr, "Config load failed!\n");
        return 1;
    }
    /* --- 환경 변수 읽기 --- */
    cmd_host = "127.0.0.1";
    rpl_host = "127.0.0.1";
    cli_port   = g_cfg.bat_cli_port;
    cmd_port   = g_cfg.bat_cmd_port;
    rpl_port   = g_cfg.bat_rpl_port;

    // 로그 초기화
    //log_init_default();
    log_set_level(g_cfg.bat_log_level);
	if( log_open_daily(g_cfg.bat_log_file, pname) != 0)
    {
        LOG_WARN("failed to open log file, keep stderr only");
    }
 
    LOG_INFO("START");
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
	bat_register_fd( &ctx, ctx.lfd, EPOLLIN );

    // cid 초기화
	// cid_table_init();

    // mq 연결
#if 0	
    rc = msg_queue_init( g_cfg.bat_msg_key);
    if( rc != 0)
    {
          LOG_INFO("[%s] msg_queue_init fail.", ctx->msg_key );
          return -1;
    }
    ctx.msg_id  = rc;
#endif

    // backend 연결
    if ( bat_oms_connect(&ctx) < 0)
	{
	   LOG_ERR("handler_init err!!!");
	   exit (1);
    }

	LOG_INFO ( "event....start ");
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
		    int      fd = evs[i].data.fd;
		    uint32_t ev = evs[i].events;

			if(ev &(EPOLLERR | EPOLLHUP ) )
			{
                LOG_INFO("epoll siganl STOP");
				bat_close_fd( &ctx,fd);
				break;
            }
		    rc = bat_dispatch_event( &ctx, fd, ev );
			if( rc != 0)
			{
                LOG_ERR(" dispatch_event error...종료..");
				return -1;
			}

		}
     } 
	 close(ctx.lfd );
	 close(ctx.epfd);
     LOG_INFO("signal STOP");

	 return 0;
}

static int bat_dispatch_event ( Context *ctx, int fd, uint32_t events)
{
    int rc =0;
    if( events & (EPOLLERR | EPOLLHUP) )
    {
         return ;
    }

    if( fd == ctx->lfd   )
    {
        rc = bat_handle_listen_fd( ctx);
    }
    else if( fd == ctx->cmd_fd)   //  사용되지 않음
    {
        ;
        // rc = handle_cmd_fd   ( ctx,fd);
    }
    else if( fd == ctx->rfd) //응답을 받으면  client에 전달 필요
    {
        rc = bat_handle_rpl_fd   ( ctx);
    }
    else if( fd == ctx->tfd)
    {
        
        rc = bat_handle_timer_fd ( ctx);
    }
    else  // client 요청
    {
        rc = bat_handle_client_fd( ctx);
    }
    if( rc == -1) return -1;

    return 0;
}

static int bat_handle_client_fd( Context *ctx)
{
    int     rc = 0;
    char    recv[2048];
    char    send[2048];
    int     len  = 0;
    char    cid[11];
    char    lenbuf[5] = {0};
    int     recvlen = 0;

    LOG_INFO("[client]bat 수신 ..." );
    rc = readn_timed( ctx->cfd, lenbuf, 4, IO_TIMEOUT_MS);
    if( rc < 0)
    {
        if( rc == -2)
        {
            LOG_INFO("client connect out..");
			bat_close_fd( ctx, ctx->cfd);
            return 0;
        }
        else
        {
            LOG_ERR("client 수신 read1 error !! rc = %d", rc);
			bat_close_fd( ctx, ctx->cfd);
			return -1;
        }
        return 0;
    }
    recvlen = atoi(lenbuf);

    rc = readn_timed( ctx->cfd, recv, recvlen, IO_TIMEOUT_MS);
    if( rc < 0)
    {
        LOG_ERR("client 수신 read2 error !! rc = %d", rc);
		bat_close_fd( ctx, ctx->cfd);
        return  -1;
    }
	LOG_INFO("CLIENT RD;[%s]",recv);

    memcpy( send,   lenbuf , 4);
    memcpy( send+4, recv   , recvlen);
    len = recvlen + 4;
    /* 송신 */
    rc = sendn_timed( ctx->cmd_fd, send, len, IO_TIMEOUT_MS );
    if( rc == -ETIMEDOUT  || rc < 0)
    {
        LOG_ERR("bat 송신 wrtte error !! rc = %d", rc);
		bat_close_fd( ctx, ctx->cmd_fd);
        return  -1;
    }

    /* 수신 */
    rc = readn_timed( ctx->cmd_fd, recv, 50,  IO_TIMEOUT_MS);
    if( rc < 0)
    {
        LOG_ERR("bat 수신 read  error !! rc = %d", rc);
		bat_close_fd( ctx, ctx->cmd_fd);
        return -1;
    }
	LOG_INFO("RD;[%.50s]",recv);
    ///////////////////////////////////////////////////////

    if(memcmp( &recv[4],"DAOK", 4)==0 )
    {
        LOG_INFO("DAOK");
    }
    /* client  송신 */
    rc = sendn_timed( ctx->cfd, recv, 50, IO_TIMEOUT_MS );
    if( rc == -ETIMEDOUT  || rc < 0)
    {
        LOG_INFO("[client] timeout..");
		bat_close_fd( ctx, ctx->cfd);
    }
    LOG_INFO("[client]접수 완료.. 응답대기... ");

    return 0;
}


/* oms 응답 data -> client 전송 */
static int  bat_handle_rpl_fd( Context *ctx )
{
    char    recvbuf[2048] = {0};
    char    sendbuf[2048] = {0};
    char    lenbuf[5] = {0};
    int     blen;

    int itoday = get_today();
    char today[9];
    sprintf( today,"%8d",itoday); today[8] = 0x00;

LOG_INFO(" rpl event....");
    int rc = readn_timed(ctx->rfd, lenbuf, 4, IO_TIMEOUT_MS);
    if( rc < 0)
    {
        LOG_ERR("read  oms1 error. rc = %d",rc);
		bat_close_fd( ctx, ctx->rfd);
        return -1;
    }
    blen = atoi(lenbuf);

    rc = readn_timed(ctx->rfd, recvbuf, blen, IO_TIMEOUT_MS);
    if( rc < 0)
    {
        LOG_ERR("read  oms2 error. rc = %d",rc);
		bat_close_fd( ctx, ctx->rfd);
        return -1;
    }
    LOG_INFO("RD:[%s%s]",lenbuf,recvbuf);
 

    if(  !strncmp( &recvbuf[0], "POLL",4) )
    {
        /* pool 응답 */
        memset( sendbuf, 0x20, 50);
        memcpy( &sendbuf[0], "0046",4);
        memcpy( &sendbuf[4], "POOK",4);
        memcpy( &sendbuf[8], "0000",4);
        memcpy( &sendbuf[12], today ,8);
        memcpy( &sendbuf[20], "00000000" ,8);
        rc = sendn_timed( ctx->rfd, sendbuf, 50, IO_TIMEOUT_MS );
        if( rc == -ETIMEDOUT  || rc < 0)
        {
            LOG_ERR("send  oms1 error. rc = %d",rc);
		    bat_close_fd( ctx, ctx->rfd);
            return -1;
        }
        LOG_INFO("WD:[%s]",sendbuf);
        return 0;
        
    }
    else if( !strncmp( &recvbuf[0],"LIOK", 4) )
    {
        LOG_INFO("RPL LIOK OK");
        return;
    }
    else  // DATA
    {
        /* pool 응답 */
        LOG_INFO("RPL 응답...");
        memset( sendbuf, 0x20, 50);

        //memcpy( &sendbuf[0], lenbuf, 4);
        memcpy( &sendbuf[0], "0046", 4);
        memcpy( &sendbuf[4], "DAOK",4);
        memcpy( &sendbuf[8], "0000",4);
        memcpy( &sendbuf[12], today ,8);
        rc = sendn_timed( ctx->rfd, sendbuf,50, IO_TIMEOUT_MS );
        if( rc == -ETIMEDOUT  || rc < 0)
        {
            return -1;
        }

        // client에 응답 전송
		memcpy( sendbuf,   lenbuf, 4);
		memcpy( sendbuf+4, recvbuf, blen);
LOG_INFO("Client 응답  WR[%.*s]", blen+4, sendbuf );
        //rc = sendn_timed(ctx->cfd, &recvbuf[46], len, IO_TIMEOUT_MS );
        rc = sendn_timed(ctx->cfd, sendbuf, blen+4, IO_TIMEOUT_MS );
        if( rc <0)
        {
             LOG_INFO("bat client 응답 오류 ... rc=%d", rc );
		     bat_close_fd( ctx, ctx->cfd);
             return -1;
        }
        LOG_INFO("bat client 응답 전송 완료>>> ");
    }
    return 0;
}


static int bat_register_fd( Context *ctx, int fd, uint32_t events)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    ev.events  = events;
    ev.data.fd = fd;

    int rc = epoll_ctl(ctx->epfd, EPOLL_CTL_ADD,fd, &ev);
    if( rc < 0)
    {
         disarm_and_close( ctx, fd);
		 return -1;
    }
    LOG_INFO("[event] register event fd [%d]..", fd );
	return 0;
}


/* --- 초기화 --- */
static int bat_oms_connect ( Context *ctx )
{
    int rc =0;
    ctx->cmd_fd = tcp_connect( cmd_host, cmd_port);
    if( ctx->cmd_fd > 0)
    {
        ;// register_fd(ctx,ctx->cmd_fd, EPOLLIN );
    }
    if( ctx->cmd_fd < 0)
    {
       LOG_ERR("oms connection fail... comd_fd= %d", ctx->cmd_fd );
       return -1;
    }

    ctx->rfd = tcp_connect( rpl_host,rpl_port );
    if( ctx->rfd > 0)
    {
        LOG_INFO("rpl_fd event registry ." );
        bat_register_fd(ctx,ctx->rfd, EPOLLIN );
    }

    if( ctx->rfd < 0)
    {
       LOG_ERR("oms connection fail... rpl_fd = %d", ctx->rfd);
       return -1;
    }

    LOG_INFO("oms connection ok." );

    /* timer 등록 */
    ctx->last_client_ts = time(NULL);
    ctx->last_live_ts   =  0;
    ctx->data_seq = 0;

    // pend_init( &ctx->pending);
    ctx->tfd = timerfd_create( CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC );
    if( ctx->tfd >= 0)
    {
        struct itimerspec it = {0};
        it.it_value.tv_sec = LIVE_PERIOD_SEC;
        it.it_interval.tv_sec = LIVE_PERIOD_SEC;
        timerfd_settime ( ctx->tfd, 0 ,&it, NULL);

        bat_register_fd(ctx, ctx->tfd, EPOLLIN);
    }
    else
    {
        LOG_ERR(" timer create error !!");
        return -2;
    }
    //LOG_INFO("[POLL] timer create ok..!!");

    /* 최초 link */
    rc = bat_send_link( ctx);
    if( rc != 0)
    {
          LOG_INFO("oms logon fail." );
          return -1;
    }
    LOG_INFO("[LOGON]ok." );
    return 0;
}



/* ---  초기 LINK 송신 -- */
static int bat_send_link  (Context *ctx)
{
    int rc = 0;
    CommHdr ch = {0};
    DataHdr dh = {0};
    int   blen =0;
    char *body= NULL;
    int itoday = get_today();
    char today[9];
    sprintf( today,"%8d",itoday); today[8] = 0x00;

    memset(&ch,'0',sizeof(ch));
    memcpy(ch.Length,  "0046"    ,4);
    memcpy(ch.MsgType, "LINK"    ,4);
    memcpy(ch.TradeDate, today   ,8);
    memcpy(ch.SeqNo  , "00000000",8);

    rc = send_packet_fd_timed( ctx->cmd_fd, &ch, &dh, body ,0, IO_TIMEOUT_MS );
    if( rc < 0)
    {
          LOG_ERR("cmd send_link error!!");
          return -1;
    }
    LOG_INFO(" cmd send_link ok!! [%s]", &ch);

    rc = recv_packet_fd_timed( ctx->cmd_fd,&ch,&dh, body, &blen, sizeof(body), IO_TIMEOUT_MS);
    if( rc < 0)
    {
       LOG_ERR("cmd_fd recv error reconnecting..rc(%d)", rc);
       return  -1;
    }

     if( strncmp( ch.MsgType, "LIOK",4) )
     {
         return -1;
     }
     LOG_INFO("CMD LIOK");

    memcpy(ch.Length,  "0046"    ,4);
    memcpy(ch.MsgType, "LINK"    ,4);
    memcpy(ch.SeqNo  , "00000000",8);
    rc = send_packet_fd_timed( ctx->rfd, &ch, &dh, body ,0, IO_TIMEOUT_MS );
    if( rc < 0)
    {
          LOG_ERR("cmd send_link error!!");
          return -1;
    }
    //LOG_INFO(" rpl send_link ok!!");
	return 0;
}

/* --- timer event ---*/
static int bat_handle_timer_fd( Context *ctx)
{
    uint64_t ticks;
    CommHdr ch;
    DataHdr dh;
    char    body[65536];
    int     blen;

    if( read(ctx->tfd , &ticks, sizeof(ticks)) < 0) return;

    time_t now = time(NULL);
    if( (now - ctx->last_client_ts ) < LIVE_IDLE_SEC )
    {
       return  0;
    }

    if( ctx->cmd_fd < 0)
    {
        reconnect_backend( ctx );
        return -1;
    }

     int rc = 0;
     char sndbuf[2048];
     char today[9];
     int snd_len = 0;
     int pool_flag = 0;
     int itoday;

     itoday = get_today();
     sprintf(today, "%8d",itoday); today[8] = 0x00;

     memset( sndbuf, 0x00,sizeof(sndbuf));
     sprintf( sndbuf,"0046POLL0000%.8s000000000000000000000000000000", today);
     snd_len = strlen(sndbuf);

     if( (now - ctx->last_client_ts) >= LIVE_IDLE_SEC)
     {
         if( (now-ctx->last_live_ts ) > LIVE_IDLE_SEC )
         {
            rc = sendn_timed( ctx->cmd_fd, sndbuf, snd_len, IO_TIMEOUT_MS );
            if( rc < 0)
            {
               LOG_INFO( "CMD live 전송오류..reconnect ...");
               return -1;
            }

            LOG_INFO("CMD POLL send ");
            rc = recv_packet_fd_timed( ctx->cmd_fd,&ch,&dh, body, &blen, sizeof(body), IO_TIMEOUT_MS);
            if( rc < 0)
            {
                LOG_ERR("CMD_POOL recv  error reconnecting..rc(%d)", rc);
                return -1;
            }

            if( !strncmp( ch.MsgType, "POOK",4) )
            {
                ctx->last_client_ts = now;
                LOG_INFO("CMD POOK recv");
            }

         }
     }

     return 0;
}

static int bat_handle_listen_fd( Context *ctx)
{
    ctx->cfd = accept( ctx->lfd, NULL,NULL);
    if( ctx->cfd < 0 ) return -1;

    bat_register_fd( ctx, ctx->cfd, EPOLLIN );
    LOG_INFO("[agent] client connected fd=%d", ctx->cfd);
    return 0;
}

static void bat_close_fd( Context *ctx, int fd)
{
    if( fd < 0) return;

    epoll_ctl( ctx->epfd ,EPOLL_CTL_DEL,fd, NULL);
    close(fd);

    if( fd == ctx->cmd_fd ) ctx->cmd_fd = -1;
    if( fd == ctx->rfd )    ctx->rfd = -1;
    if( fd == ctx->lfd )    ctx->lfd = -1;
}
