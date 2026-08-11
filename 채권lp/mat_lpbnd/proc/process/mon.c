#include <stdio.h>
#include <stdlib.h>

#include "task.h"

MAP				*Map;
extern PROC		*Proc;
extern int		Continue;

int			Alarm = 1;					/* alarm이 켜진 상태로 시작  0=off 1=on */
time_t		CurTime;
int			StartRec = 0;				/* 모니터링 시작 프로세스 번호 */
int			Lines;						/* 출력 레코드 수 */
char		Filter[ 32];				/* 모니터링 필터 - 일치하는 내용만 보임(id) */

int Mon( int argc, char *argv[])
{
	MapInit();

	if( argc > 1)	sprintf( Filter, "%s", argv[ 1]);
	else			sprintf( Filter, "%s", "");
	MonMain();
	MapEnd();

	return 1;
}

int MonMain()
{
	int			rtn;
	int			pos;
	int			id, field, line;
	char		*argv[ 8];
	char		p_name[ 64];
	MAP_MENU	*menu;
	PROC_TBL	*tbl;

	if( Proc == NULL)
	{
		Proc = Proc_Open( Param->cfg_name);
		if( Proc == NULL)
		{
			LogCri( "Proc_Open error.");
			goto error;
		}
	}

	Map = Map_Open( Param->map_name);
	if( Map == NULL)
	{
		LogCri( "Map_Open error. name=[%s]", Param->map_name);
		goto error_1;
	}

	MonInit( Map, Proc);

	menu = Map_MenuOpen( Map, '1');
	if( menu == NULL)
	{
		LogCri( "Map_MenuOpen error.");
		goto error_2;
	}

	Map_DisplayMap( Map);
	Map_Message( Map, "Monitoring start. filter=[%s]", Filter);
	Continue = 1;

	while( Continue)
	{
		Map_CursorOff( Map);
		Map_DisplayField( Map);
		Map_GotoEnd( Map);

		Map_CursorOn( Map);
		id = Map_Menu( Map, menu, Param->mon_interval);
		field = id / 1000;
		line  = id % 1000;
		switch( field)
		{
			case 101:		/* sub map */
				sprintf( p_name, "%s", menu->select->data);
				rtn = MonProcessStat( Map, Proc, p_name);
				Map_DisplayMap( Map);
				break;
			case 103:		/* run */
				pos = AtoI( menu->select->buf, 4);
				Map_Message( Map, "%d", pos);
				tbl = &Proc->base[ pos];
				argv[ 1] = tbl->name;
				sprintf( p_name, "%s", tbl->id);
				argv[ 1] = p_name;
				TaskRun( 2, argv);
				Map_Message( Map, "run  process id=[%s] pid=[%d]", tbl->name, tbl->pid);
				break;
			case 104:		/* stop */
				pos = AtoI( menu->select->buf, 4);
				tbl = &Proc->base[ pos];
				Map_Message( Map, "stop process field=[%d] line=[%d] id=[%s] pid=[%d]", field, line, tbl->name, tbl->pid);
				sprintf( p_name, "%s", tbl->id);
				argv[ 1] = p_name;
				TaskStop( 2, argv);
				break;
			case 0:
			default:
				switch( menu->key)
				{
					case WIN_KEY_ESC:
					case WIN_KEY_ENTER:
						goto end;
						break;
					case 'r':
					case 'R':
						MonInit( Map, Proc);
						Map_DisplayMap( Map);
						break;
#if 0
					case WIN_KEY_PGDN:
						StartRec += Lines;
						if( StartRec >= Proc->stat->cnt)
						{
							StartRec = Proc->stat->cnt -1;
							Map_Message( Map, "End of Process.");
						}
						MonInit( Map, Proc);
						Map_DisplayMap( Map);
						break;
					case WIN_KEY_PGUP:
						StartRec -= Lines;
						if( StartRec < 0) 
						{
							Map_Message( Map, "Start of Process.");
							StartRec = 0;
						}
						MonInit( Map, Proc);
						Map_DisplayMap( Map);
						break;
					case WIN_KEY_DOWN:
						StartRec++;
						if( StartRec >= Proc->stat->cnt)
						{
							StartRec = Proc->stat->cnt -1;
							Map_Message( Map, "End of Process.");
						}
						MonInit( Map, Proc);
						Map_DisplayMap( Map);
						break;
					case WIN_KEY_UP:
						StartRec--;
						if( StartRec < 0) 
						{
							Map_Message( Map, "Start of Process.");
							StartRec = 0;
						}
						MonInit( Map, Proc);
						Map_DisplayMap( Map);
						break;
#endif
					default:
						break;
				}	/* switch menu->key */
				break;
		}	/* switch id */
	}

	end:
	Map_MenuClose( Map, menu);
	Map_CursorOn( Map);
	Map_Close( Map);
	return rtn;

	error_2:
		Map_Close( Map);
	error_1:
		Proc_Close( Proc);
		Proc = NULL;
	error:
		return -1;
}

