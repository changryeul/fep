/* otp_agent.c */
/********************************************************/
/* Copyright 2007 Cyclops Inc.                    */
/********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <termios.h>
/*
#include <sys/termio.h>
*/
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>

#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <signal.h>

#include "iniparser.h"
#include "otp_agent.h"

#include "log.h"

#ifdef __64BIT_KERNEL
        off_t   st_size;        /* 64 bit file size     */
#else
        int     st_size;        /* 32 bit file size     */
#endif

/* Log Max file size limit : 200K */

#define MAX_Q_BUFFER    1024

/* MAX Zombi process state number */
#define MAX_STAT                600
#define BACKLOG 1024

#define   UNAME_LEN             12
#define TCODE_LEN               6

/* 거래구분 코드 값 */
#define AUTH_BIZ_CODE           (char *)"OTPS01"
#define RESYNC_BIZ_CODE         (char *)"OTPS02"


typedef struct CHANNEL_PKT {
        char DOC_LEN[4];                /* 전문길이 필드를 제외한 길이 */

        char RES_CODE[LEN_RES_CODE];            /* 응답코드, 인터넷 뱅킹 */
        char ERR_COUNT[LEN_ERR_COUNT];

        char BIZ_CODE[LEN_BIZ_CODE];             /* 거래구분 코드 , OTPS01(인증), OTPS02(보정)  */
        char OTHER_ORG[LEN_OTHER_ORG];          /* 타행 1, 자행 0 */
        char VENDER_CODE[LEN_VENDER_CODE];
        char USER_CODE[LEN_USER_CODE];          /* 실명번호(텔레뱅킹), 이용자번호(인터넷뱅킹) */
        char TOKEN_SERIAL[LEN_TOKEN_SERIAL];
        char TOKEN_CODE[LEN_TOKEN_CODE];                /* OTP 응답값 */
        char TOKEN_CODE2[LEN_TOKEN_CODE];                /* OTP 응답값2 */
        char LAST_AUTH_DATE[LEN_AUTH_DATE];             /* 마지막 인증 성공일자 */
        char LAST_AUTH_TIME[LEN_AUTH_TIME];             /* 마지막 인증 성공시간 */
        char DUMMY[2];
}channelPkt;

#define CHANNEL_PKT_SIZE        sizeof(struct CHANNEL_PKT)-2

/* 전문 입력값 오류 */
char ERR_CODE_00[LEN_RES_CODE+2];
/* TIMEOUT 오류 */
char ERR_CODE_01[LEN_RES_CODE+2];
/* 유효한 세션 없음 오류 */
char ERR_CODE_02[LEN_RES_CODE+2];

/* network info & log info */
typedef struct CONFIG_ARG {
        char otpc_server_ip[50];
        int otpc_server_port;
        int transaction_timeout;
        int retry_count;
        int adm_log_flag;
        char adm_log_path[50];
        int doc_log_flag;
        char doc_log_path[50];
}configArg;

int check_parameter(stOtpData *in);
void Af_AuthOtpUser(stOtpData *in, stOtpData *out);
void Af_ResyncOtpUser(stOtpData *in, stOtpData *out);
int get_config(configArg *cArg);
int generate_document( char *bizCode, stOtpData *in,  channelPkt *cPkt);
int snd_otp(configArg *cArg, char *sndData, char *recvData);
int snd_otpc(int sockfd, char * sendData, int len);
int recvtimeout_otp(int sockfd,  char *buf, int timeout);
int recv_otp(int sockfd,  char *recvData);
void err_Handler(char * err_msg, char * err_func, int err_num);


/*********************************************************************************
* check_parameter()
*********************************************************************************/
int check_parameter(stOtpData *in)
{
        if( in == NULL )
                return 1;

        if( strncmp( in->otherOrg, IBK_ORG_CODE, 1 ) != 0 && strncmp( in->otherOrg, OTHER_ORG_CODE, 1 ) != 0  ) 
                return 1;

        if( in->venderCode == NULL || strlen(in->venderCode) != LEN_VENDER_CODE )
                return 1;

        if( in->tokSerial == NULL || strlen(in->tokSerial) > LEN_TOKEN_SERIAL)
                return 1;

        if( in->tokCode== NULL || strlen(in->tokCode) > LEN_TOKEN_CODE )
                return 1;

        return 0;
}


