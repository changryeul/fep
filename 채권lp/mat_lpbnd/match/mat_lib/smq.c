/** ***************************************************************************
**  @file       smq.c
**  @date       2023/08/23
**  @author     cdc
**  @version    V2.0.20230823
**  @brif
**  SMQ 라이브러리
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>


#include "log.h"
#include "etc.h"
#include "cfg.h"
#include "smq.h"

extern int		Continue;

/** ***************************************************************************
**  @fu         int Smq_Create()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - smq pointer
**  @retval     실패    - NULL
**  @brief
**  SMQ에 필요한 ipc를 생성
**	shared memory, semaphore 생성
***************************************************************************** */
SMQ* Smq_CreateForce( char *cfg_name)
{
	int		rtn;
	SMQ		*smq;

	smq = malloc( sizeof( SMQ));
	if( smq == NULL)
	{
		LogErr( "malloc error.");
	}
	memset( smq, 0x00, sizeof( SMQ));

	smq->mem = Mem_Create( SMQ_IPC_KEY, sizeof( SMQ_MAP));
	if( smq->mem == NULL)
	{
		LogLib( "Mem_Create error.");
	}
	else 
	{
		smq->map = Mem_GetPtr( smq->mem);
	}

	smq->sem_mem = Sem_Create( SMQ_IPC_KEY);
	if( smq->sem_mem == NULL)
	{
		LogLib( "Sem_Create error.");
		goto error;
	}

	rtn = Smq_Init( smq);
	if( rtn < 0)
	{
		LogCri( "Smq_Init error. smq=[%p]", smq);
	}

	rtn = Smq_Load( smq, cfg_name);
	if( rtn < 0)
	{
		LogCri( "Smq_Load error. smq=[%p] cfg_name=[%s]", smq, cfg_name);
	}

	return smq;

	error:
		return NULL;
}

