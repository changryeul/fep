#ifndef MSG_H
#define	MSG_H	1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/msg.h>

#define		MSG_SIZE		65535
#define		MSG_MAX_CNT		10

typedef struct _msg_
{
	key_t		key;
	int			id;
}   MSG;

typedef struct _msg_data_
{
	long		mtype;
	char		mtext[ MSG_SIZE];
}	MSG_DATA;

#endif /* MSG_H */

/***** Module : msg.c *****/
MSG*        Msg_Create( key_t key);                                         /* message queue create */
MSG*        Msg_Open( key_t key);                                           /* message queue create */
int         Msg_Remove( MSG *msg);                                          /* message queue create */
int         Msg_RemoveByKey( key_t key);                                    /* message queue create */
int         Msg_Close( MSG *msg);                                           /* message queue create */
int         Msg_Send( MSG *msg, char *data, int sz);                        /* message queue create */
int         Msg_Recv( MSG *msg, char *data, int sz);                        /* message queue create */
int         Msg_RecvT( MSG *msg, char *data, int sz, int timeout);          /* message queue create */