/*********************************************************************************
* Af_AuthOtpUser()
*********************************************************************************/
void Af_AuthOtpUser(stOtpData *in, stOtpData *out)
{
        int irtn;
        configArg cArg;
        channelPkt cPkt, *recvPkt;
        char *sndData;
        char recvData[MAX_Q_BUFFER];
        int retryCount = 0;

        memcpy( out, in, sizeof(stOtpData) );

        strncpy( out->errCount , "00", LEN_ERR_COUNT);
        
        if( check_parameter( in ) != 0 ) {
                strcpy( out->resCode, ERR_CODE_00 );
				LogDbg( "입력값 오류");
                return ;  /* 입력값 오류 */
        }
        get_config(&cArg);

#if 0
        if( get_config(&cArg) != 0 ) {
                strcpy( out->resCode, ERR_CODE_02 );
                return; /* 초기화 오류 */
        }
#endif
        /* 전문 생성 */
        generate_document(AUTH_BIZ_CODE, in, &cPkt);

        sndData = (char *)&cPkt;

        /* 전문 송신,수신 */
        irtn = -1; 
        while( irtn != 0 && retryCount < cArg.retry_count )   {
                if( (irtn = snd_otp(&cArg, sndData, recvData)) != 0 ) {
                        /* time out */
                        if( irtn == -2)  {  
                                strcpy( out->resCode, ERR_CODE_01 );
                                break;
                        }
                        /* session error */
                        else  {
                                strcpy( out->resCode, ERR_CODE_02 );
                        }
                }
                retryCount++;
        }

        recvPkt = (channelPkt *)recvData;

        if( irtn == 0 )  {
                strncpy( out->resCode, recvPkt->RES_CODE, LEN_RES_CODE );
                strncpy( out->lastAuthDate, recvPkt->LAST_AUTH_DATE, LEN_AUTH_DATE);
                strncpy( out->lastAuthTime, recvPkt->LAST_AUTH_TIME, LEN_AUTH_TIME);
                strncpy( out->errCount , recvPkt->ERR_COUNT, LEN_ERR_COUNT);
        }
        
        return;
        
}

/*********************************************************************************
* Af_ResyncOtpUser()
*********************************************************************************/
void Af_ResyncOtpUser(stOtpData *in, stOtpData *out)
{
        int irtn;
        configArg cArg;
        channelPkt cPkt, *recvPkt;
        char *sndData;
        char recvData[MAX_Q_BUFFER];
        int retryCount = 0;

        memcpy( out, in, sizeof(stOtpData) );

        if( check_parameter( in ) != 0 ) {
                strcpy( out->resCode, ERR_CODE_00 );
                return ;  /* 입력값 오류 */
        }

        get_config(&cArg);
#if 0
        if( get_config(&cArg) != 0 ) {
                strcpy( retCode, OTP_201 );
                return; /* 초기화 오류 */
        }
#endif
        /* 전문 생성 */
        generate_document(RESYNC_BIZ_CODE, in, &cPkt);

        sndData = (char *)&cPkt;

        /* 전문 송신,수신 */
        irtn = -1; 
        while( irtn != 0 &&  retryCount < cArg.retry_count )   {
                if( (irtn = snd_otp(&cArg, sndData, recvData)) < 0 ) {
                        /* time out */
                        if( irtn == -2)  {  
                                strcpy( out->resCode, ERR_CODE_01 );
                                break;
                        }
                        /* session error */
                        else  {
                                strcpy( out->resCode, ERR_CODE_02);
                        }
                }
                retryCount++;
        }

        recvPkt = (channelPkt *)recvData;

        if( irtn == 0 )  {
                strncpy( out->resCode, recvPkt->RES_CODE, LEN_RES_CODE );
                strncpy( out->lastAuthDate, recvPkt->LAST_AUTH_DATE, LEN_AUTH_DATE);
                strncpy( out->lastAuthTime, recvPkt->LAST_AUTH_TIME, LEN_AUTH_TIME);
                strncpy( out->errCount , recvPkt->ERR_COUNT, LEN_ERR_COUNT);
        }

        return;
}

