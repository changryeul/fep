/*******************************************************************************
 * (C) COPYRIGHT Winway	Co., Ltd. 2009
 * All Rights Reserved
 * Licensed Materials - Property of Winway
 *
 * This	program	contains proprietary information of Winway System.
 * All embodying confidential information, ideas and expressions can't be
 * reproceduced, or transmitted in any form or by any means, electronic,
 * mechanical, or otherwise without the written permission of Winway System.
 *
 * Components	 : netconf.c
 * Description	 : network configure subroutine module
 * Rev. History :
 *		Ver		Date	Information
 *		-------	-------	-----------------------------------------------
 *		1.00	2009-12	Winway initial version.
 ******************************************************************************/
#include "context.h"
#include <sys/utsname.h>
/*******************************************************************************
 * User	Define
 ******************************************************************************/
#define	nw_errno	(h_errno + 300)

/*******************************************************************************
 * Global Variable
 ******************************************************************************/

/*******************************************************************************
 * Function prototype
 ******************************************************************************/
int	l_getip(char *);
int	l_ishost(char *hostnm);
int	l_getipbyname(char *hostnm,char *ipaddr);
int	l_getportbyname(char *svcnm,char *proto);
int	l_hostname(char *hostnm);
int	l_big_endian();
int	l_hostcfg(char *svcnm, char *ipaddr,int * portno);
int	l_getip_ledger(char *ipaddr);
int	l_readcfg(char *sessid,char *key,char *def,char *val, int maxlen, char *cfgfile );

/*******************************************************************************
 * NAME	: l_getip()
 * DESC	: CALL 한 프로그램이 운영되는 기기의 IP-ADDRESS
 * NOTE	:
 * IN	: (char *ip_addr)
 * OUT	:
 * RTN	: Opon success, it returns 0 else -1
 * VER	: 1.00 2009-12 Winway	initial	version.
 * AUTH.:
 ******************************************************************************/
int l_getip(ipaddr)
char	*ipaddr;
{
	struct	hostent *hostent;
	struct	in_addr	inet_addr;
	char	host_nm[64];
	int	retc;

	ipaddr[0] = '\0';
	retc = gethostname(host_nm, sizeof(host_nm));
	if (retc < 0)
	{
		return(-1);
	}
	hostent = gethostbyname(host_nm);
	if (hostent == NULL)
	{
		errno = nw_errno;
		return(-1);
	}

	memcpy(&inet_addr, hostent->h_addr, hostent->h_length);
	strcpy(ipaddr, (char *)inet_ntoa(inet_addr));
	return(0);
}

/*******************************************************************************
 * NAME	: l_ishost()
 * DESC	: 프로그램이 운영되는 서버 HOST 여부 check.
 * NOTE	:
 * IN	:
 * OUT	:
 * RTN	: If the hostname is equal, then returns 1 else 0
 * VER	: 1.00 2009-12 Winway	initial	version.
 * AUTH.:
 ******************************************************************************/
int l_ishost(char *hostnm)
{
	struct	hostent *hostent;
	struct	in_addr	inet_addr;
	char	myip[16], *ip_addr;

	if (hostnm == NULL || strlen(hostnm) == 0)
	{
		errno = EINVAL;
		return(-1);
	}
	if (l_getip(myip) < 0)
		return(-1);

	hostent = gethostbyname(hostnm);
	if (hostent == NULL)
	{
		errno = nw_errno;
		return(-1);
	}

	memcpy(&inet_addr, hostent->h_addr, hostent->h_length);
	ip_addr = (char *)inet_ntoa(inet_addr);
	if (strcmp(myip, ip_addr) == 0)
		return(1);

	return(0);
}

/*******************************************************************************
 * NAME	: l_getipbyname()
 * DESC	: get the ip address for a host with the hostname
 * NOTE	:
 * IN	:
 * OUT	:
 * RTN	: Opon success, it returns 0 else -1
 * VER	: 1.00 2009-12 Winway	initial	version.
 * AUTH.:
 ******************************************************************************/
