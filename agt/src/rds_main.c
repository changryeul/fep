#define _POSIX_C_SOURCE 200809L
#include "netutil.h"
#include "protocol.h"
#include "log.h"
#include "config.h"

#include <poll.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>


#define MAX_EVENTS  256
#define IO_TIMEOUT_MS  10000 // 응답이 안오면 10초후 timeout을 내 보낸다.
//#define TIMEOUT_SEC 10


#if 0
#include "wfaapi.h"
#define     WG_DAT_DIR          "/fsfile/kei/wfg"        
#define     WG_SHMKEY           0x17001700                  // TODO : 임시 define
#define     WGDQ_RCVRQ          "WGRCVRQ"                   // Quote Request : FIX  -> Gate
#define     WGDQ_SNDRS          "WGSNDRS"                   // Quote Feeding : Gate -> FIX
#define     WGSEM_RCVRQ         0xff160043
#define     WGSEM_SNDRS         0xff160044
#endif


typedef struct 
{
    char pname[20];
	char section[7];
    int epfd;
    int lfd;
	int rfd;
	int tfd;
	int ffd;
	int msg_id;
	key_t msg_key;

    int data_seq;
	char filename[256];
    struct epoll_event evs[MAX_EVENTS];  
} Context;
	
extern Config g_cfg ;
static int stop_flag = 0;

static int handle_rds_rpl_fd( Context *ctx );
static int handle_rds_client_fd( Context *ctx );
static int handle_rds_listen_fd( Context *ctx );
static int on_sig(int s)
{ 
    (void)s; 
	stop_flag = 1; 
}

static int get_hhmmss_now(void)
{
    time_t t = time(NULL);
	struct tm *tm_info = localtime(&t);
	return tm_info->tm_hour*10000 + tm_info->tm_min*100 + tm_info->tm_sec;
}


static  void fill_seq ( char *dst, int seq )
{
    char tmp[16];
    snprintf(tmp, sizeof(tmp), "%08d", seq );
    memcpy( dst, tmp, 8);
}
static void file_close( Context *ctx )
{
    close( ctx->ffd);
	return ;
}

static int file_open( Context *ctx )
{

    char filename[256] = {0};
    int itoday = get_today();
    char today[9];
    sprintf( today,"%8d",itoday); today[8] = 0x00;

    if( !strncmp(ctx->section,"RDS_DC",6)  
	 || !strncmp(ctx->section,"RDS_IS",6))
	{
	    sprintf( filename, "%s/%s_%s.dat",ctx->filename, ctx->section,today);
	}
	else
    {
	    sprintf( filename, "%s",ctx->filename );
    }
    ctx->ffd = open ( filename, O_WRONLY | O_CREAT | O_APPEND,0644);
	if(ctx->ffd < 0)
	{
	    LOG_ERR("file open error filename[%s] [%d:%s]",filename, errno ,strerror(errno));
		return -1;
	}
	return 0;
}

static int write_to_file( Context *ctx, char *data , size_t len )
{
    int written = 0 ;
	char recvdata[2048] = {0};

    // file_open( ctx);

	memcpy( recvdata, data, len ); recvdata[len] = '\n';
    written = write ( ctx->ffd, recvdata, len+1 );
    if( written < 0)
	{
	    close(ctx->ffd);
		return -1;
    }
	return 0;
}

static int register_pollfds( Context *ctx )
{
    int rc = 0;
    struct epoll_event ev;

    memset(&ev, 0, sizeof(ev));
    ev.events  = EPOLLIN;
    ev.data.fd = ctx->lfd;
#if 0
    rc = epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, ctx->lfd, &ev);
    if( rc < 0)
    {
         LOG_INFO("[event] register  error..lfd[%d] ", ctx->lfd);
         return -1;
    }
