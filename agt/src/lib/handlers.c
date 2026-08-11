
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <arpa/inet.h>
#include <mqueue.h>

#include "handlers.h"
#include "protocol.h"
#include "cid.h"
#include "netutil.h"
#include "log.h"

#define  IO_TIMEOUT_MS  10000 // 응답이 안오면 10초후 timeout을 내 보낸다.
#define  CID_DEADLINE_SEC  10
#define  LIVE_IDLE_SEC     30
#define  LIVE_PERIOD_SEC   5

#define  MQ_MAXSIZE   2048
#define  MQ_MAXMSG    10


extern char *cmd_host;
extern char *rpl_host;
extern int cmd_port;
extern int rpl_port;


static 	void fill_seq ( char *dst, int seq )
{
    char tmp[16];
	snprintf(tmp, sizeof(tmp), "%08d", seq );
	memcpy( dst, tmp, 8);
}

/* ----- TIMEOUT 회신 -- */
static void reply_timeout_to_client( AgentCtx *ctx, int client_fd,char *cid)
{
    int rc =0;
    CommHdr ch = {0};
    DataHdr dh = {0};
    char *body = NULL;
    int itoday = get_today();
	char today[9];
	sprintf( today,"%8d",itoday); today[8] = 0x00;

    if( client_fd < 0) 
	   return 0;

    memset(&ch,'0',sizeof(ch));
    memcpy(ch.Length,  "0046"    ,4);
    memcpy(ch.MsgType, "TIME"    ,4);
    memcpy(ch.TradeDate, today   ,8);
    memcpy(ch.SeqNo  , "00000000",8);

    rc = send_packet_fd_timed( client_fd, &ch, &dh, body, 0, IO_TIMEOUT_MS);
	if( rc < 0)
    {
	      LOG_ERR("reply timeout  to client error!! rc[%d]", rc);
	      return -1;
	}
	// timeout 보내고 client 닫는다.
    disarm_and_close ( ctx , client_fd);
}

/* ---  초기 LINK 송신 -- */
static int send_link  (AgentCtx *ctx)
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
   
if( ctx->recv_yn !='N')
{
    memcpy(ch.Length,  "0046"    ,4);
    memcpy(ch.MsgType, "LINK"    ,4);
    memcpy(ch.SeqNo  , "00000000",8);
    rc = send_packet_fd_timed( ctx->rpl_fd, &ch, &dh, body ,0, IO_TIMEOUT_MS );
	if( rc < 0)
    {
	      LOG_ERR("cmd send_link error!!");
	      return -1;
	}
	//LOG_INFO(" rpl send_link ok!!");
}	
	

    return 0;
}

/* --- 초기화 --- */
int handlers_init ( AgentCtx *ctx )
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

if( ctx->recv_yn != 'N')
{
	ctx->rpl_fd = tcp_connect( rpl_host,rpl_port );
	if( ctx->rpl_fd > 0)
	{
	    LOG_INFO("rpl_fd event registry ." );
	    register_fd(ctx,ctx->rpl_fd, EPOLLIN );
	}

	if( ctx->rpl_fd < 0)
    {
	   LOG_ERR("oms connection fail... rpl_fd = %d", ctx->rpl_fd);
	   return -1;
	}
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

		register_fd(ctx, ctx->tfd, EPOLLIN);
    }
	else
	{
	    LOG_ERR(" timer create error !!");
	    return -2;
	}
	//LOG_INFO("[POLL] timer create ok..!!");

    /* 최초 link */
    rc = send_link( ctx);
	if( rc != 0)
	{
	      LOG_INFO("oms logon fail." );
		  return -1;
	}
	LOG_INFO("[LOGON]ok." );
	return 0;
}

void disarm_and_close( AgentCtx *ctx, int fd)
{
    if( fd < 0) return;
    
    epoll_ctl( ctx->epfd ,EPOLL_CTL_DEL,fd, NULL);
    close(fd);

    if( fd == ctx->cmd_fd ) ctx->cmd_fd = -1;
    if( fd == ctx->rpl_fd ) ctx->rpl_fd = -1;
    if( fd == ctx->lfd    ) ctx->lfd = -1;
}