/*********************************************************************************
* get_config()
*********************************************************************************/
int get_config(configArg *cArg)
{
        char *value;
        char tmp[256];
        dictionary * d ;
        
        memset( cArg, 0x00, sizeof(struct CONFIG_ARG) );
        
#if 0
        if( (d = iniparser_new("/var/ace/sdotpagent.ini") ) == NULL ) {
                return 1;
        }
#else
        if( (d = iniparser_new("./sdotpagent.ini") ) == NULL ) {
                return 1;
        }
#endif


        if( (value = iniparser_getstr(d, "OTPC_INFO:OTPC_SERVER")) != NULL ) {
                strcpy( cArg->otpc_server_ip, value );
        }

        cArg->otpc_server_port = iniparser_getint(d, "OTPC_INFO:OTPC_PORT", cArg->otpc_server_port);

        cArg->transaction_timeout = iniparser_getint(d, "TRN_INFO:TIME_OUT", cArg->transaction_timeout);

        cArg->retry_count = iniparser_getint(d, "TRN_INFO:RETRY_COUNT", cArg->retry_count );

        memset( ERR_CODE_00, 0x00, LEN_RES_CODE+2 );
        if( (value = iniparser_getstr(d, "ERR_INFO:ERR00" )) != NULL ) {
                        strncpy( ERR_CODE_00, value, LEN_RES_CODE);
        }

        memset( ERR_CODE_01, 0x00, LEN_RES_CODE+2 );
        if( (value = iniparser_getstr(d, "ERR_INFO:ERR01")) != NULL ) {
                        strncpy( ERR_CODE_01, value, LEN_RES_CODE);
        }

        memset( ERR_CODE_02, 0x00, LEN_RES_CODE+2 );
        if( (value = iniparser_getstr(d, "ERR_INFO:ERR02")) != NULL ) {
                        strncpy( ERR_CODE_02, value, LEN_RES_CODE);
        }

        iniparser_free(d);

        return 0;
}

/*********************************************************************************
* generate_document()
*********************************************************************************/
int generate_document( char *bizCode, stOtpData *in,  channelPkt *cPkt)
{
        int i, len;
        char chLen[5];
        memset(cPkt, 0x00, sizeof(channelPkt) );

        memset( chLen, 0x00, 5 );
        sprintf( chLen, "%04d", CHANNEL_PKT_SIZE-4 );

        strncpy( cPkt->DOC_LEN, chLen, 4 );
        memset( cPkt->RES_CODE, ' ', LEN_RES_CODE );
        memset( cPkt->ERR_COUNT, ' ', LEN_ERR_COUNT );
        strncpy(cPkt->BIZ_CODE, bizCode, LEN_BIZ_CODE);

        strncpy(cPkt->OTHER_ORG, in->otherOrg, LEN_OTHER_ORG);
        strncpy(cPkt->VENDER_CODE, in->venderCode, LEN_VENDER_CODE);
        
        strncpy(cPkt->USER_CODE, in->userCode, LEN_USER_CODE);
        if( (len = strlen(cPkt->USER_CODE)) < LEN_USER_CODE ) 
                memset(&cPkt->USER_CODE[len], ' ', LEN_USER_CODE-len);
        
        strncpy(cPkt->TOKEN_SERIAL, in->tokSerial, LEN_TOKEN_SERIAL);
        if( (len = strlen(cPkt->TOKEN_SERIAL)) < LEN_TOKEN_SERIAL ) 
                memset(&cPkt->TOKEN_SERIAL[len], ' ', LEN_TOKEN_SERIAL-len);
        
        strncpy(cPkt->TOKEN_CODE, in->tokCode, LEN_TOKEN_CODE);
        if( (len = strlen(cPkt->TOKEN_CODE)) < LEN_TOKEN_CODE ) 
                memset(&cPkt->TOKEN_CODE[len], ' ', LEN_TOKEN_CODE-len);

        memset(cPkt->TOKEN_CODE2, ' ', LEN_TOKEN_CODE);

        memset(cPkt->LAST_AUTH_DATE, ' ', LEN_AUTH_DATE);
        memset(cPkt->LAST_AUTH_TIME, ' ', LEN_AUTH_TIME);

        return 0;
}