#endif
    ev.data.fd = ctx->rfd;
    rc = epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, ctx->rfd, &ev);
    if( rc < 0)
    {
         LOG_INFO("[event] register  error..rfd[%d] ", ctx->rfd);
         return -1;
    }
    LOG_INFO("[event] register event ok..lfd/rfd[%d][%d] ", ctx->lfd,ctx->rfd );
    return 0;
}

/* ---  초기 LINK 송신 -- */
static int send_login  (Context *ctx)
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


    memcpy(ch.Length,  "0046"    ,4);
    memcpy(ch.MsgType, "LINK"    ,4);
    memcpy(ch.SeqNo  , "00000000",8);
    rc = send_packet_fd_timed( ctx->rfd, &ch, &dh, body ,0, IO_TIMEOUT_MS );
    if( rc < 0)
    {
        LOG_ERR("cmd send_link error!!");
        return -1;
    }
    return 0;

}
// ---------------------------------
// main 
// ---------------------------------
int main(int argc, char **argv)
{
    int     rc;
    Context ctx;
    memset( &ctx, 0x00, sizeof(Context));

    if( argc != 2)
	{
	    printf("Usage :  %s RDS_DC/RDS_IS/RDS_R1/RDS_R2 \n" , argv[0]);
	    printf("              -  RDS_DC: 채권 dropy copy 수신 pa_8111_ts\n" );
	    printf("              -  RDS_IS: 채권 장운영   수신  \n");
	    printf("              -  RDS_R1: 채권 종목정보 수신  \n");
	    printf("              -  RDS_R2: KTS  종목정보 수신  \n");
		return -1;
	}

    if( strncmp( argv[1], "RDS_DC", 6)  != 0
     && strncmp( argv[1], "RDS_IS", 6)  != 0 
     && strncmp( argv[1], "RDS_R1", 6)  != 0 
     && strncmp( argv[1], "RDS_R2", 6)  != 0 )
    {
	    printf("Usage :  %s RDS_DC/RDS_IS/RDS_R1/RDS_R2 \n" , argv[0]);
	    printf("              -  RDS_DC: 채권 dropy copy 수신 pa_8111_ts\n" );
	    printf("              -  RDS_IS: 채권 장운영   수신  \n");
	    printf("              -  RDS_R1: 채권 종목정보 수신  \n");
	    printf("              -  RDS_R2: KTS  종목정보 수신  \n");
	    return -1;
	}
	 
	strcpy( ctx.pname,   basename(argv[0]));
    memcpy( ctx.section, argv[1]     ,6);
    // init
    rc = main_init(  &ctx);
	if( rc != 0)
	{
		LOG_ERR(" main_init 오류 !!! rc= %d",rc);
		exit(-1);
	}

    while (!stop_flag)
	{
        int n = epoll_wait(ctx.epfd, ctx.evs, MAX_EVENTS, 10000 ); // 1000 ms
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
        for( int i= 0 ; i<n;i++)
        {
            int fd = ctx.evs[i].data.fd;
            uint32_t ev = ctx.evs[i].events;

            if(ev &(EPOLLERR | EPOLLHUP ) )
            {
	            LOG_INFO("epoll siganl STOP");
                main_fini( &ctx );
				return 0;
                // continue;
            }
            main_process( &ctx, fd );
        }

    } 
    // fini
	main_fini( &ctx);
	return 0;

}