void register_fd( AgentCtx *ctx, int fd, uint32_t events)
{
    struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));

    ev.events  = events;
    ev.data.fd = fd;

    int rc = epoll_ctl(ctx->epfd, EPOLL_CTL_ADD,fd, &ev);
	if( rc < 0)
	{
		 disarm_and_close( ctx, fd);
    } 
	LOG_INFO("[event] register event fd [%d]..", fd );
}



/* --- timer event ---*/
int handle_timer_fd( AgentCtx *ctx,int tfd)
{
    uint64_t ticks;
    CommHdr ch;
	DataHdr dh;
    char    body[65536];
	int     blen;

	if( read(tfd , &ticks, sizeof(ticks)) < 0) return;

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
            pool_flag = -1;
            rc = sendn_timed( ctx->cmd_fd, sndbuf, snd_len, IO_TIMEOUT_MS );
	        if( rc < 0)
	        {
	           LOG_INFO( "CMD live 전송오류..reconnect ...");
	           reconnect_backend(ctx);
			   return -1;
            }

            LOG_INFO("CMD POLL send ");
            rc = recv_packet_fd_timed( ctx->cmd_fd,&ch,&dh, body, &blen, sizeof(body), IO_TIMEOUT_MS);
            if( rc < 0)
            {
                LOG_ERR("CMD_POOL recv  error reconnecting..rc(%d)", rc);
   	            reconnect_backend( ctx);
 	            return -1;
            }

            if( !strncmp( ch.MsgType, "POOK",4) )
	        {
	            ctx->last_client_ts = now;
                LOG_INFO("CMD POOK recv");
            }
            else
	        {
	            disarm_and_close(ctx, ctx->cmd_fd);
   	            reconnect_backend( ctx);
            }
            pool_flag = 1;

         }
	 }
     if ( pool_flag == 1)
     {
        ;
     }
   
     return 0;
}


/* --- reply event ---*/
int  handle_rpl_fd( AgentCtx *ctx, int fd)
{
    CommHdr ch;
	DataHdr dh;
	char    body[2048];
	int     blen;
    char cid[11];
	memset( cid, 0x00 , sizeof(cid));

    int rc = recv_packet_fd_timed(fd, &ch , &dh, body,&blen, sizeof(body), IO_TIMEOUT_MS);
    if( rc < 0)
    {
        LOG_INFO("recevie packet  에러  재거넥션 ...rc [%d] ch[%s] ", rc, &ch  );
		// reconnect_backend( ctx);
        return -1;
    }

    if( !strncmp( ch.MsgType, "POLL",4) )
    {
	    LOG_INFO("RPL POLL recv");
		/* pool 응답 */
		memcpy( ch.MsgType, "POOK",4);
        rc = send_packet_fd_timed( fd, &ch, &dh, body, blen, IO_TIMEOUT_MS );
	    if( rc == -ETIMEDOUT  || rc < 0)
	    {
            disarm_and_close(ctx, fd);
		    return -1;
        }
		//LOG_INFO("RPL POOK send");
	    return;
	}
    else if( !strncmp( ch.MsgType, "LIOK",4) )
	{
	    LOG_INFO("RPL LIOK OK");
	    return;
	}
	else  // DATA
	{
	    LOG_INFO("client 응답...");
		/* pool 응답 */
		memcpy( ch.MsgType, "DAOK",4);
        rc = send_packet_fd_timed( fd, &ch, &dh, body, blen, IO_TIMEOUT_MS );
	    if( rc == -ETIMEDOUT  || rc < 0)
	    {
            disarm_and_close(ctx, fd);
		    return -1;
        }
		//LOG_INFO("RPL 응답 end");


        memcpy( cid, dh.Cid ,10); 
        /* rpl 전송 */
        if( cid && strlen(cid) > 0)
        {
	        int cfd= cid_find_fd( cid);
            if( cfd >= 0 )
            {
                rc = send_packet_fd_timed(cfd, &ch, &dh,body, blen,IO_TIMEOUT_MS);
			    if( rc <0)
                {
		            LOG_INFO("client 응답 오류 ... rc=%d", rc );
                    disarm_and_close( ctx, cfd);
                    return -1;
                }
                else
                {
		            LOG_INFO("client 응답 전송 완료[%s]fd[%d]", cid,cfd);
			        return  -1;
                }
            }
            else
            {
		        LOG_INFO("client 응답 cid를 찾을수 없음... cid[%s]", cid);
			    return  -1;
            }
        } 
        else         /* cid가 없는 msg 수신 */
        {
		        int  len = 0;
		        char msg[sizeof(DataHdr)+2048];

		        memcpy( msg,&dh, sizeof(DataHdr));
		        len += sizeof(DataHdr);
		        memcpy( msg+len, body, blen );
		        LOG_INFO("RPL 기타msg 처리 하지 않음 [%s](%d)", msg,len+blen);
		        if( msg == NULL ) return 0;
           #if 0  
                rc = msg_queue_send( ctx->msg_id , msg, len+blen );
                if( rc != 0)
                {
		            LOG_ERR("client  msg_quue_send 오류.. rc = %d",  rc );
                    return 0;
                }
           #endif
		}
    }
	return 0;
}