/*********************************************************************************
* snd_otp()
*********************************************************************************/
int snd_otp(configArg *cArg, char *sndData, char *recvData)
{
        int errCode = 0;
        struct sockaddr_in server_address;
        int server_sockfd;
                
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = inet_addr(cArg->otpc_server_ip);
        server_address.sin_port = cArg->otpc_server_port;

        /* Create socket and Return file descriptor */
        if( (server_sockfd  = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
                err_Handler("socket: fail", "socket", errno);
                return -1;
        }

        if (connect(server_sockfd , (struct sockaddr *)&server_address, sizeof(struct sockaddr)) == -1) {
                close(server_sockfd);
                err_Handler("connect: fail", "connect", errno);
                return -1;
        }

        if( snd_otpc(server_sockfd, sndData, strlen(sndData)) < 0 ) {
                close(server_sockfd);
                err_Handler("snd_otpc: fail", "snd_otpc", errno);
                return -1;
        }


        if( (errCode = recvtimeout_otp(server_sockfd,  recvData, cArg->transaction_timeout)) <= 0 ) {
                if( errCode == -2 ) {
                        close(server_sockfd);
                        err_Handler("recvtimeout_otp: timeout fail", "recvtimeout_otp", errno);
                        return -2;  /* timeout */
                }
                else {
                        err_Handler("recvtimeout_otp: fail", "recvtimeout_otp", errno);
                        close(server_sockfd);
                        return -1;
                }
                
        }

        close(server_sockfd);
        
        return 0;

}
/* end of snd_otp ***************************************************/


/*********************************************************************************
* 전문을 송신
*********************************************************************************/
int snd_otpc(int sockfd, char * sendData, int len)
{
        
        int total = 0;        
        int bytesleft = len; 
        int n;

        while(total < len) {
            n = send(sockfd, sendData+total, bytesleft, 0);
            if (n == -1) { 
                        if( errno == EEXIST )
                                continue;
                        else
                                break; 
             }
            total += n;
            bytesleft -= n;
        }

        return n;  /* n=-1:fail */

}
/* end of snd_otpc *********************************************************/

/*********************************************************************************
* recvtimeout_otp()
*********************************************************************************/
int recvtimeout_otp(int sockfd,  char *buf, int timeout)
{
    fd_set fds;

    int n;
    struct timeval tv;

    /*set up the file descriptor set*/
    FD_ZERO(&fds);
    FD_SET(sockfd,&fds);

    /*set up the struct timeval for the timeout*/
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    /*wait until timeout or data received */
    n=select(sockfd+1,&fds,NULL,NULL,&tv);
    if(n==0) return -2; /*timeout!*/
    if(n==-1) return -1; /*error */

    /*data must be here, so do a normal recv()*/
    return recv_otp(sockfd,buf);
}

/*********************************************************************************
* OTP server 요구,응답 수신
*********************************************************************************/
int recv_otp(int sockfd,  char *recvData)
{
     /*data must be here, so do a normal recv()*/
    int bytesleft = 0; 
    int total = 0;
    int len = 0;
    int n;

    memset( recvData, 0x00, MAX_Q_BUFFER );

    if( ( total = recv(sockfd,recvData, 4, 0) ) <= 0 ) {
                /*printf("\n errno : %d", errno );
                if( errno == EEXIST || errno == EINTR   ) {
                        return 0;
                }
                else */
                
                return total;
    }
    len = atoi(recvData);
        
    bytesleft = len;

    while( total <  (len+4) ) {
                n = recv(sockfd, recvData+total, bytesleft, 0);
                if( n <= 0 ) 
                        break;
                total += n;
                bytesleft -= n;
    }

    /*if( n == -1 ) 
                printf("\n 12");*/
    return n; /* n == -1 : fail */
}


/*********************************************************************************
* err_Handler() 
*********************************************************************************/
void err_Handler(char * err_msg, char * err_func, int err_num){

        /*adm_log_func(err_func, err_msg, err_num);*/
         /*printf("\n");
         printf(err_msg);
         printf("    %d", err_num );*/

}
/* end of err_Handler ***********************************************************/