int main_init( Context *ctx )
{
    int rc =0;
    int rfd = -1, lfd = -1;
    int cli_port;
    int rpl_port;
    char *rpl_host = "127.0.0.1";
	char namebuf[64] = {0};

    signal(SIGINT,  on_sig);
    signal(SIGTERM, on_sig);

    // config
    if (load_config("/fsfxwin/fep/agt/conf/config.ini") != 0) {
        fprintf(stderr, "Config load failed!\n");
        return 1;
    }

     // 로그 초기화
#if 0   // 여기를 풀면 화면에 display 됨.
    log_init_default();
#endif

	LOG_INFO("START...");
    if( !strncmp(ctx->section , "RDS_DC", 6))
    {   
	    sprintf( namebuf, "%s_dc", ctx->pname );
        log_set_level(g_cfg.rds_dc_log_level);
        if( log_open_daily(g_cfg.rds_dc_log_file, namebuf) != 0)
        {
            LOG_WARN("failed to open log file, keep stderr only [%s]", g_cfg.rds_dc_log_file);
        }
        log_set_level(g_cfg.rds_dc_log_level);
        memcpy( ctx->filename, g_cfg.rds_dc_file_name, sizeof(g_cfg.rds_dc_file_name)-1);
        cli_port     = g_cfg.rds_dc_cli_port;
        rpl_port     = g_cfg.rds_dc_port;
		ctx->msg_key = g_cfg.rds_dc_msg_key;
    }
	else if(!strncmp(ctx->section , "RDS_IS", 6)) 
	{
	    sprintf( namebuf, "%s_is", ctx->pname );
        log_set_level(g_cfg.rds_is_log_level);
        if( log_open_daily(g_cfg.rds_is_log_file, namebuf) != 0)
        {
            LOG_WARN("failed to open log file, keep stderr only [%s]", g_cfg.rds_is_log_file);
        }
        memcpy( ctx->filename, g_cfg.rds_is_file_name, sizeof(g_cfg.rds_is_file_name)-1);
        cli_port     = g_cfg.rds_is_cli_port;
        rpl_port     = g_cfg.rds_is_port;
		ctx->msg_key = g_cfg.rds_is_msg_key;
	}
	else if(!strncmp(ctx->section , "RDS_R1", 6)) 
	{
	    sprintf( namebuf, "%s_r1", ctx->pname );
        log_set_level(g_cfg.rds_r1_log_level);
        if( log_open_daily(g_cfg.rds_r1_log_file, namebuf) != 0)
        {
            LOG_WARN("failed to open log file, keep stderr only [%s]", g_cfg.rds_r1_log_file);
        }
        memcpy( ctx->filename, g_cfg.rds_r1_file_name, sizeof(g_cfg.rds_r1_file_name)-1);
        cli_port     = g_cfg.rds_r1_cli_port;
        rpl_port     = g_cfg.rds_r1_port;
		ctx->msg_key = 0;
	}
	else if(!strncmp(ctx->section , "RDS_R2", 6)) 
	{
	    sprintf( namebuf, "%s_r2", ctx->pname );
        log_set_level(g_cfg.rds_r2_log_level);
        if( log_open_daily(g_cfg.rds_r2_log_file, namebuf) != 0)
        {
            LOG_WARN("failed to open log file, keep stderr only [%s]", g_cfg.rds_r2_log_file);
        }
        memcpy( ctx->filename, g_cfg.rds_r2_file_name, sizeof(g_cfg.rds_r2_file_name)-1);
        cli_port     = g_cfg.rds_r2_cli_port;
        rpl_port     = g_cfg.rds_r2_port;
		ctx->msg_key = 0;
	}

   
	if(  !strncmp(ctx->section , "RDS_R1", 6) 
	  || !strncmp(ctx->section , "RDS_R2", 6) ) 
    {
		char hostname[256];
    	rc = gethostname(hostname, sizeof(hostname));
    	if(rc != 0)
    	{    
			LOG_DEBUG("[%s] hostname out error" , ctx->section);
			exit(0);
    	}    
	    int now = get_hhmmss_now();
	        //LOG_DEBUG("[%s] time(%d)" , ctx->section, now);
		if(memcmp(&hostname[3], "pko", 3) == 0)
		{
			if(now < 170000 && now > 34000)
			{
				//LOG_DEBUG("[%s] 기동시간(09:00)이 아닙니다.(%d)" , ctx->section, now);
				sleep(3);
				exit(0);
			}
		}
		else if (memcmp(&hostname[3], "pkd", 3) == 0 || memcmp(&hostname[3], "pks", 3) == 0)
		{
			if(now < 60000 && now > 230000)
			{
				//LOG_DEBUG("[%s] 기동시간(09:00)이 아닙니다.(%d)" , ctx->section, now);
				sleep(3);
				exit(0);
			}
		}
	}

	LOG_INFO("<<< START ctx->section[%s] >>>",  ctx->section );
// server
#if 0  //client 접속이 필요하면 열면 된다..
    ctx->lfd = mk_server( cli_port, 128);
	if( ctx->lfd < 0)
	{
	   LOG_ERR("[%s] mk_server  err!!!",ctx->section);
	   exit (-1);
	}
#endif
    ctx->rfd = tcp_connect( rpl_host,rpl_port );
    if( ctx->rfd < 0)
    {
        LOG_ERR("[%s] connect errror!!! rfd[%d] ", ctx->section,ctx->rfd );
	    exit (-1);
    }

    LOG_INFO("[%s]  connect  port = %d ok.",ctx->section, rpl_port);

#if 0
    rc = file_open(ctx);
	if( rc < 0)
	{
		LOG_ERR("수신파일 open 에러..rc= %d", rc );
	}
#endif

    // epoll 생성
    ctx->epfd  = epoll_create1(0);
    if( ctx->epfd < 0)
    {
       LOG_ERR("[%s] epoll_create  err!!!", ctx->section);
       exit (1);
    }

#if 0 /* 송신이면 내가 poll 해야 하지만 수신은 상대편이 poll하기에  */
    /* timer 등록 */
    ctx->last_client_ts = time(NULL);
    ctx->last_live_ts   =  0;
    ctx->data_seq = 0;

    ctx->tfd = timerfd_create( CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC );
    if( ctx->epfd < 0)
    {
        LOG_ERR(" timer create error !!");
        return -2;
    }
    struct itimerspec it = {0};
    it.it_value.tv_sec    = LIVE_PERIOD_SEC;
    it.it_interval.tv_sec = LIVE_PERIOD_SEC;
    timerfd_settime ( ctx->tfd, 0 ,&it, NULL);

    register_poll_fd(ctx->tfd );
    }
    else
    {
    }