int handle_listen_fd( AgentCtx *ctx, int lfd)
{
    int cfd= accept( lfd, NULL,NULL);
    if( cfd < 0 ) return -1;


    register_fd( ctx, cfd, EPOLLIN );
	LOG_INFO("[agent] client connected fd=%d", cfd);
	return 0;
}

/* --- 송신 event --- */
int  handle_cmd_fd( AgentCtx *ctx, int fd)
{
    CommHdr ch;
	DataHdr dh;
    char    body[65536];
	int     blen;

    LOG_INFO("==>handle cmd_fd  handle start... ");

	return 0;
}

/* --- client  event -------------------*/
int handle_client_fd( AgentCtx *ctx, int cfd )
{
    CommHdr ch;
	DataHdr dh;
    char    body[65536];
	int     blen;
	char    cid[11];

    
    LOG_INFO("[client] 요청..." );
    int rc = recv_packet_fd_timed( cfd, &ch, &dh, body, &blen, sizeof(body),  IO_TIMEOUT_MS);
    if( rc < 0)
    {
        if( rc == -2) LOG_INFO("[client] connect out." );
		cid_del_by_fd(cfd);
        disarm_and_close(ctx, cfd);
        return -1;
    }
	    
    cid_generate( dh.Cid );
	memcpy( cid, dh.Cid , 10); cid[10] = 0x00;

	LOG_INFO( "cid generated [%.10s]", cid);
    cid_add_with_ttl( dh.Cid, cfd, CID_DEADLINE_SEC );
  
    ctx->last_client_ts = time(NULL);
    if( memcmp( ch.MsgType,"DATA",4) == 0)
	{
	    ctx->data_seq ++;
		fill_seq ( ch.SeqNo, ctx->data_seq );
    }

	/* backend 송신 */
    rc = send_packet_fd_timed( ctx->cmd_fd, &ch, &dh, body, blen, IO_TIMEOUT_MS );
	if( rc == -ETIMEDOUT  || rc < 0)
	{
		reply_timeout_to_client(ctx, cfd, cid);
		cid_del_by_fd(cfd);
        disarm_and_close(ctx, cfd);
        /* 
		 * reconnect backend 
		 */
		return -1;
	}

    /* backend 수신 */
    rc = recv_packet_fd_timed( ctx->cmd_fd, &ch, &dh, body, &blen, sizeof(body),  IO_TIMEOUT_MS);
    if( rc < 0)
    {
		reply_timeout_to_client(ctx, cfd, cid);
		cid_del_by_fd(cfd);
        disarm_and_close(ctx, cfd);
        /* 
		 * reconnect backend 연결여부  
		 */
        return -1;
    }
    if(memcmp(ch.MsgType,"DAOK", 4)==0 )
	{
	    LOG_INFO("DAOK");
	}

	/* client  송신 */
    rc = send_packet_fd_timed( cfd, &ch, &dh, body, blen, IO_TIMEOUT_MS );
	if( rc == -ETIMEDOUT  || rc < 0)
	{
		reply_timeout_to_client(ctx, cfd, cid);
		cid_del_by_fd(cfd);
        disarm_and_close(ctx, cfd);
		return -1;
	}
    LOG_INFO("[client]접수 응답  완료... ");

	return 0;

}

/* ------- cid timeout ----------------*/
int  handle_cid_timeouts( AgentCtx *ctx)
{
    char cid[11];
	int  cfd;
	int  rc =0;

	while( cid_pop_expired(cid, &cfd) ) 
	{
	    if(cfd > 0)
		{
		    reply_timeout_to_client(ctx, cfd, cid);
			disarm_and_close(ctx,cfd);
        }
		else
		{
		    ;
		}
     }
	 return 0;
}