/*********************************************************************************************************
 *
*********************************************************************************************************/
int MonInit( MAP *map, PROC *proc)
{
	int			line = 0, pos = 0;
	int			filter_len = 0;
	char		name[ 32], *p;
	PROC_TBL	*curr;
	PROC_STAT	*stat;

	MAP_FIELD	*field;

	Lines = Map_GetLineSize( map) -7;

	Map_SetField( map, "alarm", Map_AlarmStat, &Alarm);
	Map_SetField( map, "time", Map_TimeProc, &CurTime);


	stat = proc->stat;
	Map_SetDataPtr( map, "version", &stat->ver_string);
	Map_PrintName( map, "create", "%s", TtoS( stat->create));
	Map_SetDataPtr( map, "tblcnt", &stat->cnt);
	Map_SetDataPtr( map, "week_pos", &stat->week);

	/* 첫번째 줄을 daemon에 할당 */
	/* daemon */
	curr = &proc->base[ MAX_PROC_TBL];
	Map_SetDataPtr( map,	"id",		&curr->id);
	Map_SetDataPtr( map,	"pid",		&curr->pid);
	Map_SetField( map,		"srtime",	Map_TimeField, &curr->sr_time);
	Map_SetField( map,		"ertime",	Map_TimeField, &curr->er_time);
	Map_SetDataPtr( map,	"r_name",	&curr->name);

	while( pos < ( proc->stat->cnt - StartRec))
	{
		curr = &proc->base[ pos + StartRec];
		LogDel( "curr->name=[%s] curr->used=[%d] pos=[%d] line=[%d]", curr->name, curr->used, pos, line);

		/*************************************************/
		/* used == 0 사용하지 않은 record                */
		/*************************************************/
		if( curr->used == 0)
		{
			pos++;
			continue;
		}

		/*************************************************/
		/* Filter 적용                                   */
		/*************************************************/
		filter_len = strlen( Filter);
		if( filter_len > 0)
		{
			if( strstr( curr->id, Filter) == NULL)
			{
				pos++;
				continue;
			}
		}

		sprintf( name, "%s_%d", "id",		line);	Map_SetDataPtr( map, name, &curr->id);
		sprintf( name, "%s_%d", "cmd",		line);	Map_SetDataPtr( map, name, &curr->cmd);
		sprintf( name, "%s_%d", "stat",		line);	Map_SetField( map, name, Map_StatCheck, curr);
		sprintf( name, "%s_%d", "run",		line);	Map_SetDataPtr( map, name, &curr->r_cnt);
		sprintf( name, "%s_%d", "max",		line);	Map_SetDataPtr( map, name, &curr->max_run);
		sprintf( name, "%s_%d", "option",	line);	Map_SetDataPtr( map, name, &curr->option);
		LogDel( "s_time=[%06d]", curr->s_time);
		LogDel( "e_time=[%06d]", curr->e_time);
		sprintf( name, "%s_%d", "stime",	line);	Map_SetDataPtr( map, name, &curr->s_time);
		sprintf( name, "%s_%d", "etime",	line);	Map_SetDataPtr( map, name, &curr->e_time);
		sprintf( name, "%s_%d", "pid",		line);	Map_SetDataPtr( map, name, &curr->pid);
		sprintf( name, "%s_%d", "srtime",	line);	Map_SetField( map, name, Map_TimeField, &curr->sr_time);
		sprintf( name, "%s_%d", "ertime",	line);	Map_SetField( map, name, Map_TimeField, &curr->er_time);
		sprintf( name, "%s_%d", "r_name",	line);	Map_SetDataPtr( map, name, &curr->name);

		sprintf( name, "%s_%d", "c_r",   line);
		field = Map_GetFieldPtr( map, name);
		sprintf( field->buf, "%04d", pos + StartRec);
		sprintf( name, "%s_%d", "c_s",   line);
		field = Map_GetFieldPtr( map, name);
		sprintf( field->buf, "%04d", pos + StartRec);

		line++;
		pos++;
		if( pos >= proc->stat->cnt) break;
	}


	/*************************************************/
	/* 필요없는 값 보이는것을 막는다                 */
	/*************************************************/
	while( 1)
	{
		sprintf( name, "id_%d", line);
		p = ( char *)Map_GetFieldPtr( map, name);
		if( p == NULL) break;

		LogDel( "curr->name=[%s] curr->used=[%d] pos=[%d] line=[%d]", curr->name, curr->used, pos, line);

		sprintf( name, "%s_%d", "id",		line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "%s_%d", "cmd",		line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "%s_%d", "stat",		line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "%s_%d", "run",		line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "%s_%d", "max",		line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "%s_%d", "option",	line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "%s_%d", "stime",	line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "%s_%d", "etime",	line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "%s_%d", "pid",		line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "%s_%d", "srtime",	line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "%s_%d", "ertime",	line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "%s_%d", "r_name",	line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "%s_%d", "c_r",		line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "%s_%d", "c_s",		line);	Map_SetDataPtr( map, name, NULL);

		/*
		sprintf( name, "%s_%d", "c_r",		line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "%s_%d", "c_s",		line);	Map_SetDataPtr( map, name, NULL);
		*/
		line++;
	}

	return 1;
}