#endif
    if(ctx->msg_key != 0 )
	{
        rc = msg_queue_init( ctx->msg_key);
	    if( rc < 0)
	    {
		    LOG_ERR("[%s] msg_quue_init 오류 !!! rc= %d",ctx->section,rc);
		    exit(-1);
	    }
	    ctx->msg_id  = rc;
         
        LOG_INFO( "msgkey .... init[%d]", ctx->msg_key);
	}
 
    // 이벤트 등록
    rc = register_pollfds( ctx );
	if( rc != 0)
	{
		LOG_ERR("[%s] register_pollfd 오류 !!! rc= %d",ctx->section,rc);
		exit(-1);
	}

    /* 최초 link */
    rc = send_login( ctx);
    if( rc != 0)
    {
          LOG_INFO("[%s] link fail.", ctx->section );
          return -1;
    }
    LOG_INFO("[%s]link  ok.", ctx->section );


    /* file open */
    rc = file_open(ctx);
	if( rc < 0)
	{
		LOG_ERR("수신파일 open 에러..rc= %d", rc );
	}
    LOG_INFO("[%s]file open ..filename[%s]", ctx->section, ctx->filename );


    return 0;
} 

int main_fini( Context *ctx )
{

     file_close( ctx ); 
	 close(ctx->lfd);
	 close(ctx->rfd);
	 LOG_INFO("main_fini ..signal STOP");
	 return 0;
}

int main_process( Context *ctx, int fd  )
{

    int rc = 0;
    if ( fd == ctx->lfd)   
    {
	    rc = handle_rds_listen_fd( ctx);
		if( rc == -1) return -1;
    }
    else if( fd == ctx->rfd)   
	{
	    rc = handle_rds_rpl_fd   ( ctx);
		if( rc == -1) return -1;
    }
    else                     
	{
	    rc = handle_rds_client_fd( ctx);
		if( rc == -1) return -1;
    }

    return 0;
}