/* -------  backend 재 연결 ----- */
int reconnect_backend(AgentCtx *ctx)
{
LOG_INFO( "backend reconnect  start... ");
   if(ctx->cmd_fd >=0) disarm_and_close(ctx, ctx->cmd_fd);

if( ctx->recv_yn != 'N')
   if(ctx->rpl_fd >=0) disarm_and_close(ctx, ctx->rpl_fd);

   /* 200ms ~ 5s */
   static int backoff_ms = 200;
   usleep( backoff_ms *1000);
   if( backoff_ms < 5000)  backoff_ms *=2;

    ctx->cmd_fd = tcp_connect( cmd_host, cmd_port);
	if( ctx->cmd_fd < 0) return -1;
#if 0
	struct epoll_event ev={.events=EPOLLIN, .data.fd=ctx->cmd_fd};
	if( epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, ctx->cmd_fd, &ev) < 0)
	{
	    close(ctx->cmd_fd);
		ctx->cmd_fd = -1;
		return -1;
    }
#endif
if( ctx->recv_yn != 'N')
{
LOG_INFO( "backend reconnect 1... ");
    ctx->rpl_fd = tcp_connect( rpl_host, rpl_port);
	if( ctx->rpl_fd < 0) return -1;
	struct epoll_event ev={.events=EPOLLIN, .data.fd=ctx->rpl_fd};
	if( epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, ctx->rpl_fd, &ev) < 0)
	{
	    disarm_and_close( ctx, ctx->cmd_fd);
	    close(ctx->rpl_fd);
		ctx->rpl_fd = -1;
		return -1;
    }
}

    if(send_link(ctx) < 0)
	{
	    disarm_and_close( ctx, ctx->cmd_fd);
	    disarm_and_close( ctx, ctx->rpl_fd);
		return -1;
    }
    ctx->data_seq = 0;
    backoff_ms = 200;

LOG_INFO( "backend reconnect ok ");
	return 0;
}

int handle_client_bat_fd( AgentCtx *ctx, int cfd )
{
    int     rc = 0;
    char    recv[2048];
    char    send[2048];
	int     len  = 0;
	char    cid[11];
	char    lenbuf[5] = {0};
	int     recvlen = 0;

    LOG_INFO("[client]bat 수신 ..." );
    rc = readn_timed( cfd, lenbuf, 4, IO_TIMEOUT_MS);
    if( rc < 0)
    {
	    if( rc == -2) 
		{
		    LOG_INFO("client connect out..");
            disarm_and_close(ctx, cfd);
            return -1;
        }
        else
        {
	        LOG_ERR("client 수신 read1 error !! rc = %d", rc);
            disarm_and_close(ctx, cfd);
        }
        return 0;
    }
	recvlen = atoi(lenbuf);

    rc = readn_timed( cfd, recv, recvlen, IO_TIMEOUT_MS);
    if( rc < 0)
    {
	    LOG_ERR("client 수신 read2 error !! rc = %d", rc);
        disarm_and_close(ctx, cfd);
        return  -1;
    }

    memcpy( send,   lenbuf , 4);
    memcpy( send+4, recv   , recvlen); 
    len = recvlen + 4;
	/* 송신 */
    rc = sendn_timed( ctx->cmd_fd, send, len, IO_TIMEOUT_MS );
	if( rc == -ETIMEDOUT  || rc < 0)
	{
	    LOG_ERR("oms 송신 wrtte error !! rc = %d", rc);
        disarm_and_close(ctx, cfd);
		return  -1;
	}

    /* 수신 */
    rc = readn_timed( ctx->cmd_fd, recv, 50,  IO_TIMEOUT_MS);
    if( rc < 0)
    {
	    LOG_ERR("oms 수신 read  error !! rc = %d", rc);
        disarm_and_close(ctx, cfd);
        return -1;
    }

    if(memcmp( &recv[4],"DAOK", 4)==0 )
    {
        LOG_INFO("DAOK");
    }

    /* client  송신 */
    rc = sendn_timed( cfd, recv, 50, IO_TIMEOUT_MS );
    if( rc == -ETIMEDOUT  || rc < 0)
    {
        // return -1;
        LOG_INFO("[client] timeout..");
    }
    LOG_INFO("[client]접수 응답  완료... ");

    return 0;
}

