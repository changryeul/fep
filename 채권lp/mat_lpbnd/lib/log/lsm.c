/** ***************************************************************************
**  @file       mem.c
**  @date       2023/08/23
**  @author     cdc
**  @version    V2.0.20230823
**  @brif
**  Log shared memory manager
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>


#include "etc.h"
#include "cfg.h"
#include "log.h"

extern int		Continue;

char			LsmBuf[ LOG_REC_SZ];
char			LogTailStr[ 4] = "\n";
LOG				*LsmPtr = NULL;



/** ***************************************************************************
**  @fu         int Lsm_Create()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - mem pointer
**  @retval     실패    - NULL
**  @brief
**  매칭엔진에 필요한 ipc를 생성
**	shared memory, semaphore 생성
***************************************************************************** */
LSM* Lsm_CreateForce()
{
	int		rtn;
	LSM		*mem;
	char	pipe_name[ 512];

	mem = malloc( sizeof( LSM));
	if( mem == NULL)
	{
		LsmErr( "malloc error.");
	}
	memset( mem, 0x00, sizeof( LSM));

	mem->mem = Mem_Create( LOG_IPC_KEY, sizeof( LOG_MAP));
	if( mem->mem == NULL)
	{
		LsmCri( "Mem_Create error.");
	}
	else
	{
		mem->map = Mem_GetPtr( mem->mem);
	}

	mem->sem = Sem_Create( LOG_IPC_KEY);
	if( mem->sem == NULL)
	{
		LsmCri( "Sem_Create error.");
		goto error;
	}

	sprintf( pipe_name, "%s/%s", getenv( "MAT_DAT"), "log.fifo");
	rtn = mkfifo( pipe_name, 0666);
	if( rtn < 0)
	{
		LsmErr( "pipe create error. name=[%s]", pipe_name);
	}

	rtn = Lsm_Init( mem);
	if( rtn < 0)
	{
		LsmCri( "Lsm_Init error. mem=[%p]", mem);
	}

	return mem;

	error:
		return NULL;
}