static int handle_rds_client_fd( Context *ctx )
{
   ;
}
/* --- reply event ---*/
static int handle_rds_rpl_fd( Context *ctx )
{
    char    recvbuf[2048] = {0};
    char    sendbuf[2048] = {0};
	char    lenbuf[5] = {0};
    int     blen;

    int itoday = get_today();
	char today[9];
	sprintf( today,"%8d",itoday); today[8] = 0x00;

// LOG_INFO("rpl event start.. " );
    int rc = readn_timed(ctx->rfd, lenbuf, 4, IO_TIMEOUT_MS);
    if( rc < 0)
    {
		LOG_ERR("read  oms1 error. rc = %d",rc);
        return -1;
    }
	blen = atoi(lenbuf);

    rc = readn_timed(ctx->rfd, recvbuf, blen, IO_TIMEOUT_MS);
    if( rc < 0)
    {
		LOG_ERR("read  oms2 error. rc = %d",rc);
        return -1;
    }
	LOG_INFO("RD:[%s%s]",lenbuf,recvbuf);

	// poll data 이면  바로 응답을 주고 
    if( !strncmp( &recvbuf[0], "POLL",4) )  // POLL 응답
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
            // disarm_and_close(ctx, fd);
		    LOG_ERR("send  oms1 error. rc = %d",rc);
            return -1;
        }
        LOG_INFO("WD:[%s]",sendbuf);
        return 0; 
    }
	else if( !strncmp( &recvbuf[0],"LIOK", 4)) 
	{
		return 0;
	}
    else   // DATA 이면 응답을 주고.. 후처리를 한다.
    {
        char seqbuf[8+1] = {0};
        LOG_INFO("RPL 응답...");
        /* pool 응답 */
		memset( sendbuf, 0x20, 50);
        memcpy( &sendbuf[0], "0046",4);
        memcpy( &sendbuf[4], "DAOK",4);
        memcpy( &sendbuf[8], "0000",4);
        memcpy( &sendbuf[12], today ,8);

        ctx->data_seq ++;
        fill_seq ( seqbuf, ctx->data_seq );
        memcpy( &sendbuf[20], seqbuf ,8);

        rc = sendn_timed( ctx->rfd, sendbuf,50, IO_TIMEOUT_MS );
        if( rc == -ETIMEDOUT  || rc < 0)
        {
            //disarm_and_close(ctx, fd);
		    LOG_ERR("send  oms2 error. rc = %d",rc);
            return  -1;
        }
	    LOG_INFO("수신Data[%s]",recvbuf);
        //LOG_INFO("RPL 응답 end");

        int len = blen - 46;
        rc = write_to_file( ctx, &recvbuf[46], len );
		if( rc != 0)
		{
		    LOG_ERR("write to file error. rc = %d",rc);
			return -1;
        } 
	    LOG_INFO("file write ok..");
	

        if(ctx->msg_key != 0 ) 
        {
            rc = msg_queue_send( ctx->msg_id , &recvbuf[46], blen );
		    if( rc != 0)
		    {
		        LOG_ERR("msg_queue_send error. rc = %d",rc);
			    return -1;
            }
	        // LOG_INFO("mesq write ok[%s]",&recvbuf[46]);
        }
	    LOG_INFO(" 처리완료 ok..");
    }

	return 0;

}

static int handle_rds_listen_fd( Context *ctx )
{
    int cfd= accept( ctx->lfd, NULL,NULL);
    if( cfd < 0 ) return -1;

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    ev.events  = EPOLLIN;
    ev.data.fd = ctx->lfd;

    int rc = epoll_ctl(ctx->epfd, EPOLL_CTL_ADD,ctx->lfd, &ev);
    if( rc < 0)
    {
         //disarm_and_close( ctx, fd);
    }
    LOG_INFO("[agent] client connected fd=%d", cfd);

    return 0;
}
#if 0
     ctx->data_seq ++;
     fill_seq ( seqbuf, ctx->data_seq );
#endif


