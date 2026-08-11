/** ***************************************************************************
**  @file       msg.c
**  @date       2025/10/27
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  message queue module
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>

#ifndef _OMS_SOURCE_
#include "log.h"
#endif
#include "msg.h"

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     MSG pointer - 성공
**  @retval     NULL        - 실패
**  @brief
**  message queue create
***************************************************************************** */
MSG *Msg_Create( key_t key)
{
	MSG		*msg;

	msg = ( MSG *)malloc( sizeof( MSG));
	if( msg == NULL)
	{
		LogErr( "malloc error. msg size=[%d]", sizeof( MSG));
		goto error;
	}
	memset( msg, 0x00, sizeof( MSG));

	msg->key = key;

	msg->id = msgget( msg->key, IPC_CREAT | IPC_EXCL | 0666);
	if( msg->id < 0)
	{
		LogErr( "msgget error. key=[0x%08x]", msg->key);
		goto error_1;
	}

	LogDel( "message queue created. key=[0x%08x] id=[%d]", key, msg->id);

	return msg;

	error_1:
		free( msg);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     MSG pointer - 성공
**  @retval     NULL        - 실패
**  @brief
**  message queue create
***************************************************************************** */
MSG *Msg_Open( key_t key)
{
	MSG		*msg;

	msg = ( MSG *)malloc( sizeof( MSG));
	if( msg == NULL)
	{
		LogErr( "malloc error. msg size=[%d]", sizeof( MSG));
		goto error;
	}
	memset( msg, 0x00, sizeof( MSG));

	msg->key = key;

	msg->id = msgget( key, 0666);
	if( msg->id < 0)
	{
		LogErr( "msgget error. key=[0x%08x]", msg->key);
		goto error_1;
	}

	LogDel( "shard msgory open. key=[0x%08x] id=[%d]", key, msg->id);
	return msg;

	error_1:
		free( msg);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     MSG pointer - 성공
**  @retval     NULL        - 실패
**  @brief
**  message queue create
***************************************************************************** */
int Msg_Remove( MSG *msg)
{
	int				rtn;

	rtn = msgctl( msg->id, IPC_RMID, NULL);
	if( rtn < 0)
	{
		LogErr( "message queue remove error. id=[%d]", msg->id);
		return -1;
	}

	return rtn;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     MSG pointer - 성공
**  @retval     NULL        - 실패
**  @brief
**  message queue create
***************************************************************************** */
int Msg_RemoveByKey( key_t key)
{
	int				rtn;
	int				id;

	id = msgget( key, 0666);
	if( id < 0)
	{
		LogErr( "msgget error. key=[0x%08x]", key);
		return -1;
	}

	rtn = msgctl( id, IPC_RMID, NULL);
	if( rtn < 0)
	{
		LogErr( "message queue remove error. id=[%d]", id);
		return -1;
	}

	return rtn;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     MSG pointer - 성공
**  @retval     NULL        - 실패
**  @brief
**  message queue create
***************************************************************************** */
int Msg_Close( MSG *msg)
{
	free( msg);
	msg = NULL;
	
	return 1;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     MSG pointer - 성공
**  @retval     NULL        - 실패
**  @brief
**  message queue create
***************************************************************************** */
int Msg_Send( MSG *msg, char *data, int sz)
{
	int				rtn;
	struct msqid_ds	info;
	MSG_DATA		_b, *msg_data = &_b;

	if( sz > MSG_SIZE)
	{
		LogCri( "data size error. sz=[%d] limit=[%d]", sz, MSG_SIZE);
		return -1;
	}

	/* queue 갯수 check */
	rtn = msgctl( msg->id, IPC_STAT, &info);
	if( rtn < 0)
	{
		LogErr( "msgctl");
		return -1;
	}
	LogDbg( "info.msg_qnum = [%d]", info.msg_qnum);

	if( info.msg_qnum >= MSG_MAX_CNT)
	{
		LogMsg( "message queue full. cnt=[%d]", info.msg_qnum);
		return 0;
	}

	msg_data->mtype = 1;
	memcpy( msg_data->mtext, data, sz);

	rtn = msgsnd( msg->id, ( struct msgbuf *)msg_data, sz, 0);
	if( rtn < 0)
	{
		LogErr( "msgsnd error. id=[%d] data=[%d:%.30s]", msg->id, sz, data);
		return -1;
	}
	LogDbg( "send data %d byte(s). data=[%d:%.30s]", sz, data);

	return rtn;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     MSG pointer - 성공
**  @retval     NULL        - 실패
**  @brief
**  message queue create
***************************************************************************** */
int Msg_Recv( MSG *msg, char *data, int sz)
{
	int				rtn;
	MSG_DATA		_b, *msg_data = &_b;

	msg_data->mtype = 0L;

	rtn = msgrcv( msg->id, msg_data, MSG_SIZE, 0, 0);
	if( rtn < 0)
	{
		LogErr( "msgrcv error. id=[%d]", msg->id);
		return -1;
	}
	LogDbg( "recv data %d byte(s). data=[%d:%.30s]", sz, data);
	memcpy( data, msg_data->mtext, rtn);
	data[ rtn] = 0;

	return rtn;
}

/** ***************************************************************************
**  @fu         int CmdFunction( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      int timeout - micro second
**  @return     MSG pointer - 성공
**  @retval     NULL        - 실패
**  @brief
**  message queue create
***************************************************************************** */
int Msg_RecvT( MSG *msg, char *data, int sz, int timeout)
{
	int				rtn;
	MSG_DATA		_b, *msg_data = &_b;
	struct timespec	start_time, cur_time;
	int				wait_time;

	clock_gettime( CLOCK_MONOTONIC, &start_time);
	msg_data->mtype = 0L;

	retry:
	rtn = msgrcv( msg->id, msg_data, MSG_SIZE, 0, IPC_NOWAIT);
	if( rtn < 0)
	{
		if( errno == ENOMSG)
		{
			clock_gettime( CLOCK_MONOTONIC, &cur_time);
			wait_time = ( int)(( cur_time.tv_sec - start_time.tv_sec) * 1000000 + 
				( cur_time.tv_nsec / 1000 - start_time.tv_nsec / 1000));
			if( wait_time >= timeout) return 0;
			usleep( MIN( timeout / 10, wait_time));
			goto retry;
		}
		else
		if( errno == EINTR)	goto retry;

		LogErr( "msgrcv error. id=[%d]", msg->id);
		return -1;
	}
	LogDbg( "recv data %d byte(s). data=[%d:%.30s]", sz, data);
	memcpy( data, msg_data->mtext, rtn);
	data[ rtn] = 0;

	return rtn;
}