int MonProcessStat( MAP *main_map, PROC *proc, char *name)
{
	int			rtn;
	char		map_name[ 512];
	MAP			*map;
	MAP_EDIT	*edit;
	PROC_TBL	*tbl;
	PROC_TBL	tbl_buf, *tblp = &tbl_buf;

	tbl = Proc_FindId( proc, name);
	if( tbl == NULL)
	{
		Map_Message( main_map, "id not found. id=[%s]", name);
		return -1;
	}

	memcpy( tblp, tbl, sizeof( PROC_TBL));

	sprintf( map_name, "%s/map/%s", getenv( "MAT_CFG"), "proc_stat.map");
	map = Map_Open( map_name);
	if( map == NULL)
	{
		Map_Message( main_map, "Map_Open error. name=[%s]", map_name);
		LogCri( "Map_Open error. name=[%s]", map_name);
		return -1;
	}

	Map_SetDataPtr( map, "id",		&tblp->id);
	Map_SetDataPtr( map, "cmd",		&tblp->cmd);
	Map_SetDataPtr( map, "cmd_cnt",	&tblp->cmd_cnt);
	Map_SetDataPtr( map, "used",		&tblp->used   );
	Map_SetDataPtr( map, "stat",		&tblp->stat   );
	Map_SetDataPtr( map, "pid",		&tblp->pid    );
	Map_SetDataPtr( map, "s_time",	&tblp->s_time );
	Map_SetDataPtr( map, "e_time",	&tblp->e_time );
	Map_SetDataPtr( map, "week",		&tblp->week   );
	Map_SetDataPtr( map, "name",		&tblp->name   );
	Map_SetDataPtr( map, "option",	&tblp->option );
	Map_SetDataPtr( map, "r_cnt",		&tblp->r_cnt  );
	Map_SetDataPtr( map, "max_run",	&tblp->max_run);

	Map_SetField( map, "sr_time",	Map_TimeConvert, &tblp->sr_time);
	Map_SetField( map, "er_time",	Map_TimeConvert, &tblp->er_time);

	Map_Message( map, "%s", "edit ");

	edit = Map_EditOpen( map, '1', NULL);
	if( edit == NULL)
	{
		LogCri( "Map_EditOpen error.");
		goto error;
	}

	Map_DisplayMap( map);

	while( Continue)
	{
		Map_DisplayField( map);
#if 0
		rtn = Map_GetKey( map, Param->mon_interval);
#else
		rtn = Map_Edit( map, edit);
#endif
		Map_Message( map, "key=[%d]", rtn);
		switch( rtn)
		{
			case 0:
				break;
			default:
				goto end;
				break;
		}
	}

	end:
	Map_EditClose( map, edit);
	Map_Close( map);

	return 1;

	error:
		Map_Close( map);
		return -1;
}

