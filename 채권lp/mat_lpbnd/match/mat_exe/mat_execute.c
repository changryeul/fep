/** ***************************************************************************
**  @file       mat.c
**  @date       2023/08/23
**  @author     cdc
**  @version    V2.0.20230823
**  @brif
**  매칭엔진 라이브러리
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
#include "mat.h"

#include "order.h"
#include "sise.h"

#include "main.h"

extern int		Continue;

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat - 매칭 struct pointer
**  @return     성공    - next position
**  @retval     실패    - -1
**  @brief
**  체결처리 - mat_exe main
***************************************************************************** */
int Mat_Execute( MAT *mat, int timeout)
{
	int			pos;

	pos = Mat_ReadPipe( mat, timeout);
	if( pos <= 0)
	{
		if( pos == MAT_TIMEOUT)
		{
			LogDel( "Mat_ReadPipe timeout... exe_cnt=[%d]", mat->map->stat.exe_cnt);
			return pos;
		}
		return -1;
	}

	Mat_Lock( mat);

	LogDel( "execute send ... pos[%d]", pos);
	mat->map->stat.exe_cnt--;

	Mat_Unlock( mat);

	LogDel( "exe_cnt = [%d]", mat->map->stat.exe_cnt);

	return pos;
}

/** ***************************************************************************
**  @fu         int Mat_( MAT *mat)
**  @param      MAT *mat 	- 매칭 struct pointer
**  @param      int timeout - timeout(micro second)
**  @return     성공    - record position
**  @return     timeout - MAT_TIMEOUT(-9999)
**  @retval     실패    - -1
**  @brief
**  pipe로 부터 체결 record position을 수신
***************************************************************************** */
int Mat_ReadPipe( MAT *mat, int timeout)
{
	int				rtn, pos;
	fd_set			rfds;
	struct timeval	tv, *tp = &tv;

	while( Continue)
	{
		tp->tv_sec  = timeout / 1000000;
		tp->tv_usec = timeout % 1000000;

		FD_ZERO( &rfds);
		FD_SET( mat->exe_fd, &rfds);

		rtn = select( mat->exe_fd +1, &rfds, NULL, NULL, tp);
		if( rtn < 0)
		{
			LogErr( "pipe select error. fd=[%d]", mat->exe_fd);
			switch( errno)
			{
				case EBUSY:
				case ECANCELED:
				case EINTR:
				case EAGAIN:
					continue;
				default:
					break;
			}
			return -1;
		}
		else if( rtn == 0)
		{
			LogDel( "recv timeout. timeout=[%d.%d]", timeout / 1000000, timeout % 1000000);
			return MAT_TIMEOUT;
		}

		if( FD_ISSET( mat->exe_fd, &rfds))
		{
			pos = -1;
			rtn = read( mat->exe_fd, ( char *)&pos, sizeof( int));
			if( rtn < ( int)sizeof( int))
			{
				if( rtn >= 0)
				{
					/* non_block mode */
					return MAT_TIMEOUT;
				}
				switch( errno)
				{
					case EBUSY:
					case ECANCELED:
					case EINTR:
					case EAGAIN:
						continue;
					default:
						break;
				}
				LogErr( "pipe read error. fd=[%d] rtn=[%d]", mat->exe_fd, rtn);
				return -1;
			}
			if( pos <= 0 || pos >= MAT_MAX_RECORD)
			{
				LogCri( "get pos error. pos=[%d] rtn=[%d]", pos, rtn);
				return -1;
			}

			LogDel( "pipe read ... pos=[%d]", pos);
			return pos;
		}
	}

	return -1;
}


