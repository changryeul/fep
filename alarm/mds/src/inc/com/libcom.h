/******************************************************************************
 * Components  : libcom.h
 * Description : lib com에서 사용하는 함수
 ******************************************************************************/
#ifndef __LIBCOM_H__
#define __LIBCOM_H__

/*******************************************************************************
 * ipcsub.c
 ******************************************************************************/
int	l_token(char *ipc_name,int ipc_type);
int	l_msgget(char * queue_name,int size,int mode) ;
char	*l_shmat(char *shm_name);
int	l_putmsg(int qid, int mtype, char *trans_b, int trans_l, int wait_mode);
int	l_putmsgx(char *queue_name, int mtype, char *trans_b, int trans_l, int wait_mode);
int	l_getmsg(int qid, int *mtype, char *trans_b, int bsize, int timeout);
int	l_getmsgx(char *queue_name, int *mtype, char *trans_b, int bsize, int timeout);
//static	int l_name2id(char *queue_name);
void	_alarm_handler(int signo);

/*******************************************************************************
 * netconf.c
 ******************************************************************************/
int	l_getip(char *);
int	l_ishost(char *hostnm);
int	l_getipbyname(char *hostnm,char *ipaddr);
int	l_getportbyname(char *svcnm,char *proto);
int l_hostname(char *hostnm);
int	l_big_endian();
int	l_hostcfg(char *svcnm, char *ipaddr,int * portno);
int	l_getip_ledger(char *ipaddr);
int	l_readcfg(char *sessid,char *key,char *def,char *val, int maxlen, char *cfgfile );

#endif