/** ***************************************************************************
**  @fu         int Lsm_Create()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - mem pointer
**  @retval     실패    - NULL
**  @brief
**  매칭엔진에 필요한 ipc를 생성
**	shared memory, semaphore 생성
***************************************************************************** */
LSM* Lsm_Create()
{
	int		rtn;
	LSM	*mem;

	mem = malloc( sizeof( LSM));
	if( mem == NULL)
	{
		LsmErr( "malloc error.");
		goto error;
	}
	memset( mem, 0x00, sizeof( LSM));

	mem->mem = Mem_Create( LOG_IPC_KEY, sizeof( LOG_MAP));
	if( mem->mem == NULL)
	{
		LsmCri( "Mem_Create error.");
		goto error_1;
	}
	mem->map = Mem_GetPtr( mem->mem);
	mem->map->stat.key = LOG_IPC_KEY;

	mem->sem = Sem_Create( LOG_IPC_KEY);
	if( mem->sem == NULL)
	{
		LsmCri( "Sem_Create error.");
		goto error_2;
	}

	rtn = mkfifo( LOG_PIPE_NAME, 0666);
	if( rtn < 0)
	{
		LsmErr( "pipe create error. name=[%s]", LOG_PIPE_NAME);
	}

	rtn = Lsm_Init( mem);
	if( rtn < 0)
	{
		LsmCri( "Lsm_Init error. mem=[%p]", mem);
		goto error_3;
	}

	return mem;

	error_3:
		Sem_Remove( mem->sem);
	error_2:
		Mem_Remove( mem->mem);
	error_1:
		free( mem);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fu         int Lsm_Remove( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  매칭엔진에서 생성한 ipc를 삭제
**	shared memory, semaphore 삭제
***************************************************************************** */
int Lsm_Remove( LSM *mem)
{
	int			rtn;

	rtn = Sem_Remove( mem->sem);
	if( rtn < 0)
	{
		LsmCri( "mem semaphore remove error. sem=[%p] key=[0x%08x]", mem->sem, LOG_IPC_KEY);
	}

	LsmMsg( "shared memory remove. key=[0x%08x]", mem->mem->key);
	rtn = Mem_Remove( mem->mem);
	if( rtn < 0)
	{
		LsmCri( "Mem_Remove error. mem=[%p] key=[0x%08x]", mem->mem, LOG_IPC_KEY);
		return -1;
	}

	free( mem);

	return 1;
}

/** ***************************************************************************
**  @fu         int Lsm_Remove( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  매칭엔진에서 생성한 ipc를 삭제
**	shared memory, semaphore 삭제
***************************************************************************** */
int Lsm_RemoveForce( char *cfg_name)
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


	rtn = Sem_RemoveByKey( LOG_IPC_KEY);
	if( rtn < 0)
	{
		LsmCri( "mem semaphore remove error. key=[0x%08x]", LOG_IPC_KEY);
	}
	else
	{
		LsmMsg( "shared memory remove. key=[0x%08x]", LOG_IPC_KEY);
	}

	rtn = Mem_RemoveByKey( LOG_IPC_KEY);
	if( rtn < 0)
	{
		LsmCri( "Mem_Remove error. key=[0x%08x]", LOG_IPC_KEY);
	}
	else
	{
		LsmMsg( "Mem_Remove. key=[0x%08x]", LOG_IPC_KEY);
	}

	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LsmCri( "Cfg_Open error. name=[%s]", cfg_name);
		return -1;
	}

	line = Cfg_GetFirstNamePtr( cfg, "SharedMemoryQueue");
	while( line != NULL)
	{
		LsmDel( "line=[%s]", line);
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
					rtn = unlink( LOG_PIPE_NAME);
					if( rtn < 0)
					{
						LsmErr( "unlink error. name=[%s]", f_name);
					}
					break;
				case 1:			/* key */
					key = StoI( ptr);
					rtn = Sem_RemoveByKey( key);
					if( rtn < 0)
					{
						LsmCri( "Sem_RemoveByKey error. key=[0x%08x]", key);
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
**  @fu         int Lsm_Open()
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공    - mem pointer
**  @retval     실패    - NULL
**  @brief
**  매칭엔진 Open
***************************************************************************** */
LSM* Lsm_Open( char *name)
{
	LSM	*mem;
	char	*ptr;
	char	file_name[ 512];

	mem = malloc( sizeof( LSM));
	if( mem == NULL)
	{
		LsmErr( "malloc error.");
		goto error;
	}
	memset( mem, 0x00, sizeof( LSM));

	mem->mem = Mem_Open( LOG_IPC_KEY);
	if( mem->mem == NULL)
	{
		LsmCri( "Mem_Open error.");
		goto error_1;
	}
	mem->map = Mem_GetPtr( mem->mem);
	LsmMsg( "attach shared memory ... ptr=[%p]", mem->map);

	mem->sem = Sem_Open( LOG_IPC_KEY);
	if( mem->sem == NULL)
	{
		LsmCri( "Sem_Open error. key=[0x%08x]", LOG_IPC_KEY);
		goto error_2;
	}

	sprintf( file_name, "%s", mem->map->stat.pipe_name);

	mem->fd = open( file_name, O_RDWR | O_NONBLOCK);
	if( mem->fd < 0)
	{
		LsmErr( "pipe open error. name=[%s]", file_name);
		goto error_3;
	}
	LsmMsg( "pipe open. name=[%s] fd=[%d]", file_name, mem->fd);

	if( name != NULL)
	{
		ptr = strrchr( name, '/');
		if( ptr == NULL)	ptr = name;
		else				ptr += 1;
		mem->name_sz = strlen( ptr);
		memcpy( mem->name, ptr, mem->name_sz);
		mem->index_pos = Lsm_FindIdx( mem, ptr);
	}
	else
	{
		mem->name[ 0] = 0;
		mem->name_sz = 0;
	}


	return mem;

#if 0
	error_4:
		close( mem->fd);
#endif
	error_3:
		Sem_Close( mem->sem);
	error_2:
		Mem_Close( mem->mem);
	error_1:
		free( mem);
	error:
		return NULL;
}

/** ***************************************************************************
**  @fu         int Lsm_Close( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  매칭엔진 Close
***************************************************************************** */
int Lsm_Close( LSM *mem)
{
	int		rtn;

	rtn = Sem_Close( mem->sem);
	if( rtn < 0)
	{
		LsmCri( "Sem_Close error. sem=[%p] key=[0x%08x]", mem->sem, LOG_IPC_KEY);
		return -1;
	}

	rtn = Mem_Close( mem->mem);
	if( rtn < 0)
	{
		LsmCri( "Mem_Close error. mem=[%p] key=[0x%08x]", mem->mem, LOG_IPC_KEY);
		return -1;
	}

	if( mem->fd >= 0) close( mem->fd);

	mem->fd = -1;
	mem->sem = NULL;
	mem->mem = NULL;

	free( mem);

	return 1;
}

/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  index initial
***************************************************************************** */
int Lsm_Init( LSM *mem)
{
	int			i;
	int			rtn;
	LOG_RECORD	*rec;
	LOG_INDEX	*index;

	time( &mem->map->stat.ctime);
	mem->map->stat.key = LOG_IPC_KEY;
	mem->map->stat.rtime = 0;
	mem->map->stat.dtime = 0;
	mem->map->stat.wpos = 1;
	mem->map->start = -1;
	mem->map->end = -1;

	/* record initial */
	for( i = 0; i < LOG_MAX_REC; i++)
	{
		rec = &mem->map->rec[ i];
		rec->pos = i;
	}
	/* index initial */
	for( i = 0; i < LOG_MAX_IDX; i++)
	{
		index = &mem->map->index[ i];
		index->pos = -1;
	}

	rtn = Lsm_LoadConfig( mem);
	if( rtn < 0)
	{
		LogCri( "Lsm_LoadConfig error. rtn=[%d]", rtn);
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - pos
**  @retval     실패    - -1
**  @brief
**  find index pos by name
***************************************************************************** */
int Lsm_LoadConfig( LSM *mem)
{
	char		cfg_name[ 512];
	CFG			*cfg;

	char		*ptr;
	int			pos;

	LOG_INDEX	*index;

	sprintf( cfg_name, "%s/%s", getenv( "MAT_CFG"), "log.cfg");
	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "log config file open error. name=[%s]", cfg_name);
		return -1;
	}

	Cfg_Get( cfg, "log_path", mem->map->stat.log_path, 512);
	Cfg_Get( cfg, "pipe_name", mem->map->stat.pipe_name, 512);
	mem->map->stat.dtime = Cfg_GetInt( cfg, "batch_time");

	Cfg_Set( cfg, "FileList");
	ptr = Cfg_GetFirstPtr( cfg);
	while( ptr != NULL)
	{
		int			stat = 0;
		char		*token = ":";
		char		rec[ 512];
		char		*rec_ptr = rec, *end = ptr;

		memcpy( rec, ptr, strlen( ptr) +1);
		LogDbg( "rec=[%s]", rec);
		stat = 0;
		while( end != NULL)
		{
			end = strpbrk( rec_ptr, token);
			if( end) *end = 0;
			switch( stat)
			{
				case 0:		/* pos */
					pos = atoi( rec_ptr);
					LogDbg( "pos=[%d]", pos);
					index = &mem->map->index[ pos];
					index->pos = pos;
					break;
				case 1:		/* name */
					TrimR( rec_ptr);
					memcpy( index->name, rec_ptr, strlen( rec_ptr) +1);
					break;
				case 2:		/* comment */
					memcpy( index->comment, rec_ptr, strlen( rec_ptr) +1);
					break;
			}

			rec_ptr = end +1;
			stat++;
		}
		ptr = Cfg_GetNextPtr( cfg);
	}

	Cfg_Close( cfg);

	return 1;
}

/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - pos
**  @retval     실패    - -1
**  @brief
**  find index pos by name
***************************************************************************** */
int	Lsm_Write( const char *file, const char *func, int line, int level, LOG *log, char *format, ...)
{
	va_list		args;
	int			sz, rtn, msg_sz;
	int			hd_sz = 0;

	if( log == NULL)
	{
		log = Log_Open( NULL);
		if( log == NULL)
		{
			LogCri( "Log_Open error. err=[%d:%s]", errno, strerror( errno));
			return -1;
		}
		if( LogPtr == NULL) LogPtr = log;
	}
	if( log->mem != NULL)
	{
		sz = Lsm_Write( file, func, line, level, log, format, args);
		return sz;
	}

	/* log level에 따라 거름 */
	if( level < log->level) return 0;

#if 0
	/* 마지막 로그 문자가 개행문자가 아니면 개행문자를 입력하여 정상로그를 첫 컬럼에서부터 쓴다 */
	if( log->last != log->tail[ log->tail_sz -1])		Log_Out( log, log->tail, log->tail_sz);
#endif

	log->file = ( char *)file;
	log->func = ( char *)func;
	log->line = line;

	hd_sz = Log_Head( log, file, func, line, level);
	sz = hd_sz;

	va_start( args, format);
	sz += vsnprintf( &log->buf[ sz], LOG_BUFFER_SIZE - hd_sz, format, args);
	va_end( args);

	if( sz > 60000)
	{
		msg_sz = sz;
		sz = hd_sz;
		sz += sprintf( &log->buf[ sz], "data size too long. sz=[%d]", msg_sz);
	}

	if( level == LOG_LEV_ERR)
	{
		if( LOG_BUFFER_SIZE - sz > 0) 
		sz += snprintf( &log->buf[ sz], LOG_BUFFER_SIZE - sz, " Err=[%d:%s]", errno, strerror( errno));
	}

	if( log->tail != NULL)
	{
		sz += snprintf( &log->buf[ sz], LOG_BUFFER_SIZE - sz, "%s", log->tail);
	}
	log->buf[ sz] = 0;


	if( level == LOG_LEV_USR)
	{
		if( log->user != NULL) 
		{
			rtn = log->user( log->user_data, log->buf, sz);
			if( rtn < 0)
			{
				return rtn;
			}
		}
	}
	else
	{
		Log_Out( log, log->buf, sz);
	}

	return sz;
}


/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - pos
**  @retval     실패    - -1
**  @brief
**  find index pos by name
***************************************************************************** */
int Lsm_FindIdx( LSM *mem, char *name)
{
	int			i, sz;
	LOG_INDEX	*index;

	sz = strlen( name);
	if( sz <= 0) return -1;

	for( i = 0; i < LOG_MAX_IDX; i++)
	{
		index = &mem->map->index[ i];
		if( index->pos < 0) continue;
		if( !memcmp( index->name, name, sz +1)) return i;
	}
	for( i = 0; i < LOG_MAX_IDX; i++)
	{
		index = &mem->map->index[ i];
		if( index->pos < 0)
		{
			index->pos = i;
			index->cnt = 0;
			memcpy( index->name, name, sz +1);
			return i;
		}
	}

	return -1;
}

/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem 	- 매칭 struct pointer
**  @param      int timeout - timeout(micro second)
**  @return     성공    - record position
**  @return     timeout - LOG_TIMEOUT(-9999)
**  @retval     실패    - -1
**  @brief
**  pipe로 부터 체결 record position을 수신
***************************************************************************** */
int Lsm_WritePipe( LSM*mem)
{
	int				rtn;

	retry:
	LsmDel( "write data 1 byte(s). fd=[%d]", mem->fd);
	rtn = write( mem->fd, "1", 1);
	if( rtn < 1)
	{
		if( rtn == 0)
		{
			LsmWar( "pipe full. fd=[%d] rtn=[%d]", mem->fd, rtn);
			return 0;
		}

		switch( errno)
		{
			case EAGAIN:
			case EINTR:
				goto retry;
			default:
				LsmErr( "pipe write error. fd=[%d] rtn=[%d]", mem->fd, rtn);
				return -1;
		}
	}

	return rtn;
}

/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem 	- 매칭 struct pointer
**  @param      int timeout - timeout(micro second)
**  @return     성공    - record position
**  @return     timeout - LOG_TIMEOUT(-9999)
**  @retval     실패    - -1
**  @brief
**  pipe로 부터 체결 record position을 수신
***************************************************************************** */
int Lsm_ReadPipe( LSM*mem, int timeout)
{
	int				rtn;
	fd_set			rfds;
	char			rec[ 512];
	struct timeval	tv, *tp = &tv;

	rtn = read( mem->fd, rec, 1);
	if( rtn >= 1) return rtn;
	if( mem->map->stat.rec_cnt > 0) return 1;
	if( mem->map->end > 0) return 1;

	while( Continue)
	{
		tp->tv_sec  = timeout / 1000000;
		tp->tv_usec = timeout % 1000000;

		FD_ZERO( &rfds);
		FD_SET( mem->fd, &rfds);

		rtn = select( mem->fd +1, &rfds, NULL, NULL, tp);
		if( rtn < 0)
		{
			LsmErr( "pipe select error. fd=[%d]", mem->fd);
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
			LsmDel( "recv timeout. timeout=[%d.%d]", timeout / 1000000, timeout % 1000000);
			return LOG_TIMEOUT;
		}
		LsmDel( "get event ...");

		if( FD_ISSET( mem->fd, &rfds))
		{
			rtn = read( mem->fd, rec, 1);
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

				LsmErr( "pipe read error. fd=[%d] rtn=[%d]", mem->fd, rtn);
				return -1;
			}

			LsmDel( "pipe read ... rtn=[%d]", rtn);
			return 1;
		}
	}

	return -1;
}

/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  주문 insert 
**	무조건 첫번째에 주문 레코드 삽입
**	추후 index가 확정 되면 key에 따라 해당 위치에 삽입
***************************************************************************** */
int Lsm_Send( LSM *mem, char *rec, int sz)
{
	int				rtn;
	int				pos, npos;
	LSM_HEAD	*head, *nhead;
	LOG_RECORD		*rptr, *nptr;
	LOG_INDEX		*index;

	/* 빈레코드를 찾아 가져올때부터 lock을 해야 두개의 process가 write일때 중복이 안생김 */

	pos = Lsm_GetEmptyRecordPos( mem, 1000);
	if( pos < 0)
	{
		goto no_empty_rec;
	}

	rtn = Lsm_Lock( mem, 1000);
	rptr = &mem->map->rec[ pos];
	rptr->pos = pos;
	head = &rptr->head;
	head->index_pos = mem->index_pos;
	index = &mem->map->index[ mem->index_pos];
	index->cnt++;
	index->sum++;
	time( &index->w_time);
	memcpy( head->name, mem->name, mem->name_sz +1); 
	memcpy( &rptr->rec, rec, sz +1);
	head->sz = sz;

	/* 최초 record */
	if( mem->map->start == -1)
	{
		mem->map->start = pos;
		mem->map->end   = pos;
		head->prev = -1;
		head->next = -1;
		mem->map->cnt++;
		mem->map->dcnt++;
		head->gubun = 1;
		mem->map->stat.rec_cnt++;

		goto success;
	}

	/* start record get */
	npos = mem->map->start;
	nptr = &mem->map->rec[ npos];

	/* insert current record at first */
	nhead = &nptr->head;
	nhead->prev = pos;
	mem->map->start = pos;
	head->prev = -1;
	head->next = npos;
	head->gubun = 1;
	mem->map->cnt++;
	mem->map->dcnt++;
	mem->map->stat.rec_cnt++;


	success:
	Lsm_WritePipe( mem);
	mem->map->spid = getpid();
	time( &mem->map->stime);
	Lsm_Unlock( mem);
	return pos;

	no_empty_rec:
		rtn = Log_Append( LsmPtr, rec, sz);
		if( rtn < 0) return -1;
#if 1
		printf( "[no empty rec]%.*s", sz, rec);
#endif
		Lsm_Unlock( mem);
		return LOG_TIMEOUT;

#if 0
	error:
		Lsm_Unlock( mem);
		return -1;
#endif
}

/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  queue read
***************************************************************************** */
int Lsm_Recv( LSM*mem, char *rec, int sz, int timeout)
{
	int				rtn;
	int				pos, ppos, npos;	/* curr/prev/next */
	LSM_HEAD	*head;
	LOG_RECORD		*ptr, *pptr;	/* curr/prev/next */
	LOG_INDEX		*index;

	rtn = Lsm_ReadPipe( mem, timeout);
	LsmDel( "Lsm_ReadPipe rtn=[%d]", rtn);
	if( rtn <= 0)
	{
		if( rtn == LOG_TIMEOUT)
		{
			pos = mem->map->end;
			if( pos >= 0) goto process;
			LsmDel( "Lsm_ReadPipe timeout.");
			return LOG_TIMEOUT;
		}
		LsmDel( "Lsm_ReadPipe error.");
		return rtn;
	}

	process:
	Lsm_Lock( mem, 1000);

	/* get last record */
	pos = mem->map->end;
	LsmDel( "pos=[%d]", pos);
	if( pos < 0)
	{
		mem->map->cnt = 0;
		Lsm_Unlock( mem);
		return 0;
	}
	ptr = &mem->map->rec[ pos];
	head = &ptr->head;
	ppos = head->prev;
	npos = head->next;

	index = &mem->map->index[ head->index_pos];
	index->cnt--;

	if( ppos >= 0)		/* 처음이 아니면 */
	{
		pptr = &mem->map->rec[ head->prev];
		pptr->head.next = npos;
	}
	else
	{
		/* 처음 record 이면 index update */
		mem->map->start = ppos;
	}

	/* 마지막 record 이므로 index update */
	mem->map->end = ppos;

	/* record update - gubun을 0으로 setting하여 빈레코드로 표시 */
	ptr->head.gubun = 0;

	/* index에 record 갯수를 1 감소 */
	mem->map->cnt--;
	mem->map->stat.rec_cnt--;

	memcpy( rec, ptr->rec, head->sz);
	rec[ head->sz] = 0;

	mem->map->rpid = getpid();
	time( &mem->map->rtime);

	Lsm_Unlock( mem);

	return head->sz;

#if 0
	error:
		return -1;
#endif
}

/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  queue read
***************************************************************************** */
int Lsm_GetPos( LSM*mem, int timeout)
{
	int				rtn;
	int				pos, ppos, npos;	/* curr/prev/next */
	LSM_HEAD	*head;
	LOG_RECORD		*ptr, *pptr;	/* curr/prev/next */
	LOG_INDEX		*index;

	rtn = Lsm_ReadPipe( mem, timeout);
	if( rtn < 1)
	{
		if( rtn == LOG_TIMEOUT)
		{
			return rtn;
		}
		LsmDel( "Lsm_ReadPipe error.");
		return rtn;
	}

	Lsm_Lock( mem, 1000);
	/* get last record */
	pos = mem->map->end;
	LogDel( "pos=[%d]", pos);
	if( pos < 0)
	{
		/* pipe에 남아있는 DATA 제거 */
		Lsm_Unlock( mem);
		return LOG_NODATA;
	}
	ptr = &mem->map->rec[ pos];
	head = &ptr->head;
	ppos = head->prev;
	npos = head->next;

	index = &mem->map->index[ head->index_pos];
	index->cnt--;

	if( ppos >= 0)		/* 처음이 아니면 */
	{
		pptr = &mem->map->rec[ head->prev];
		pptr->head.next = npos;
	}
	else
	{
		/* 처음 record 이면 index update */
		mem->map->start = ppos;
	}

	/* 마지막 record 이므로 index update */
	mem->map->end = ppos;

	/* record update - gubun을 0으로 setting하여 빈레코드로 표시 */
	ptr->head.gubun = 0;

	/* index에 record 갯수를 1 감소 */
	mem->map->cnt--;
	mem->map->stat.rec_cnt--;

	mem->map->rpid = getpid();
	time( &mem->map->rtime);

	Lsm_Unlock( mem);

	return pos;

#if 0
	error:
		return -1;
#endif
}

/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - record position
**  @retval     실패    - -1
**  @brief
**  Lsm_SetRecord 이후 Lock 해제 및 insert
***************************************************************************** */
int	Lsm_Insert( LSM *mem)
{
	int				pos, npos;
	LSM_HEAD	*head, *nhead;
	LOG_RECORD		*ptr, *nptr;

	LogDbg( "mem->pos=[%d]", mem->pos);
	pos = mem->pos;
	ptr = &mem->map->rec[ pos];
	ptr->pos = pos;

	head = &ptr->head;
	Lsm_Lock( mem, 1000);

	/* 최초 record */
	if( mem->map->start == -1)
	{
		mem->map->start = pos;
		mem->map->end   = pos;
		head->prev = -1;
		head->next = -1;
	}
	else
	{
		/* start record get */
		npos = mem->map->start;
		nptr = &mem->map->rec[ npos];

		/* insert current record at first */
		nhead = &nptr->head;
		nhead->prev = pos;
		mem->map->start = pos;
		head->prev = -1;
		head->next = npos;
	}

	mem->map->cnt++;
	head->gubun = 1;
	mem->map->stat.rec_cnt++;
	Lsm_Unlock( mem);

	Lsm_WritePipe( mem);
	mem->map->spid = getpid();
	time( &mem->map->stime);

	return pos;

#if 0
	error:
		Lsm_Unlock( mem);
		return -1;
#endif
}

/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - record position
**  @retval     실패    - -1
**  @brief
**  Lsm_SetRecord 이후 Lock 해제 및 insert
***************************************************************************** */
int	Lsm_InsertRecord( LSM *mem)
{
	int				pos, npos;
	LSM_HEAD	*head, *nhead;
	LOG_RECORD		*ptr, *nptr;

	pos = mem->pos;
	ptr = &mem->map->rec[ pos];
	ptr->pos = pos;

	head = &ptr->head;
	memcpy( &ptr->rec, ( char *)mem->ptr, mem->sz +1);
	head->sz = mem->sz;


	/* 최초 record */
	if( mem->map->start == -1)
	{
		mem->map->start = pos;
		mem->map->end   = pos;
		head->prev = -1;
		head->next = -1;
		mem->map->cnt++;
		head->gubun = 1;
		Lsm_Lock( mem, 1000);
		mem->map->stat.rec_cnt++;
		Lsm_Unlock( mem);

		goto success;
	}

	/* start record get */
	npos = mem->map->start;
	nptr = &mem->map->rec[ npos];

	/* insert current record at first */
	nhead = &nptr->head;
	nhead->prev = pos;
	mem->map->start = pos;
	head->prev = -1;
	head->next = npos;
	head->gubun = 1;
	mem->map->cnt++;
	Lsm_Lock( mem, 1000);
	mem->map->stat.rec_cnt++;
	Lsm_Unlock( mem);

	success:
	Lsm_WritePipe( mem);
	Lsm_Unlock( mem);
	mem->map->spid = getpid();
	time( &mem->map->stime);

	return pos;

#if 0
	error:
		Lsm_Unlock( mem);
		return -1;
#endif
}

/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - record position
**  @retval     실패    - -1
**  @brief
**  빈 record 찾기
***************************************************************************** */
int Lsm_GetEmptyRecordPos( LSM *mem, int timeout)
{
	int		pos;
	int		cnt = 0;

	Lsm_Lock( mem, 1000);

	pos = mem->map->stat.wpos;

	while( Continue)
	{
		if( mem->map->rec[ pos].head.gubun == 0) 
		{
			mem->map->stat.wpos = pos +1;
			if( mem->map->stat.wpos >= LOG_MAX_REC)	mem->map->stat.wpos = 1;
			Lsm_Unlock( mem);
			return pos;
		}
		pos++;
		if( pos >= LOG_MAX_REC)	pos = 1;
		cnt++;
		if( cnt >= LOG_MAX_REC)
		{
			LsmMsg( "빈 record가 없습니다.");
			break;
		}
	}
	Lsm_Unlock( mem);

	return LOG_TIMEOUT;
}

/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - record position
**  @retval     실패    - -1
**  @brief
**  position을 입력하여 record 찾기
***************************************************************************** */
LOG_RECORD *Lsm_GetRecordByPos( LSM*mem, int pos)
{
	return &mem->map->rec[ pos];
}

/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - record position
**  @retval     실패    - -1
**  @brief
**  position을 입력하여 record 찾기
***************************************************************************** */
int	Lsm_Head( LSM *lsm, char *buf, LOG_INDEX *index, LSM_HEAD *head)
{
	int				sz = 0;
	int				pos;
	struct timeval	*tvp;
	struct tm		*tp;
	char			lv[128] = "TDAMLWECB U                               \0";

#if 0
	int				sz_hd = 0;
	time_t			cur_time;
	int				color[16] = { 37, 37, 36, 32, 33, 31, 31, 31, 31, 37, 37};
#endif

	pos = head->level / 10;
	tvp = &head->w_time;
	tp  = localtime( &tvp->tv_sec);

	sz += sprintf( &buf[ sz], "%02d:%02d:%02d-%06ld ", tp->tm_hour, tp->tm_min, tp->tm_sec, tvp->tv_usec);
	sz += sprintf( &buf[ sz], "%c ", lv[ pos]);
	sz += sprintf( &buf[ sz], "%s:%s(%d)  ", head->file, head->func, head->line);
	if( sz <= LOG_HEAD_SIZE)
	{
		memset( &buf[ sz], 0x20, LOG_HEAD_SIZE - sz);
		sz = LOG_HEAD_SIZE;
	}

	return sz;
}

/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  semaphore lock 수행
***************************************************************************** */
int Lsm_Lock( LSM *mem, int timeout)
{
	int				rtn;
	struct timespec	ts;

	ts.tv_sec  = timeout / 1000000;
	ts.tv_nsec = ( timeout % 1000000) * 1000;
	
	while( rtn < 0)
	{
		rtn = semtimedop( mem->sem->id, mem->sem->lock, 1, &ts);
	}

	return rtn;
}

/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - 통화 번호
**  @retval     실패    - -1
**  @brief
**  semaphore lock 해제
***************************************************************************** */
int Lsm_Unlock( LSM *mem)
{
	int		rtn;

	rtn = semop( mem->sem->id, mem->sem->unlock, 1);

	return rtn;
}

/** ***************************************************************************
**  @fu         int Lsm_( LSM*mem)
**  @param      LSM*mem - 매칭 struct pointer
**  @return     성공    - 1
**  @retval     실패    - -1
**  @brief
**  stat 출력
***************************************************************************** */
int Lsm_Stat( LSM*mem)
{
	int			i;
	int			cols = 100;
	LOG_INDEX	*index;

	printf( "mem                  = [%p]\n", mem);
	printf( "mem->mem             = [%p]\n", mem->mem);
	printf( "mem->mem->id         = [%d]\n", mem->mem->id);
	printf( "mem->sem             = [%p]\n", mem->sem);
	if( mem->sem != NULL)
	{
	printf( "mem->sem->id         = [%d]\n", mem->sem->id);
	}
	printf( "fd(fifo)             = [%d]\n", mem->fd);
	printf( "base(mem->map)       = [%p]\n", ( char *)mem->map);
	printf( "---------------------------------------------\n");
	printf( "ctime                = [%s]\n", TtoS( mem->map->stat.ctime));
	printf( "rtime                = [%s]\n", TtoS( mem->map->stat.rtime));
	printf( "dtime                = [%d]\n", mem->map->stat.dtime);
	printf( "rec_cnt              = [%d]\n", mem->map->stat.rec_cnt);
	printf( "wpos                 = [%d]\n", mem->map->stat.wpos);
	printf( "key                  = [0x%08x]\n", mem->map->stat.key);
	printf( "pipe_dir             = [%s]\n", mem->map->stat.pipe_name );
	printf( "---------------------------------------------\n");

	printf( "start ");
	printf( "  end ");
	printf( "d_cnt ");
	printf( "count ");
	printf( "send_pid ");
	printf( "%-20s", "send_time");
	printf( "recv_pid ");
	printf( "%-20s", "recv_time");
	printf( "key      ");
	printf( "\n");

	printf( "%5d ", mem->map->start);
	printf( "%5d ", mem->map->end);
	printf( "%5d ", mem->map->dcnt);
	printf( "%5d ", mem->map->cnt);
	printf( "%8d ", mem->map->spid);
	printf( "%-20s ", TtoS( mem->map->stime));
	printf( "%8d ", mem->map->rpid);
	printf( "%-20s ", TtoS( mem->map->rtime));
	printf( "\n");
	for( i = 0; i < cols; i++) printf( "-"); 
	printf( "\n");

	for( i = 0; i < LOG_MAX_IDX; i++)
	{
		index = &mem->map->index[ i];
		if( index->pos < 0) continue;

		printf( "%3d ", index->pos);
		printf( "%-20s ", index->name);
		printf( "%8d ", index->cnt);
		printf( "%8d ", index->sum);
		printf( "\n");
	}

	return 1;
}