/*********************************************************************************************************
 *
*********************************************************************************************************/
int Map_AlarmStat( MAP *map, MAP_FIELD *field)
{
	if( *(int *)field->ptr)
	{
		sprintf( field->data, "%s", "ON");
		field->attr[ MAP_ATTR_FORE] = MAP_ATTR_GREEN;
	}
	else
	{
		sprintf( field->data, "%s", "OFF");
		field->attr[ MAP_ATTR_FORE] = MAP_ATTR_RED;
	}

	return 1;
}

int Map_StatCheck( MAP *map, MAP_FIELD *field)
{
	int			rtn;
	PROC_TBL	*pp = ( PROC_TBL *)field->ptr;
	static char	*stat[ 11] = { "TERM", "RUN ", NULL, NULL, NULL, NULL, NULL, NULL, NULL, "STOP", NULL };

	rtn = Proc_HealthCheck( pp);
	if( rtn)
	{
		field->attr[ MAP_ATTR_FORE] = MAP_ATTR_RED;
		AlarmProcess( map, "Process check... id=[%s] at=[%s]", pp->id, GetTimeStr());
	}
	else
	{
		field->attr[ MAP_ATTR_FORE] = MAP_ATTR_GREEN;
	}

	sprintf( field->data, "%s", stat[ pp->stat]);

	return 1;
}

int Map_TimeProc( MAP *map, MAP_FIELD *field)
{
	time_t		cur_time;
	struct tm	*tp;

	time( &cur_time);
	tp = localtime( &cur_time);
	sprintf( field->data, "%02d:%02d:%02d", tp->tm_hour, tp->tm_min, tp->tm_sec);

	return 1;
}

int Map_TimeField( MAP *map, MAP_FIELD *field)
{
	struct tm	*tp;
	time_t		cur_time;

	if( *( time_t *)field->ptr == 0)
	{
		sprintf( field->data, "00:00:00");
		return 1;
	}

	time( &cur_time);
	tp = localtime( field->ptr);

	if( cur_time - *( time_t *)field->ptr > 86400)	
	{
		field->attr[ MAP_ATTR_FORE] = MAP_ATTR_WHITE;
		sprintf( field->data, "%02d/%02d/%02d", tp->tm_year % 100, tp->tm_mon +1, tp->tm_mday);
	}
	else
	if( cur_time - *( time_t *)field->ptr <= Param->mon_interval / 1000000)
	{
		field->attr[ MAP_ATTR_FORE] = MAP_ATTR_CYAN;
		sprintf( field->data, "%02d:%02d:%02d", tp->tm_hour, tp->tm_min, tp->tm_sec);
	}
	else								
	{
		field->attr[ MAP_ATTR_FORE] = 0;
		sprintf( field->data, "%02d:%02d:%02d", tp->tm_hour, tp->tm_min, tp->tm_sec);
	}

	return 1;
}
int Map_TimeConvert( MAP *map, MAP_FIELD *field)
{
	time_t		*time_ptr;

	time_ptr = ( time_t *)field->ptr;

	if( *time_ptr == 0)
	{
		sprintf( field->data, "0000/00/00-00:00:00");
		return 1;
	}

	sprintf( field->data, "%s", TtoS( *time_ptr));

	return 1;
}


int AlarmProcess( MAP *map, char *format, ...)
{
	va_list		args;
	int			sz = 0;
	char		buf[ 8192];

	if( Alarm)
	{
		buf[ 0] = 0x07;
		sz += 1;
	}

	va_start( args, format);
	sz += vsnprintf( &buf[ sz], 8100, format, args);
	va_end( args);

	Map_Message( map, buf);

	return sz;
}







