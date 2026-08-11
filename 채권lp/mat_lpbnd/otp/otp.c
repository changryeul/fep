/** ***************************************************************************
**  @file       task.c
**  @date       2024/01/25
**  @author     ibk
**  @version    
**  @brif
**  OTP 인증을 위한 library
**  asis의 otp_agent.cpp와 iniparser.cpp를 하나의 파일로 합친  module
**  otp_agent.h와 iniparser.h는 otp.h로 통합
**  sdotpagent.ini은 환경파일 OTP_INI_PATH로 변경
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "otp.h"

/********** USER DEFINE VALIABLE **********/
/******************************************/

/** ***************************************************************************
**  @function   CMD 사용을 위한 초기화  ... 변경 불필요
**  @brief      HELP/QUIT
***************************************************************************** */

/** ***************************************************************************
**  @function   기능별 함수 작성
**  @brief      TODO
**  1. 함수 프로토타입 정의
**  2. CmdTable에 등록
**  3. 함수 작성
***************************************************************************** */
/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     1 - 성공
**  @retval     -1 - 실패
**  @brief      
**  함수 작성   
***************************************************************************** */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/** ***************************************************************************
**  @module		otp_agent.cpp
**  @brief      asis otp_agent.cpp
***************************************************************************** */
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
        if( (d = iniparser_new( getenv( "OTP_INI_PATH")) ) == NULL ) {
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
        server_address.sin_port = htons( cArg->otpc_server_port);

        /* Create socket and Return file descriptor */
        if( (server_sockfd  = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
                err_Handler("socket: fail", "socket", errno);
                return -1;
        }
		LogDbg( "server_sockfd=[%d]", server_sockfd);

		
		LogDbg( "connect sock=[%d] addr=[%s] port=[%d] ", server_sockfd, cArg->otpc_server_ip, cArg->otpc_server_port);
        if (connect(server_sockfd , (struct sockaddr *)&server_address, sizeof(struct sockaddr)) == -1) {
                close(server_sockfd);
				LogErr( "connect fail ...");
                err_Handler("connect: fail", "connect", errno);
                return -1;
        }
		LogDbg( "connect success ...");


		LogDump( sndData, strlen( sndData), "send data ... sz=[%d]", strlen( sndData));
        if( snd_otpc(server_sockfd, sndData, strlen(sndData)) < 0 ) {
                close(server_sockfd);
                err_Handler("snd_otpc: fail", "snd_otpc", errno);
                return -1;
        }


        if( (errCode = recvtimeout_otp(server_sockfd,  recvData, cArg->transaction_timeout)) <= 0 ) {
                if( errCode == -2 ) {
						LogDbg( "recv error ... errCode=[%d]", errCode);
                        close(server_sockfd);
                        err_Handler("recvtimeout_otp: timeout fail", "recvtimeout_otp", errno);
                        return -2;  /* timeout */
                }
                else {
						LogDbg( "recv error ... errCode=[%d]", errCode);
                        err_Handler("recvtimeout_otp: fail", "recvtimeout_otp", errno);
                        close(server_sockfd);
                        return -1;
                }
                
        }
		LogDump( recvData, errCode, "recv success ... errCode=[%d]", errCode);

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

	LogCri( "func=[%s] err=[%d:%s]", err_func, err_num, err_msg);
}
/* end of err_Handler ***********************************************************/






/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/** ***************************************************************************
**  @module		otp_agent.cpp
**  @brief      asis otp_agent.cpp
***************************************************************************** */
/*
 Based upon libiniparser, by Nicolas Devillard
 Hacked into 1 file (m-iniparser) by Freek/2005
 Original terms following:

 -- -

 Copyright (c) 2000 by Nicolas Devillard (ndevilla AT free DOT fr).

 Written by Nicolas Devillard. Not derived from licensed software.

 Permission is granted to anyone to use this software for any
 purpose on any computer system, and to redistribute it freely,
 subject to the following restrictions:

 1. The author is not responsible for the consequences of use of
 this software, no matter how awful, even if they arise
 from defects in it.

 2. The origin of this software must not be misrepresented, either
 by explicit claim or by omission.

 3. Altered versions must be plainly marked as such, and must not
 be misrepresented as being the original software.

 4. This notice may not be removed or altered.

 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iniparser.h"



/* strlib.c following */

#define ASCIILINESZ 1024


/*-------------------------------------------------------------------------*/
/**
  @brief    Convert a string to lowercase.
  @param    s   String to convert.
  @return   ptr to statically allocated string.

  This function returns a pointer to a statically allocated string
  containing a lowercased version of the input string. Do not free
  or modify the returned string! Since the returned string is statically
  allocated, it will be modified at each function call (not re-entrant).
 */
/*--------------------------------------------------------------------------*/

static char * strlwc(char * s)
{
    static char l[ASCIILINESZ+1];
    int i ;

    if (s==NULL) return NULL ;
    memset(l, 0, ASCIILINESZ+1);
    i=0 ;
    while (s[i] && i<ASCIILINESZ) {
        l[i] = (char)tolower((int)s[i]);
        i++ ;
    }
    l[ASCIILINESZ]=(char)0;
    return l ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Convert a string to uppercase.
  @param    s   String to convert.
  @return   ptr to statically allocated string.

  This function returns a pointer to a statically allocated string
  containing an uppercased version of the input string. Do not free
  or modify the returned string! Since the returned string is statically
  allocated, it will be modified at each function call (not re-entrant).
 */
/*--------------------------------------------------------------------------*/

static char * strupc(char * s)
{
    static char l[ASCIILINESZ+1];
    int i ;

    if (s==NULL) return NULL ;
    memset(l, 0, ASCIILINESZ+1);
    i=0 ;
    while (s[i] && i<ASCIILINESZ) {
        l[i] = (char)toupper((int)s[i]);
        i++ ;
    }
    l[ASCIILINESZ]=(char)0;
    return l ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Skip blanks until the first non-blank character.
  @param    s   String to parse.
  @return   Pointer to char inside given string.

  This function returns a pointer to the first non-blank character in the
  given string.
 */
/*--------------------------------------------------------------------------*/

static char * strskp(char * s)
{
    char * skip = s;
    if (s==NULL) return NULL ;
    while (isspace((int)*skip) && *skip) skip++;
    return skip ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Remove blanks at the end of a string.
  @param    s   String to parse.
  @return   ptr to statically allocated string.

  This function returns a pointer to a statically allocated string,
  which is identical to the input string, except that all blank
  characters at the end of the string have been removed.
  Do not free or modify the returned string! Since the returned string
  is statically allocated, it will be modified at each function call
  (not re-entrant).
 */
/*--------------------------------------------------------------------------*/

static char * strcrop(char * s)
{
    static char l[ASCIILINESZ+1];
    char * last ;

    if (s==NULL) return NULL ;
    memset(l, 0, ASCIILINESZ+1);
    strcpy(l, s);
    last = l + strlen(l);
    while (last > l) {
        if (!isspace((int)*(last-1)))
            break ;
        last -- ;
    }
    *last = (char)0;
    return l ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Remove blanks at the beginning and the end of a string.
  @param    s   String to parse.
  @return   ptr to statically allocated string.

  This function returns a pointer to a statically allocated string,
  which is identical to the input string, except that all blank
  characters at the end and the beg. of the string have been removed.
  Do not free or modify the returned string! Since the returned string
  is statically allocated, it will be modified at each function call
  (not re-entrant).
 */
/*--------------------------------------------------------------------------*/
static char * strstrip(char * s)
{
    static char l[ASCIILINESZ+1];
    char * last ;

    if (s==NULL) return NULL ;

    while (isspace((int)*s) && *s) s++;

    memset(l, 0, ASCIILINESZ+1);
    strcpy(l, s);
    last = l + strlen(l);
    while (last > l) {
        if (!isspace((int)*(last-1)))
            break ;
        last -- ;
    }
    *last = (char)0;

    return (char*)l ;
}


/* dictionary.c.c following */
/** Maximum value size for integers and doubles. */
#define MAXVALSZ    1024

/** Minimal allocated number of entries in a dictionary */
#define DICTMINSZ   128

/** Invalid key token */
#define DICT_INVALID_KEY    ((char*)-1)

/*
 Doubles the allocated size associated to a pointer
 'size' is the current allocated size.
 */
static void * mem_double(void * ptr, int size)
{
    void *newptr;

    newptr = calloc(2*size, 1);
    memcpy(newptr, ptr, size);
    free(ptr);
    return newptr ;
}


/*---------------------------------------------------------------------------
                            Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the hash key for a string.
  @param    key     Character string to use for key.
  @return   1 unsigned int on at least 32 bits.

  This hash function has been taken from an Article in Dr Dobbs Journal.
  This is normally a collision-free function, distributing keys evenly.
  The key is stored anyway in the struct so that collision can be avoided
  by comparing the key itself in last resort.
 */
/*--------------------------------------------------------------------------*/

static unsigned dictionary_hash(char * key)
{
    int         len ;
    unsigned    hash ;
    int         i ;

    len = strlen(key);
    for (hash=0, i=0 ; i<len ; i++) {
        hash += (unsigned)key[i] ;
        hash += (hash<<10);
        hash ^= (hash>>6) ;
    }
    hash += (hash <<3);
    hash ^= (hash >>11);
    hash += (hash <<15);
    return hash ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Create a new dictionary object.
  @param    size    Optional initial size of the dictionary.
  @return   1 newly allocated dictionary objet.

  This function allocates a new dictionary object of given size and returns
  it. If you do not know in advance (roughly) the number of entries in the
  dictionary, give size=0.
 */
/*--------------------------------------------------------------------------*/

static dictionary * dictionary_new(int size)
{
    dictionary *d ;

    /* If no size was specified, allocate space for DICTMINSZ */
    if (size<DICTMINSZ) size=DICTMINSZ ;

    d = (dictionary *)calloc(1, sizeof(dictionary));
    d->size = size ;
    d->val  = (char **)calloc(size, sizeof(char*));
    d->key  = (char **)calloc(size, sizeof(char*));
    d->hash = (unsigned int *)calloc(size, sizeof(unsigned));

    return d;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Delete a dictionary object
  @param    d   dictionary object to deallocate.
  @return   void

  Deallocate a dictionary object and all memory associated to it.
 */
/*--------------------------------------------------------------------------*/

static void dictionary_del(dictionary * d)
{
    int     i ;

    if (d==NULL) return ;
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]!=NULL)
            free(d->key[i]);
        if (d->val[i]!=NULL)
            free(d->val[i]);
    }
    free(d->val);
    free(d->key);
    free(d->hash);
    free(d);

    return;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Get a value from a dictionary.
  @param    d       dictionary object to search.
  @param    key     Key to look for in the dictionary.
  @param    def     Default value to return if key not found.
  @return   1 pointer to internally allocated character string.

  This function locates a key in a dictionary and returns a pointer to its
  value, or the passed 'def' pointer if no such key can be found in
  dictionary. The returned character pointer points to data internal to the
  dictionary object, you should not try to free it or modify it.
 */
/*--------------------------------------------------------------------------*/
static char * dictionary_get(dictionary * d, char * key, char * def)
{
    unsigned    hash ;
    int         i ;

    hash = dictionary_hash(key);
    for (i=0 ; i<d->size ; i++) {
        if (d->key==NULL)
            continue ;
        /* Compare hash */
        if (hash==d->hash[i]) {
            /* Compare string, to avoid hash collisions */
            if (!strcmp(key, d->key[i])) {
                return d->val[i] ;
            }
        }
    }
    return def ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Set a value in a dictionary.
  @param    d       dictionary object to modify.
  @param    key     Key to modify or add.
  @param    val     Value to add.
  @return   void

  If the given key is found in the dictionary, the associated value is
  replaced by the provided one. If the key cannot be found in the
  dictionary, it is added to it.

  It is Ok to provide a NULL value for val, but NULL values for the dictionary
  or the key are considered as errors: the function will return immediately
  in such a case.

  Notice that if you dictionary_set a variable to NULL, a call to
  dictionary_get will return a NULL value: the variable will be found, and
  its value (NULL) is returned. In other words, setting the variable
  content to NULL is equivalent to deleting the variable from the
  dictionary. It is not possible (in this implementation) to have a key in
  the dictionary without value.
 */
/*--------------------------------------------------------------------------*/

static void dictionary_set(dictionary * d, char * key, char * val)
{
    int         i ;
    unsigned    hash ;

    if (d==NULL || key==NULL) return ;

    /* Compute hash for this key */
    hash = dictionary_hash(key) ;
    /* Find if value is already in blackboard */
    if (d->n>0) {
        for (i=0 ; i<d->size ; i++) {
            if (d->key[i]==NULL)
                continue ;
            if (hash==d->hash[i]) { /* Same hash value */
                if (!strcmp(key, d->key[i])) {   /* Same key */
                    /* Found a value: modify and return */
                    if (d->val[i]!=NULL)
                        free(d->val[i]);
                    d->val[i] = val ? strdup(val) : NULL ;
                    /* Value has been modified: return */
                    return ;
                }
            }
        }
    }
    /* Add a new value */
    /* See if dictionary needs to grow */
    if (d->n==d->size) {

        /* Reached maximum size: reallocate blackboard */
        d->val  = (char **)mem_double(d->val,  d->size * sizeof(char*)) ;
        d->key  = (char **)mem_double(d->key,  d->size * sizeof(char*)) ;
        d->hash = (unsigned int *)mem_double(d->hash, d->size * sizeof(unsigned)) ;

        /* Double size */
        d->size *= 2 ;
    }

    /* Insert key in the first empty slot */
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL) {
            /* Add key here */
            break ;
        }
    }
    /* Copy key */
    d->key[i]  = strdup(key);
    d->val[i]  = val ? strdup(val) : NULL ;
    d->hash[i] = hash;
    d->n ++ ;
    return ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Delete a key in a dictionary
  @param    d       dictionary object to modify.
  @param    key     Key to remove.
  @return   void

  This function deletes a key in a dictionary. Nothing is done if the
  key cannot be found.
 */
/*--------------------------------------------------------------------------*/
static void dictionary_unset(dictionary * d, char * key)
{
    unsigned    hash ;
    int         i ;

    hash = dictionary_hash(key);
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL)
            continue ;
        /* Compare hash */
        if (hash==d->hash[i]) {
            /* Compare string, to avoid hash collisions */
            if (!strcmp(key, d->key[i])) {
                /* Found key */
                break ;
            }
        }
    }
    if (i>=d->size)
        /* Key not found */
        return ;

    free(d->key[i]);
    d->key[i] = NULL ;
    if (d->val[i]!=NULL) {
        free(d->val[i]);
        d->val[i] = NULL ;
    }
    d->hash[i] = 0 ;
    d->n -- ;
    return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Dump a dictionary to an opened file pointer.
  @param    d   Dictionary to dump
  @param    f   Opened file pointer.
  @return   void

  Dumps a dictionary onto an opened file pointer. Key pairs are printed out
  as @c [Key]=[Value], one per line. It is Ok to provide stdout or stderr as
  output file pointers.
 */
/*--------------------------------------------------------------------------*/

static void dictionary_dump(dictionary *d, FILE *f)
{
    int i;

    if (d==NULL || f==NULL) return;

    for (i=0; i<d->size; i++) {
        if (d->key[i] == NULL)
            continue ;
        if (d->val[i] != NULL) {
            fprintf(f, "[%s]=[%s]\n", d->key[i], d->val[i]);
        } else {
            fprintf(f, "[%s]=UNDEF\n", d->key[i]);
        }
    }

    return;
}


/* iniparser.c.c following */
#define ASCIILINESZ         1024
#define INI_INVALID_KEY     ((char*)-1)

/* Private: add an entry to the dictionary */
static void iniparser_add_entry(
    dictionary * d,
    char * sec,
    char * key,
    char * val)
{
    char longkey[2*ASCIILINESZ+1];

    /* Make a key as section:keyword */
    if (key!=NULL) {
        sprintf(longkey, "%s:%s", sec, key);
    } else {
        strcpy(longkey, sec);
    }

    /* Add (key,val) to dictionary */
    dictionary_set(d, longkey, val);
    return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Get number of sections in a dictionary
  @param    d   Dictionary to examine
  @return   int Number of sections found in dictionary

  This function returns the number of sections found in a dictionary.
  The test to recognize sections is done on the string stored in the
  dictionary: a section name is given as "section" whereas a key is
  stored as "section:key", thus the test looks for entries that do not
  contain a colon.

  This clearly fails in the case a section name contains a colon, but
  this should simply be avoided.

  This function returns -1 in case of error.
 */
/*--------------------------------------------------------------------------*/

int iniparser_getnsec(dictionary * d)
{
    int i ;
    int nsec ;

    if (d==NULL) return -1 ;
    nsec=0 ;
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL)
            continue ;
        if (strchr(d->key[i], ':')==NULL) {
            nsec ++ ;
        }
    }
    return nsec ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Get name for section n in a dictionary.
  @param    d   Dictionary to examine
  @param    n   Section number (from 0 to nsec-1).
  @return   Pointer to char string

  This function locates the n-th section in a dictionary and returns
  its name as a pointer to a string statically allocated inside the
  dictionary. Do not free or modify the returned string!

  This function returns NULL in case of error.
 */
/*--------------------------------------------------------------------------*/

char * iniparser_getsecname(dictionary * d, int n)
{
    int i ;
    int foundsec ;

    if (d==NULL || n<0) return NULL ;
    foundsec=0 ;
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL)
            continue ;
        if (strchr(d->key[i], ':')==NULL) {
            foundsec++ ;
            if (foundsec>n)
                break ;
        }
    }
    if (foundsec<=n) {
        return NULL ;
    }
    return d->key[i] ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Dump a dictionary to an opened file pointer.
  @param    d   Dictionary to dump.
  @param    f   Opened file pointer to dump to.
  @return   void

  This function prints out the contents of a dictionary, one element by
  line, onto the provided file pointer. It is OK to specify @c stderr
  or @c stdout as output files. This function is meant for debugging
  purposes mostly.
 */
/*--------------------------------------------------------------------------*/
void iniparser_dump(dictionary * d, FILE * f)
{
    dictionary_dump(d,f);
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Save a dictionary to a loadable ini file
  @param    d   Dictionary to dump
  @param    f   Opened file pointer to dump to
  @return   void

  This function dumps a given dictionary into a loadable ini file.
  It is Ok to specify @c stderr or @c stdout as output files.
 */
/*--------------------------------------------------------------------------*/

void iniparser_dump_ini(dictionary * d, FILE * f)
{
    int     i, j ;
    char    keym[ASCIILINESZ+1];
    int     nsec ;
    char *  secname ;
    int     seclen ;

    if (d==NULL || f==NULL) return ;

    nsec = iniparser_getnsec(d);
    if (nsec<1) {
        /* No section in file: dump all keys as they are */
        for (i=0 ; i<d->size ; i++) {
            if (d->key[i]==NULL)
                continue ;
            fprintf(f, "%s = %s\n", d->key[i], d->val[i]);
        }
        return ;
    }
    for (i=0 ; i<nsec ; i++) {
        secname = iniparser_getsecname(d, i) ;
        seclen  = (int)strlen(secname);
        fprintf(f, "\n[%s]\n", secname);
        sprintf(keym, "%s:", secname);
        for (j=0 ; j<d->size ; j++) {
            if (d->key[j]==NULL)
                continue ;
            if (!strncmp(d->key[j], keym, seclen+1)) {
                fprintf(f,
                        "%-30s = %s\n",
                        d->key[j]+seclen+1,
                        d->val[j] ? d->val[j] : "");
            }
        }
    }
    fprintf(f, "\n");
    return ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, return NULL if not found
  @param    d   Dictionary to search
  @param    key Key string to look for
  @return   pointer to statically allocated character string, or NULL.

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  NULL is returned.
  The returned char pointer is pointing to a string allocated in
  the dictionary, do not free or modify it.

  This function is only provided for backwards compatibility with
  previous versions of iniparser. It is recommended to use
  iniparser_getstring() instead.
 */
/*--------------------------------------------------------------------------*/
char * iniparser_getstr(dictionary * d, char * key)
{
    return iniparser_getstring(d, key, NULL);
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key
  @param    d       Dictionary to search
  @param    key     Key string to look for
  @param    def     Default value to return if key not found.
  @return   pointer to statically allocated character string

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the pointer passed as 'def' is returned.
  The returned char pointer is pointing to a string allocated in
  the dictionary, do not free or modify it.
 */
/*--------------------------------------------------------------------------*/
char * iniparser_getstring(dictionary * d, char * key, char * def)
{
    char * lc_key ;
    char * sval ;

    if (d==NULL || key==NULL)
        return def ;

    lc_key = strdup(strlwc(key));
    sval = dictionary_get(d, lc_key, def);
    free(lc_key);
    return sval ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, convert to an int
  @param    d Dictionary to search
  @param    key Key string to look for
  @param    notfound Value to return in case of error
  @return   integer

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the notfound value is returned.
 */
/*--------------------------------------------------------------------------*/
int iniparser_getint(dictionary * d, char * key, int notfound)
{
    char    *   str ;

    str = iniparser_getstring(d, key, INI_INVALID_KEY);
    if (str==INI_INVALID_KEY) return notfound ;
    return atoi(str);
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, convert to a double
  @param    d Dictionary to search
  @param    key Key string to look for
  @param    notfound Value to return in case of error
  @return   double

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the notfound value is returned.
 */
/*--------------------------------------------------------------------------*/
double iniparser_getdouble(dictionary * d, char * key, double notfound)
{
    char    *   str ;

    str = iniparser_getstring(d, key, INI_INVALID_KEY);
    if (str==INI_INVALID_KEY) return notfound ;
    return atof(str);
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, convert to a boolean
  @param    d Dictionary to search
  @param    key Key string to look for
  @param    notfound Value to return in case of error
  @return   integer

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the notfound value is returned.

  A true boolean is found if one of the following is matched:

  - A string starting with 'y'
  - A string starting with 'Y'
  - A string starting with 't'
  - A string starting with 'T'
  - A string starting with '1'

  A false boolean is found if one of the following is matched:

  - A string starting with 'n'
  - A string starting with 'N'
  - A string starting with 'f'
  - A string starting with 'F'
  - A string starting with '0'

  The notfound value returned if no boolean is identified, does not
  necessarily have to be 0 or 1.
 */
/*--------------------------------------------------------------------------*/
int iniparser_getboolean(dictionary * d, char * key, int notfound)
{
    char    *   c ;
    int         ret ;

    c = iniparser_getstring(d, key, INI_INVALID_KEY);
    if (c==INI_INVALID_KEY) return notfound ;
    if (c[0]=='y' || c[0]=='Y' || c[0]=='1' || c[0]=='t' || c[0]=='T') {
        ret = 1 ;
    } else if (c[0]=='n' || c[0]=='N' || c[0]=='0' || c[0]=='f' || c[0]=='F') {
        ret = 0 ;
    } else {
        ret = notfound ;
    }
    return ret;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Finds out if a given entry exists in a dictionary
  @param    ini     Dictionary to search
  @param    entry   Name of the entry to look for
  @return   integer 1 if entry exists, 0 otherwise

  Finds out if a given entry exists in the dictionary. Since sections
  are stored as keys with NULL associated values, this is the only way
  of querying for the presence of sections in a dictionary.
 */
/*--------------------------------------------------------------------------*/

int iniparser_find_entry(
    dictionary  *   ini,
    char        *   entry
)
{
    int found=0 ;
    if (iniparser_getstring(ini, entry, INI_INVALID_KEY)!=INI_INVALID_KEY) {
        found = 1 ;
    }
    return found ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Set an entry in a dictionary.
  @param    ini     Dictionary to modify.
  @param    entry   Entry to modify (entry name)
  @param    val     New value to associate to the entry.
  @return   int 0 if Ok, -1 otherwise.

  If the given entry can be found in the dictionary, it is modified to
  contain the provided value. If it cannot be found, -1 is returned.
  It is Ok to set val to NULL.
 */
/*--------------------------------------------------------------------------*/

int iniparser_setstr(dictionary * ini, char * entry, char * val)
{
    dictionary_set(ini, strlwc(entry), val);
    return 0 ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Delete an entry in a dictionary
  @param    ini     Dictionary to modify
  @param    entry   Entry to delete (entry name)
  @return   void

  If the given entry can be found, it is deleted from the dictionary.
 */
/*--------------------------------------------------------------------------*/
void iniparser_unset(dictionary * ini, char * entry)
{
    dictionary_unset(ini, strlwc(entry));
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Parse an ini file and return an allocated dictionary object
  @param    ininame Name of the ini file to read.
  @return   Pointer to newly allocated dictionary

  This is the parser for ini files. This function is called, providing
  the name of the file to be read. It returns a dictionary object that
  should not be accessed directly, but through accessor functions
  instead.

  The returned dictionary must be freed using iniparser_free().
 */
/*--------------------------------------------------------------------------*/

dictionary * iniparser_new(char *ininame)
{
    dictionary  *   d ;
    char        lin[ASCIILINESZ+1];
    char        sec[ASCIILINESZ+1];
    char        key[ASCIILINESZ+1];
    char        val[ASCIILINESZ+1];
    char    *   where ;
    FILE    *   ini ;
    int         lineno ;

    if ((ini=fopen(ininame, "r"))==NULL) {
        return NULL ;
    }

    sec[0]=0;

    /*
     * Initialize a new dictionary entry
     */
    d = dictionary_new(0);
    lineno = 0 ;
    while (fgets(lin, ASCIILINESZ, ini)!=NULL) {
        lineno++ ;
        where = strskp(lin); /* Skip leading spaces */
        if (*where==';' || *where=='#' || *where==0)
            continue ; /* Comment lines */
        else {
            if (sscanf(where, "[%[^]]", sec)==1) {
                /* Valid section name */
                strcpy(sec, strlwc(sec));
                iniparser_add_entry(d, sec, NULL, NULL);
            } else if (sscanf (where, "%[^=] = \"%[^\"]\"", key, val) == 2
                   ||  sscanf (where, "%[^=] = '%[^\']'",   key, val) == 2
                   ||  sscanf (where, "%[^=] = %[^;#]",     key, val) == 2) {
                strcpy(key, strlwc(strcrop(key)));
                /*
                 * sscanf cannot handle "" or '' as empty value,
                 * this is done here
                 */
                if (!strcmp(val, "\"\"") || !strcmp(val, "''")) {
                    val[0] = (char)0;
                } else {
                    strcpy(val, strcrop(val));
                }
                iniparser_add_entry(d, sec, key, val);
            }
        }
    }
    fclose(ini);
    return d ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Free all memory associated to an ini dictionary
  @param    d Dictionary to free
  @return   void

  Free all memory associated to an ini dictionary.
  It is mandatory to call this function before the dictionary object
  gets out of the current context.
 */
/*--------------------------------------------------------------------------*/

void iniparser_free(dictionary * d)
{
    dictionary_del(d);
}