int l_getipbyname(hostnm, ipaddr)
char	*hostnm, *ipaddr;
{
	struct	hostent	*hostent;
	struct	in_addr	inet_addr;

	ipaddr[0] = '\0';
	hostent = gethostbyname(hostnm);
	if (hostent == NULL)
	{
		errno = h_errno;
		return(-1);
	}

	memcpy(&inet_addr, hostent->h_addr, hostent->h_length);
	strcpy(ipaddr, (char *)inet_ntoa(inet_addr));
	return(0);
}

/*******************************************************************************
 * NAME	: l_getportbyname()
 * DESC	: get the port number for a service with name and protocol
 * NOTE	:
 * IN	:
 * OUT	:
 * RTN	: Opon success, it returns the port number otherwise -1
 * VER	: 1.00 2009-12 Winway	initial	version.
 * AUTH.:
 ******************************************************************************/
int l_getportbyname(svcnm, proto)
char	*svcnm, *proto;
{
	struct	servent	*servent;

	servent = getservbyname(svcnm, proto);
	if (servent == NULL)
	{
		errno = nw_errno;
		return(-1);
	}

	return( ntohs(servent->s_port) );
}

/*******************************************************************************
 * NAME	: l_hostname()
 * DESC	: get the system name
 * NOTE	:
 * IN	:
 * OUT	:
 * RTN	: Opon success, it returns 0 else -1
 * VER	: 1.00 2009-12 Winway	initial	version.
 * AUTH.:
 ******************************************************************************/
int l_hostname(char *hostnm)
{
	struct	utsname	utsnm;

	if (uname(&utsnm) < 0)
		return(-1);
	strcpy(hostnm, utsnm.nodename);
	return(0);
	//return(utsnm.nodename);
}

/*******************************************************************************
 * NAME	: l_big_endian()
 * DESC	: check whether the system is a big endian
 * NOTE	:
 * IN	:
 * OUT	:
 * RTN	: If big endian is true, then 1 else 0
 * VER	: 1.00 2009-12 Winway	initial	version.
 * AUTH.:
 ******************************************************************************/
int l_big_endian()
{
	unsigned char value[sizeof(int)] = {1, 0, };

	if ((*((int *)(&value[0]))) == 1)
		return(0);	/* little endian */
	else
		return(1);	/* big endian */
}

#if 0
/*******************************************************************************
 * NAME	: l_hostcfg()
 * DESC	: 해당 service 의 host ip-address & port-no.
 * NOTE	:
 * IN	:
 * OUT	:
 * RTN	: If the hostname is equal, then returns 1 else 0
 * VER	: 1.00 2009-12 Winway	initial	version.
 * AUTH.:
 ******************************************************************************/
int l_hostcfg(svcnm, ipaddr, portno)
char	*svcnm, *ipaddr;
int	*portno;
{
	char	filenm[128], read_b[128];
	char	arg1[128], arg2[128], arg3[128];
	FILE	*cfg_F;
	char	find_ok = 0;
	char	*appl_home;

	appl_home = getenv("APPL_HOME");
	if (appl_home == (char *)NULL)
		return(-1);

	sprintf(filenm, "%s/%s/HOST_SERV.cfg", appl_home, USR_CFG_PATH);
	cfg_F = fopen(filenm, "r");
	if (cfg_F == NULL)
		return(-1);

	while ((fgets(read_b, sizeof(read_b), cfg_F)) == read_b)
	{
		sscanf(read_b, "%s %s %s", arg1, arg2, arg3);
		if (arg1 == NULL || arg1[0] == '#' || arg1[0] == '*')
			continue;

		if (strcmp(arg1, svcnm) != 0)
			continue;
		if (strlen(arg2) == 0 || strlen(arg3) == 0)
			continue;

		strcpy(ipaddr, arg2);           /* IP-ADDRESS   */
		*portno = atoi(arg3);           /* PORT NO.     */
		find_ok = 1;
		break;
	}
	fclose(cfg_F);

	if (!find_ok)
	{
		errno = ESRCH;			/* service is not found	*/
		return(-1);
	}

	return(0);
}
#endif

/*******************************************************************************
 * NAME	: l_getip_ledger()
 * DESC	: LEDGER HOST IP-ADDRESS
 * NOTE	:
 * IN	:
 * OUT	:
 * RTN	: Opon success, it returns 0 else -1
 * VER	: 1.00 2009-12 Winway	initial	version.
 * AUTH.:
 ******************************************************************************/