/** ***************************************************************************
**  @fu         int Smq_Create()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - smq pointer
**  @retval     실패    - NULL
**  @brief
**  SMQ에 필요한 ipc를 생성
**	shared memory, semaphore 생성
***************************************************************************** */
SMQ* Smq_Create( char *cfg_name)
{
	int		rtn;
	SMQ		*smq;

	smq = malloc( sizeof( SMQ));
	if( smq == NULL)
	{
		LogErr( "malloc error.");
		goto error;
	}
	memset( smq, 0x00, sizeof( SMQ));

	smq->mem = Mem_Create( SMQ_IPC_KEY, sizeof( SMQ_MAP));
	if( smq->mem == NULL)
	{
		LogLib( "Mem_Create error.");
		goto error_1;
	}
	smq->map = Mem_GetPtr( smq->mem);
	smq->map->stat.key = SMQ_IPC_KEY;

	smq->sem_mem = Sem_Create( SMQ_IPC_KEY);
	if( smq->sem_mem == NULL)
	{
		LogLib( "Sem_Create error.");
		goto error_2;
	}

	rtn = Smq_Init( smq);
	if( rtn < 0)
	{
		LogCri( "Smq_Init error. smq=[%p]", smq);
		goto error_3;
	}

	rtn = Smq_Load( smq, cfg_name);
	if( rtn < 0)
	{
		LogCri( "Smq_Load error. smq=[%p] cfg_name=[%s]", smq, cfg_name);
		goto error_3;
	}

	return smq;

	error_3:
		Sem_Remove( smq->sem);
	error_2:
		Mem_Remove( smq->mem);
	error_1:
		free( smq);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fu         int Smq_Remove( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  SMQ에서 생성한 ipc를 삭제
**	shared memory, semaphore 삭제
***************************************************************************** */
int Smq_Remove( SMQ *smq)
{
	int			rtn, i;
	SMQ_INDEX	*index;
	char		file_name[ 1024];

	for( i = 0; i < SMQ_MAX_IDX; i++)
	{
		index = &smq->map->index[ i];
		if( strlen( index->name) <= 0) continue;
		sprintf( file_name, "%s/%s.fifo", smq->map->stat.pipe_dir, index->name);
		LogDel( "remove pipe file. name=[%s]", file_name);
		rtn = unlink( file_name);
		if( rtn < 0)
		{
			LogErr( "pipe unlink error. name=[%s]", smq->map->stat.pipe_dir);
		}
		LogDel( "remove semaphore. key=[%d]", index->key);
		rtn = Sem_RemoveByKey( index->key);
		if( rtn < 0)
		{
			LogCri( "semaphore remove error. key=[0x%08x]", index->key);
		}
	}

	rtn = Sem_Remove( smq->sem_mem);
	if( rtn < 0)
	{
		LogLib( "mem semaphore remove error. sem_mem=[%p] key=[0x%08x]", smq->sem_mem, SMQ_IPC_KEY);
	}

	LogDel( "shared memory remove. key=[0x%08x]", smq->mem->key);
	rtn = Mem_Remove( smq->mem);
	if( rtn < 0)
	{
		LogLib( "Mem_Remove error. mem=[%p] key=[0x%08x]", smq->mem, SMQ_IPC_KEY);
		return -1;
	}

	free( smq);

	return 1;
}

/** ***************************************************************************
**  @fu         int Smq_Remove( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  SMQ에서 생성한 ipc를 삭제
**	shared memory, semaphore 삭제
***************************************************************************** */
int Smq_RemoveForce( char *cfg_name)
{
	int			rtn;
	key_t		key;
	char		rec[ 512];
	char		name[ 32], f_name[ 512];
	char		*line;
	int			stat = 0;
	char		*token = ":";
	char		*ptr = rec, *end = ptr;
	CFG			*cfg;


	rtn = Sem_RemoveByKey( SMQ_IPC_KEY);
	if( rtn < 0)
	{
		LogLib( "mem semaphore remove error. key=[0x%08x]", SMQ_IPC_KEY);
	}
	else
	{
		LogDbg( "shared memory remove. key=[0x%08x]", SMQ_IPC_KEY);
	}

	rtn = Mem_RemoveByKey( SMQ_IPC_KEY);
	if( rtn < 0)
	{
		LogLib( "Mem_Remove error. key=[0x%08x]", SMQ_IPC_KEY);
	}
	else
	{
		LogLib( "Mem_Remove. key=[0x%08x]", SMQ_IPC_KEY);
	}

	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", cfg_name);
		return -1;
	}

	line = Cfg_GetFirstNamePtr( cfg, "SharedMemoryQueue");
	while( line != NULL)
	{
		LogDbg( "line=[%s]", line);
		stat = 0;
		memcpy( rec, line, strlen( line) +1);
		end = ptr = rec;
		while( end != NULL)
		{
			end = strpbrk( ptr, token);
			if( end) *end = 0;
	
			switch( stat)
			{
				case 0:			/* name */
					memcpy( name, ptr, strlen( ptr) +1);
					sprintf( f_name, "%s/%s.fifo", getenv( "MAT_DAT") /*SMQ_PIPE_DIR*/, name);
					rtn = unlink( f_name);
					if( rtn < 0)
					{
						LogErr( "unlink error. name=[%s]", f_name);
					}
					break;
				case 1:			/* key */
					key = StoI( ptr);
					rtn = Sem_RemoveByKey( key);
					if( rtn < 0)
					{
						LogCri( "Sem_RemoveByKey error. key=[0x%08x]", key);
					}
					break;
				default:
					break;
			}
			stat++;
			ptr = end +1;
		}
		line = Cfg_GetNextNamePtr( cfg, "SharedMemoryQueue");
	}

	Cfg_Close( cfg);

	return 1;
}

/** ***************************************************************************
**  @fu         int Smq_Open()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - smq pointer
**  @retval     실패    - NULL
**  @brief
**  SMQ Open
***************************************************************************** */
SMQ* Smq_Open( char *name)
{
	SMQ		*smq;
	char	file_name[ 1024];

	SMQ_INDEX	*index;

	smq = malloc( sizeof( SMQ));
	if( smq == NULL)
	{
		LogErr( "malloc error.");
		goto error;
	}
	memset( smq, 0x00, sizeof( SMQ));

	smq->mem = Mem_Open( SMQ_IPC_KEY);
	if( smq->mem == NULL)
	{
		LogLib( "Mem_Open error.");
		goto error_1;
	}
	smq->map = Mem_GetPtr( smq->mem);
	LogDel( "attach shared memory ... ptr=[%p]", smq->map);

	smq->sem_mem = Sem_Open( SMQ_IPC_KEY);
	if( smq->sem_mem == NULL)
	{
		LogLib( "Sem_Open error. key=[0x%08x]", SMQ_IPC_KEY);
		goto error_2;
	}

	if( name == NULL) return smq;
	memcpy( smq->name, name, strlen( name) +1);

	smq->idx = Smq_FindIndex( smq, smq->name);
	if( smq->idx < 0)
	{
		LogMsg( "index name=[%s] not found.", smq->name);
		goto error_3;
	}

	LogDel( "index=[%d]", smq->idx);
	index = &smq->map->index[ smq->idx];

	sprintf( file_name, "%s/%s.fifo", smq->map->stat.pipe_dir, smq->name);

	smq->fd = open( file_name, O_RDWR | O_NONBLOCK);
	if( smq->fd < 0)
	{
		LogCri( "pipe open error. name=[%s]", file_name);
		goto error_3;
	}
	LogDel( "pipe open. name=[%s] fd=[%d]", file_name, smq->fd);

	smq->sem = Sem_Open( index->key);
	if( smq->sem == NULL)
	{
		LogMsg( "semaphore open error. key=[0x%08x]", index->key);
		goto error_4;
	}

	return smq;

	error_4:
		close( smq->fd);
	error_3:
		Sem_Close( smq->sem_mem);
	error_2:
		Mem_Close( smq->mem);
	error_1:
		free( smq);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fu         int Smq_Close( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  SMQ Close
***************************************************************************** */
int Smq_Close( SMQ *smq)
{
	int		rtn;

	rtn = Sem_Close( smq->sem_mem);
	if( rtn < 0)
	{
		LogLib( "Sem_Close error. sem_mem=[%p] key=[0x%08x]", smq->sem_mem, SMQ_IPC_KEY);
		return -1;
	}

	rtn = Mem_Close( smq->mem);
	if( rtn < 0)
	{
		LogLib( "Mem_Close error. mem=[%p] key=[0x%08x]", smq->mem, SMQ_IPC_KEY);
		return -1;
	}

	if( smq->fd >= 0) close( smq->fd);
	if( smq->sem != NULL)	Sem_Close( smq->sem);

	smq->fd = -1;
	smq->sem = NULL;
	smq->mem = NULL;

	free( smq);

	return 1;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  index initial
***************************************************************************** */
int Smq_Init( SMQ *smq)
{
	int			i;
	SMQ_INDEX	*index;
	SMQ_RECORD	*rec;
	struct stat	statbuf;

	time( &smq->map->stat.ctime);
	memcpy( &smq->map->stat.pipe_dir, getenv( "MAT_DAT") /*SMQ_PIPE_DIR*/, strlen( getenv( "MAT_DAT") /*SMQ_PIPE_DIR*/));
	smq->map->stat.max_rec = SMQ_MAX_REC;
	smq->map->stat.key = SMQ_IPC_KEY;

	/* index initial */
	for( i = 0; i < SMQ_MAX_IDX; i++)
	{
		index = &smq->map->index[ i];
		memset( index->name, 0x00, sizeof( index->name));
		index->start 	= -1;		/* start position */
		index->end		= -1;		/* end position */
		index->cnt		= 0;		/* data 갯수 */
	}

	/* record initial */
	for( i = 0; i < SMQ_MAX_REC; i++)
	{
		rec = &smq->map->rec[ i];
		rec->pos = i;
	}

	/* semaphore create */
	fstat( smq->fd, &statbuf);
	smq->sem = Sem_Open( SMQ_IPC_KEY);
	if( smq->sem == NULL)
	{
		LogMsg( "semaphore open error. create ... ");
		smq->sem = Sem_Create( index->key);
		if( smq->sem == NULL)
		{
			LogMsg( "semaphore create error.");
			return -1;
		}
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  index initial
***************************************************************************** */
int Smq_Load( SMQ *smq, char *cfg_name)
{
	int			rtn;
	CFG			*cfg;
	char		*ptr;
	int			pos = 0;
	char		f_name[ 1024];
	SMQ_INDEX	*index;

	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", cfg_name);
		return -1;
	}

	ptr = Cfg_GetFirstNamePtr( cfg, "SharedMemoryQueue");
	while( ptr != NULL)
	{
		LogDbg( "line=[%s]", ptr);
		rtn = Smq_GetIndex( smq, ptr, pos);
		if( rtn < 0) goto error;
		pos++;
		if( pos >= SMQ_MAX_IDX) break;
		ptr = Cfg_GetNextNamePtr( cfg, "SharedMemoryQueue");
	}

	Cfg_Close( cfg);

	return 1;
	
	error:
		while( pos)
		{
			index = &smq->map->index[ pos];
			LogMsg( "remove semaphore  ... key=[0x%08x]", index->key);
			Sem_RemoveByKey( index->key);
			LogMsg( "remove named pipe ... f_name=[%s]", f_name);
			sprintf( f_name, "%s/%s.fifo", getenv( "MAT_DAT") /*smq->map->stat.pipe_dir*/, index->name);
			unlink( f_name);
			pos--;
		}
		return -1;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  index initial
***************************************************************************** */
int Smq_GetIndex( SMQ *smq, char *line, int pos)
{
	int			rtn;
	int			stat = 0;
	char		rec[ 512], f_name[ 1024];
	char		*token = ":";
	char		*ptr = rec, *end = ptr;
	SEM			*sem;
	SMQ_INDEX	*index;

	memcpy( rec, line, strlen( line) +1);
	index = &smq->map->index[ pos];

	while( end != NULL)
	{
		end = strpbrk( ptr, token);
		if( end) *end = 0;

		switch( stat)
		{
			case 0:			/* name */
				memcpy( index->name, ptr, strlen( ptr) +1);
				sprintf( f_name, "%s/%s.fifo", smq->map->stat.pipe_dir, index->name);
				rtn = mkfifo( f_name, 0666);
				if( rtn < 0)
				{
					LogErr( "mkfifo error. name=[%s]", f_name);
					goto error;
				}
				LogMsg( "create named pipe [%s].", f_name);
				break;
			case 1:			/* key */
				index->key = StoI( ptr);
				sem = Sem_Create( index->key);
				if( sem == NULL)
				{
					LogCri( "Sem_Create error. key=[0x%08x]", index->key);
					goto error_1;
				}
				break;
			case 2:			/* max data */
				index->mcnt = atoi( ptr);
				break;
			case 3:			/* comment */
				memcpy( index->comment, ptr, strlen( ptr) +1);
				break;
			default:
				break;
		}
		stat++;
		ptr = end +1;
	}

	return stat;

	error_1:
		unlink( f_name);
		LogMsg( "remove named pipe [%s].", f_name);
	error:
		return -1;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  find index - name=NULL 이면 빈 index return
***************************************************************************** */
int Smq_FindIndex( SMQ *smq, char *name)
{
	int			i;
	SMQ_INDEX	*index;

	if( name == NULL)
	{
		for( i = 0; i < SMQ_MAX_IDX; i++)
		{
			index = &smq->map->index[ i];
			if( strlen( index->name) > 0) continue;
			return i;
		}
		return -1;
	}

	for( i = 0; i < SMQ_MAX_IDX; i++)
	{
		index = &smq->map->index[ i];
		if( strlen( index->name) <= 0) continue;
		if( !memcmp( index->name, name, strlen( name) +1)) return i;
	}
	return -1;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq 	- 매칭 struct pointer
**  @param      int timeout - timeout(micro second)
**  @return     성공    - record position
**  @return     timeout - SMQ_TIMEOUT(-9999)
**  @retval     실패    - -1
**  @brief
**  pipe로 부터 체결 record position을 수신
***************************************************************************** */
int Smq_WritePipe( SMQ *smq)
{
	int				rtn;

	retry:
	LogDel( "write data 1 byte(s). fd=[%d]", smq->fd);
	rtn = write( smq->fd, "1", 1);
	if( rtn < 1)
	{
		if( rtn == 0)
		{
			LogMsg( "pipe full. fd=[%d] rtn=[%d]", smq->fd, rtn);
			return 0;
		}

		switch( errno)
		{
			case EAGAIN:
			case EINTR:
				goto retry;
			default:
				LogErr( "pipe write error. fd=[%d] rtn=[%d]", smq->fd, rtn);
				return -1;
		}
	}

	return rtn;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq 	- 매칭 struct pointer
**  @param      int timeout - timeout(micro second)
**  @return     성공    - record position
**  @return     timeout - SMQ_TIMEOUT(-9999)
**  @retval     실패    - -1
**  @brief
**  pipe로 부터 체결 record position을 수신
***************************************************************************** */
int Smq_ReadPipe( SMQ *smq, int timeout)
{
	int				rtn;
	fd_set			rfds;
	char			rec[ 512];
	struct timeval	tv, *tp = &tv;
	SMQ_INDEX		*index;

	rtn = read( smq->fd, rec, 1);
	if( rtn >= 1) return 1;

	index = &smq->map->index[ smq->idx];

	if( index->end >= 0) 
	{
		return 1;
	}
	else
	{
		index->cnt = 0;
	}

	while( Continue)
	{
		tp->tv_sec  = timeout / 1000000;
		tp->tv_usec = timeout % 1000000;

		FD_ZERO( &rfds);
		FD_SET( smq->fd, &rfds);

		LogDel( "select wait ... fd=[%d] timeout=[%d.%d]", smq->fd, tp->tv_sec, tp->tv_usec);
		rtn = select( smq->fd +1, &rfds, NULL, NULL, tp);
		if( rtn < 0)
		{
			LogErr( "pipe select error. fd=[%d]", smq->fd);
			switch( errno)
			{
				case 4:	/* Interrupted system call */
					continue;
				default:
					break;
			}
			return -1;
		}
		else if( rtn == 0)
		{
			LogDel( "recv timeout. timeout=[%d.%d]", timeout / 1000000, timeout % 1000000);
			return SMQ_TIMEOUT;
		}
		LogDel( "get event ...");

		if( FD_ISSET( smq->fd, &rfds))
		{
			rtn = read( smq->fd, rec, 1);
			if( rtn < 1)
			{
				switch( errno)
				{
					case EAGAIN:
					case EINTR:
						continue;
					default:
						break;
				}

				LogErr( "pipe read error. fd=[%d] rtn=[%d]", smq->fd, rtn);
				return -1;
			}
			if( index->cnt == 0) continue;

			return 1;
		}
	}

	return -1;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  data write - lock 없음
**	무조건 첫번째에 주문 레코드 삽입
***************************************************************************** */
int Smq_Write( SMQ *smq, char *rec, int sz)
{
	int			pos, npos;
	SMQ_HEAD	*head, *nhead;
	SMQ_RECORD	*rptr, *nptr;
	SMQ_INDEX	*index;

	/* check max data */
	index = &smq->map->index[ smq->idx];
	if( index->mcnt != 0)
	{
		if( index->cnt >=  index->mcnt) return 0;
	}

	Smq_MemLock( smq);

	pos = Smq_GetEmptyRecordPos( smq);
	if( pos < 0)
	{
		LogMsg( "Smq_GetEmptyRecordPos error.");
		goto no_empty_rec;
	}

	rptr = &smq->map->rec[ pos];
	rptr->pos = pos;
	head = &rptr->head;
	gettimeofday( &head->w_time, NULL);
	memcpy( &rptr->rec, rec, sz +1);
	head->sz = sz;


	/* 최초 record */
	if( index->start == -1)
	{
		index->start = pos;
		index->end   = pos;
		head->prev = -1;
		head->next = -1;
		index->cnt++;
		index->dcnt++;
		head->gubun = 1;
		smq->map->stat.rec_cnt++;

		goto success;
	}

	/* start record get */
	npos = index->start;
	nptr = &smq->map->rec[ npos];

	/* insert current record at first */
	nhead = &nptr->head;
	nhead->prev = pos;
	index->start = pos;
	head->prev = -1;
	head->next = npos;
	head->gubun = 1;
	index->cnt++;
	index->dcnt++;
	smq->map->stat.rec_cnt++;


	success:
	index->spid = getpid();
	time( &index->stime);
	Smq_MemUnlock( smq);

	Smq_WritePipe( smq);

	return pos;

	no_empty_rec:
		Smq_MemUnlock( smq);
		return SMQ_TIMEOUT;
#if 0
	error:
		Smq_MemUnlock( smq);
		return -1;
#endif
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  queue read - lock 없음
***************************************************************************** */
int Smq_Read( SMQ *smq, char *rec, int sz)
{
	int			pos, ppos, npos;	/* curr/prev/next */
	SMQ_INDEX	*index;
	SMQ_HEAD	*head;
	SMQ_RECORD	*ptr, *pptr;		/* curr/prev/next */

	Smq_MemLock( smq);

	/* get last record */
	index = &smq->map->index[ smq->idx];
	pos = index->end;
	if( pos < 0) 
	{
		/* data 없음 */
		goto no_data;
	}
	ptr = &smq->map->rec[ pos];
	head = &ptr->head;
	gettimeofday( &head->r_time, NULL);
	ppos = head->prev;
	npos = head->next;

	if( ppos >= 0)		/* 처음이 아니면 */
	{
		pptr = &smq->map->rec[ head->prev];
		pptr->head.next = npos;
	}
	else
	{
		/* 처음 record 이면 index update */
		index->start = ppos;
	}

	/* 마지막 record 이므로 index update */
	index->end = ppos;

	/* record update - gubun을 0으로 setting하여 빈레코드로 표시 */
	ptr->head.gubun = 0;

	/* index에 record 갯수를 1 감소 */
	index->cnt--;
	smq->map->stat.rec_cnt--;

	memcpy( rec, ptr->rec, head->sz);
	rec[ head->sz] = 0;

	index->rpid = getpid();
	time( &index->rtime);

	Smq_MemUnlock( smq);
	return head->sz;

	no_data:
		Smq_MemUnlock( smq);
		return 0;

#if 0
	error:
		Smq_MemUnlock( smq);
		return -1;
#endif
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  data send - lock/unlock
**	무조건 첫번째에 주문 레코드 삽입
***************************************************************************** */
int Smq_Send( SMQ *smq, char *rec, int sz)
{
	int			rtn;

	Smq_Lock( smq);
	rtn = Smq_Write( smq, rec, sz);
	Smq_Unlock( smq);

	return rtn;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  data receive - lock/unlock
***************************************************************************** */
int Smq_Recv( SMQ *smq, char *rec, int sz, int timeout)
{
	int			rtn;

	rtn = Smq_ReadPipe( smq, timeout);
	if( rtn < 0)
	{
		if( rtn == SMQ_TIMEOUT)
		{
			LogMsg( "Smq_ReadPipe timeout.");
			return SMQ_TIMEOUT;
		}
		LogCri( "Smq_ReadPipe error.");
		return rtn;
	}

	Smq_Lock( smq);
	rtn = Smq_Read( smq, rec, sz);
	Smq_Unlock( smq);

	return rtn;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  pipe event check
***************************************************************************** */
int Smq_Check( SMQ *smq, int timeout)
{
	int			rtn;

	rtn = Smq_ReadPipe( smq, timeout);
	if( rtn < 0)
	{
		if( rtn == SMQ_TIMEOUT)
		{
			LogMsg( "Smq_ReadPipe timeout.");
			return 0;
		}
		LogCri( "Smq_ReadPipe error.");
		return rtn;
	}

	return rtn;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - record position
**  @retval     실패    - -1
**  @brief
**  Lock 해제 및 insert/delete
***************************************************************************** */
int Smq_Commit( SMQ *smq)
{
	int		rtn;

	switch( smq->flag)
	{
		case 0:		/* commit */
			LogMsg( "이미 처리되었습니다.");
			return 0;
		case 1:
			rtn = Smq_InsertRecord( smq);
			Smq_Unlock( smq);
			break;
		case 2:
			rtn = Smq_DeleteRecord( smq);
			Smq_Unlock( smq);
			break;
		default:
			return -1;
	}
	smq->flag = 0;

	return rtn;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - record position
**  @retval     실패    - -1
**  @brief
**  Lock 해제
***************************************************************************** */
int Smq_Rollback( SMQ *smq)
{
	switch( smq->flag)
	{
		case 0:		/* commit */
			LogMsg( "이미 처리되었습니다.");
			return 0;
		case 1:
			Smq_Unlock( smq);
			break;
		case 2:
			Smq_Unlock( smq);
			break;
		default:
			return -1;
	}
	smq->flag = 0;

	return 1;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - record position
**  @retval     실패    - -1
**  @brief
**  빈 record get - Lock 유지, Smq_PutRecord/Smq_Commit - Lock 해제
***************************************************************************** */
int Smq_SetRecord( SMQ *smq, char *rec, int sz)
{
	int			pos;

	if( rec == NULL)
	{
		LogMsg( "argument error. rec=[%p]", rec);
		return -1;
	}

	if( sz <= 0 || sz >= SMQ_REC_SZ)
	{
		LogMsg( "sz error. sz=[%d] max=[%d]", sz, SMQ_REC_SZ);
		return -1;
	}

	Smq_Lock( smq);
	Smq_MemLock( smq);

	pos = Smq_GetEmptyRecordPos( smq);
	if( pos < 0)
	{
		LogMsg( "Smq_GetEmptyRecordPos error.");
		Smq_MemUnlock( smq);
		Smq_Unlock( smq);
		return -1;
	}

	smq->pos = pos;
	smq->ptr = rec;
	smq->sz = sz;
	smq->flag = 1;
	Smq_MemUnlock( smq);

	return pos;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq	- 매칭 struct pointer
**  @return     성공    	- record position
**  @retval     실패    	- -1
**  @brief
**  Lock 해제 및 insert
***************************************************************************** */
int	Smq_PutRecord( SMQ *smq)
{
	int			rtn;
	int			pos;
	SMQ_HEAD	*head;
	SMQ_RECORD	*ptr;

	pos  = smq->pos;
	ptr  = &smq->map->rec[ pos];
	head = &ptr->head;

	head->sz = smq->sz;
	memcpy( &ptr->rec, smq->ptr, smq->sz);

	rtn = Smq_InsertRecord( smq);
	Smq_Unlock( smq);
	smq->flag = 0;

	return rtn;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - record pointer
**  @retval     실패    - NULL
**  @brief
**  record pointer get - Lock 유지, Smq_PutPtr에서 Lock 해제
***************************************************************************** */
char *Smq_SetPtr( SMQ *smq)
{
	int			pos;

	Smq_Lock( smq);
	Smq_MemLock( smq);
	pos = Smq_GetEmptyRecordPos( smq);
	if( pos < 0)
	{
		LogMsg( "Smq_GetEmptyRecordPos error.");
		Smq_MemUnlock( smq);
		Smq_Unlock( smq);
		return NULL;
	}

	smq->pos = pos;
	smq->ptr = NULL;
	smq->sz = 0;
	smq->flag = 1;
	Smq_MemUnlock( smq);

	return smq->map->rec[ pos].rec;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq	- 매칭 struct pointer
**  @return     성공    	- record position
**  @retval     실패    	- -1
**  @brief
**  Lock 해제 및 insert
***************************************************************************** */
int	Smq_PutPtr( SMQ *smq, char *rec, int sz)
{
	int			rtn;
	int			pos;
	SMQ_HEAD	*head;
	SMQ_RECORD	*ptr;

	pos  = smq->pos;
	ptr  = &smq->map->rec[ pos];
	head = &ptr->head;

	if( ptr->rec != rec)
	{
		LogCri( "pointer not matched. ptr=[%p] rec=[%p]", ptr, rec);
		return -1;
	}
	if( sz >= SMQ_REC_SZ)
	{
		LogCri( "data size too big. sz=[%d] max=[%d]", sz, SMQ_REC_SZ);
		return -1;
	}
	smq->sz = sz;

	head->sz = smq->sz;
	memcpy( &ptr->rec, smq->ptr, smq->sz);

	rtn = Smq_InsertRecord( smq);
	Smq_Unlock( smq);
	smq->flag = 0;

	return rtn;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq	- 매칭 struct pointer
**  @return     성공    	- record position
**  @retval     실패    	- -1
**  @brief
**  internal - smq->pos 내용을 index에 insert
***************************************************************************** */
int	Smq_InsertRecord( SMQ *smq)
{
	int			pos, npos;
	SMQ_HEAD	*head, *nhead;
	SMQ_RECORD	*ptr, *nptr;
	SMQ_INDEX	*index;

	pos = smq->pos;
	ptr = &smq->map->rec[ pos];

	ptr->pos = pos;
	head = &ptr->head;
	head->sz = smq->sz;
	index = &smq->map->index[ smq->idx];
	gettimeofday( &head->w_time, NULL);

	/* 최초 record */
	if( index->start == -1)
	{
		index->start = pos;
		index->end   = pos;
		head->prev = -1;
		head->next = -1;
		index->cnt++;
		head->gubun = 1;
		Smq_MemLock( smq);
		smq->map->stat.rec_cnt++;
		Smq_MemUnlock( smq);

		goto success;
	}

	/* start record get */
	npos = index->start;
	nptr = &smq->map->rec[ npos];

	/* insert current record at first */
	nhead = &nptr->head;
	nhead->prev = pos;
	index->start = pos;
	head->prev = -1;
	head->next = npos;
	head->gubun = 1;
	index->cnt++;
	Smq_MemLock( smq);
	smq->map->stat.rec_cnt++;
	Smq_MemUnlock( smq);

	success:
	Smq_WritePipe( smq);
	index->spid = getpid();
	time( &index->stime);

	return pos;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  current queue get - Lock 유지, Smq_DeleteRecord에서 Lock 해제
***************************************************************************** */
int Smq_GetRecord( SMQ *smq, char *rec, int sz, int timeout)
{
	int			rtn;
	int			pos, data_sz;
	SMQ_INDEX	*index;
	SMQ_HEAD	*head;
	SMQ_RECORD	*ptr;


	rtn = Smq_ReadPipe( smq, timeout);
	if( rtn < 0)
	{
		if( rtn == SMQ_TIMEOUT)
		{
			LogDel( "Smq_ReadPipe timeout.");
			return SMQ_TIMEOUT;
		}
		LogCri( "Smq_ReadPipe error.");
		return rtn;
	}

	Smq_Lock( smq);
	/* get last record */
	index = &smq->map->index[ smq->idx];
	pos = index->end;
	if( pos < 0)
	{
		Smq_Unlock( smq);
		return SMQ_TIMEOUT;
	}
	ptr = &smq->map->rec[ pos];
	head = &ptr->head;
	gettimeofday( &head->r_time, NULL);

	LogDel( "pos=[%d] head->sz=[%d]", pos, head->sz);
	if( head->sz >= sz)
	{
		LogWar( "buffer size error. data_sz=[%d] buffer_sz=[%d]", head->sz, sz);
		data_sz = sz -1;
	}
	else	data_sz = head->sz;

	memcpy( rec, ptr->rec, data_sz);
	rec[ data_sz] = 0;

	smq->pos = pos;
	smq->ptr = rec;
	smq->sz = sz;
	smq->flag = 2;

	LogDel( "return data_sz=[%d]", data_sz);
	return data_sz;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  index에서 record delete, Lock 해제
***************************************************************************** */
int Smq_DelRecord( SMQ *smq)
{
	int		rtn;

	rtn = Smq_DeleteRecord( smq);
	Smq_Unlock( smq);
	return rtn;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  internal - index에서 record delete
***************************************************************************** */
int Smq_DeleteRecord( SMQ *smq)
{
	int			pos, ppos, npos;	/* curr/prev/next */
	SMQ_INDEX	*index;
	SMQ_HEAD	*head;
	SMQ_RECORD	*ptr, *pptr;		/* curr/prev/next */


	pos   = smq->pos;
	ptr   = &smq->map->rec[ pos];
	head  = &ptr->head;
	index = &smq->map->index[ smq->idx];

	ppos = head->prev;
	npos = head->next;

	if( ppos >= 0)		/* 처음이 아니면 */
	{
		pptr = &smq->map->rec[ head->prev];
		pptr->head.next = npos;
	}
	else
	{
		/* 처음 record 이면 index update */
		index->start = ppos;
	}

	/* 마지막 record 이므로 index update */
	index->end = ppos;

	/* record update - gubun을 0으로 setting하여 빈레코드로 표시 */
	ptr->head.gubun = 0;

	/* index에 record 갯수를 1 감소 */
	index->cnt--;
	Smq_MemLock( smq);
	smq->map->stat.rec_cnt--;
	Smq_MemUnlock( smq);
	index->rpid = getpid();
	time( &index->rtime);

	return head->sz;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - record position
**  @retval     실패    - -1
**  @brief
**  빈 record 찾기
***************************************************************************** */
int Smq_GetEmptyRecordPos( SMQ *smq)
{
	int		pos;
	int		cnt = 0;

	pos = smq->map->stat.wpos;

	while( Continue)
	{
		if( smq->map->rec[ pos].head.gubun == 0) 
		{
			smq->map->stat.wpos = pos +1;
			if( smq->map->stat.wpos >= SMQ_MAX_REC)	smq->map->stat.wpos = 0;
			return pos;
		}
		LogDbg( "search empty record pos=[%d] gubun=[%d]", pos, smq->map->rec[ pos].head.gubun);
		pos++;
		if( pos >= SMQ_MAX_REC)	pos = 0;
		cnt++;
		if( cnt >= SMQ_MAX_REC)
		{
			LogMsg( "빈 record가 없습니다.");
			break;
		}
	}

	return SMQ_TIMEOUT;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - record position
**  @retval     실패    - -1
**  @brief
**  position을 입력하여 record 찾기
***************************************************************************** */
SMQ_RECORD *Smq_GetRecordByPos( SMQ *smq, int pos)
{
	return &smq->map->rec[ pos];
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  semaphore lock 수행 - 공유메모리
***************************************************************************** */
int Smq_MemLock( SMQ *smq)
{
	int		rtn;

#if 0
	rtn = Sem_Lock( smq->sem_mem);
#else
	retry:
	rtn = Sem_LockT( smq->sem_mem, 1000000);
	if( rtn < 0)
	{
		LogWar( "Sem_LockT fail. id=[%d]", smq->sem_mem->id);
		goto retry;
	}
#endif

	return rtn;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  semaphore lock 해제 - 공유메모리
***************************************************************************** */
int Smq_MemUnlock( SMQ *smq)
{
	int		rtn;

	rtn = Sem_Unlock( smq->sem_mem);

	return rtn;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  semaphore lock 수행 - queue
***************************************************************************** */
int Smq_Lock( SMQ *smq)
{
	int		rtn;

	retry:
	rtn = Sem_LockT( smq->sem, 1000000);
	if( rtn < 0)
	{
		LogWar( "Sem_LockT fail ... id=[%d]", smq->sem->id);
		goto retry;
	}

	return rtn;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  semaphore lock 해제 - queue
***************************************************************************** */
int Smq_Unlock( SMQ *smq)
{
	int		rtn;

	rtn = Sem_Unlock( smq->sem);

	return rtn;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  stat 출력
***************************************************************************** */
int Smq_StatisReset( SMQ *smq)
{
	int			i;
	SMQ_INDEX	*index;

	for( i = 0; i < SMQ_MAX_IDX; i++)
	{
		index = &smq->map->index[ i];

		if( strlen( index->name) <= 0) continue;

		index->dcnt = 0;
	}
	return 1;
}

/** ***************************************************************************
**  @fu         int Smq_( SMQ *smq)
**  @param      SMQ *smq - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  stat 출력
***************************************************************************** */
int Smq_Stat( SMQ *smq)
{
	int			i, sz;
	int			cols = 100;

	SMQ_INDEX	*index;

	printf( "smq                  = [%p]\n", smq);
	printf( "mem                  = [%p]\n", smq->mem);
	printf( "mem->id              = [%d]\n", smq->mem->id);
	printf( "sem                  = [%p]\n", smq->sem);
	if( smq->sem != NULL)
	{
	printf( "sem->id              = [%d]\n", smq->sem->id);
	}
	printf( "fd(fifo)             = [%d]\n", smq->fd);
	printf( "base(smq->map)       = [%p]\n", ( char *)smq->map);
	printf( "---------------------------------------------\n");
	printf( "ctime                = [%s]\n", TtoS( smq->map->stat.ctime));
	printf( "rec_cnt              = [%d]\n", smq->map->stat.rec_cnt);
	printf( "wpos                 = [%d]\n", smq->map->stat.wpos);
	printf( "key                  = [0x%08x]\n", smq->map->stat.key);
	printf( "pipe_dir             = [%s]\n", smq->map->stat.pipe_dir );
	printf( "---------------------------------------------\n");

	printf( "name         ");
	printf( "start ");
	printf( "  end ");
	printf( "count ");
	printf( "send_pid ");
	printf( "%-20s", "send_time");
	printf( "recv_pid ");
	printf( "%-20s", "recv_time");
	printf( "key      ");
	printf( "\n");
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	for( i = 0; i < SMQ_MAX_IDX; i++)
	{
		index = &smq->map->index[i];
		sz = strlen( index->name);
		if( sz <= 0) continue;
		printf( "%-12s ", index->name);
		printf( "%5d ", index->start);
		printf( "%5d ", index->end);
		printf( "%5d ", index->cnt);

		printf( "%8d ", index->spid);
		printf( "%s ", TtoS( index->stime));
		printf( "%8d ", index->rpid);
		printf( "%s ", TtoS( index->rtime));
		printf( "0x%08x ", index->key);
		printf( "\n");
	}
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	return 1;
}