int l_getip_ledger(ipaddr)
char	*ipaddr;
{
	char    hostP[128];
	char    lineB[256], host_ip[80], host_nm[80];
	char    *homedir;
	FILE    *hostF;

	homedir = getenv("MDS_HOME");
	if (homedir == NULL)
		return(-1);

	sprintf(hostP, "%s/hosts.cfg", ETC_DIR);
	hostF = fopen(hostP, "r");
	if (hostF == NULL)
		return(-1);
	while ((fgets(lineB, sizeof(lineB), hostF)) == lineB)
	{
		host_nm[0] = '\0';
		host_ip[0] = '\0';
		sscanf(lineB, "%s %s", host_nm, host_ip);
		if (strcmp(host_nm, "ledger"))
			continue;
		if (!(host_ip[0] >= '0' && host_ip[0] <= '9') || strlen(host_ip) < 7)
			continue;
		strcpy(ipaddr, host_ip);
	}
	fclose(hostF);
	return(0);
}

#if 0
/*******************************************************************************
 * NAME	: l_readcfg()
 * DESC	: Config File에서 Config 내용을 알아낸다
 * NOTE	:
 * IN	: 세션,키,디폴트값,값,길이,파일이름
 * OUT	:
 * RTN	:
 * VER	: 1.00 2009-12 Winway	initial	version.
 * AUTH.:
 ******************************************************************************/
int l_readcfg( sessid, key, def, val, maxlen, cfgfile )
char    *sessid, *key, *def, *val;
int      maxlen;
char    *cfgfile;
{
	FILE	*fp = NULL;
	char	cfgf[128];
	char	buf[1024], tmp[256];
	char	*fpPtr;
	int	findflag, i;
	char	*appl_home = NULL;

	appl_home = getenv("APPL_HOME");
	if (appl_home == (char *)NULL)
		return(-1);


	memset(cfgf, 0x00, sizeof(cfgf));
	sprintf(cfgf, "%s/%s/%s", appl_home, USR_CFG_PATH,cfgfile);

	if ( (fp = fopen(cfgf, "rt")) == NULL )
	{
	strcpy ( val, def );
	return (0);
	}
	findflag = 0x00;
	while ( fgets ( buf, sizeof(buf), fp ) != NULL ) {
	fpPtr = buf;
	memset ( tmp, 0x00, sizeof(tmp) );
	i = 0;

	while ( *fpPtr != '#' && *fpPtr != '\n' && *fpPtr != 0x00 ) {
	    if ( *fpPtr == '[' ) {
		if ( findflag & SESSION_FIND ) {
		    findflag = FIND_FAIL;
		    break;
		}
		fpPtr++;
		while ( *fpPtr != ']' && *fpPtr != '#' &&
			*fpPtr != '\n' && *fpPtr != 0x00 ) {
		    tmp[i++] = *fpPtr;
		    fpPtr++;
		}
		if  ( !strncmp ( sessid, tmp, strlen(sessid) ) )
			findflag = SESSION_FIND;
		break;
	    }
	    if ( findflag & KEY_FIND ) {
		if ( *fpPtr != ' ' && *fpPtr != '\t' ) val[i++] = *fpPtr;
	    }
	    if ( (findflag & SESSION_FIND) && !(findflag & KEY_FIND) )
	    {
		if ( *fpPtr == '=' ) {
		    i = 0;
		    if ( strncmp ( key, tmp, strlen(key) ) == 0 )
			findflag = KEY_FIND;
		    else    break;
		}
		else    tmp[i++] = *fpPtr;
	    }
	    fpPtr++;
	}
	if ( findflag == FIND_FAIL || findflag == KEY_FIND )
	    break;
	}
	fclose ( fp );
	if ( findflag == FIND_FAIL ) {
		strcpy ( val, def);
		return ( 0 );
	}
	if ( maxlen > i )
	{
		val[i] = 0x00;
	}
	else
	{
		val[maxlen] = 0x00;
	}

	return ( 1 );
}
#endif
