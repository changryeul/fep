/** ***************************************************************************
**  @file       mon.c
**  @date       2023/08/23
**  @author     cdc
**  @version    V2.0.20230901
**  @brif
**  모니터링
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>

#include "map.h"
#include "log.h"
#include "etc.h"

#include "main.h"
#include "task.h"

extern int	Continue;

typedef struct _mon_
{
	int		alarm;
	time_t	cur_time;
}	MON;

MON		_Mon = 
{
	0,
	0
};
MON		*Mon = &_Mon;

extern CUSTMRGN_SHM_ST	*CustMrgn;


/** ***************************************************************************
**  @fu         int CmdInsert( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int Mon_Main( CUSTMRGN_ST *cust_mrgn)
{
	int			id;
	MAP			*map;
	MAP_MENU	*menu;

	map = Map_Open( Param->map_name);
	if( map == NULL)
	{
		LogMsg( "Map_Open error. name=[%s]", Param->map_name);
		goto error;
	}

	Mon_MainInit( map, cust_mrgn);

	menu = Map_MenuOpen( map, '1');
	if( menu == NULL) goto error_1;

	Map_DisplayMap( map);
	Map_Message( map, "loop start");

	Map_CursorOff( map);
	while( Continue)
	{
		Map_DisplayField( map);
		id = Map_Menu( map, menu, 1000000);

		switch( id)
		{
			case 0: /* timeout */
			default:
				switch( menu->key)
				{
					case 0: /* timeout */
						continue;
					case WIN_KEY_ENTER:
					case WIN_KEY_ESC:
						goto end;
				}
				continue;
		}
	}

	end:
	Map_CursorOn( map);
	Map_Close( map);
	return 1;

	error_1:
		Map_Close( map);
	error:
		return 0;
}

/** ***************************************************************************
**  @func       int Mon_()
**  @param      MAP	*map
**  @return     성공    - +
**  @retval     실패    - -
**  @brief
**	모니터링맵 초기화
***************************************************************************** */
int Mon_MainInit( MAP *map, CUSTMRGN_ST *cust_mrgn)
{
	int			i, line = 0;
	char		name[ 32];
	void		*ptr;
	PAIRMRGN_ST	*fnl_pair;
	PAIRMRGN_ST	*std_pair;

	LogDbg( "Monitor start ... MAX_CUST_CNT=[%d]", MAX_CUST_CNT);

	Map_SetField(   map, "time",	Mon_TimeSet, &Mon->cur_time);
	Map_SetDataPtr( map, "st_no",	&cust_mrgn->s_csac_idnt_no);
	Map_SetDataPtr( map, "grp_id",	&cust_mrgn->s_cust_grp_id);
	Map_SetDataPtr( map, "fnl_cnt",	&cust_mrgn->n_fnl_cnt);
	Map_SetDataPtr( map, "std_cnt",	&cust_mrgn->n_std_cnt);

	for( i = 0; i < cust_mrgn->n_fnl_cnt; i++)
	{
		sprintf( name, "s_pair_id_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		fnl_pair = &cust_mrgn->fnlmgst[ i];

		sprintf( name, "s_pair_id_%d", line);	Map_SetDataPtr( map, name, &fnl_pair->s_pair_id);
//		sprintf( name, "s_clc_dsnc_%d", line);	Map_SetDataPtr( map, name, &fnl_pair->s_clc_dsnc);
//		sprintf( name, "n_digit_%d", line);		Map_SetDataPtr( map, name, &fnl_pair->n_digit);
//		sprintf( name, "d_digit_val_%d", line);	Map_SetDataPtr( map, name, &fnl_pair->d_digit_val);
//		sprintf( name, "d_clc_unit_%d", line);	Map_SetDataPtr( map, name, &fnl_pair->d_clc_unit);
		sprintf( name, "spt_bomg_%d", line);	Map_SetDataPtr( map, name, &fnl_pair->s_spt_bomg_dcd);
		sprintf( name, "d_spt_bymg_%d", line);	Map_SetDataPtr( map, name, &fnl_pair->d_spt_bymg);
		sprintf( name, "d_spt_slmg_%d", line);	Map_SetDataPtr( map, name, &fnl_pair->d_spt_slmg);
		sprintf( name, "fwd_bomg_%d", line);	Map_SetDataPtr( map, name, &fnl_pair->s_fwd_bomg_dcd);
		sprintf( name, "d_fwd_bymg_%d", line);	Map_SetDataPtr( map, name, &fnl_pair->d_fwd_bymg);
		sprintf( name, "d_fwd_slmg_%d", line);	Map_SetDataPtr( map, name, &fnl_pair->d_fwd_slmg);

		line++;
	}

	while( 1)
	{
		sprintf( name, "s_pair_id_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "s_pair_id_%d", line);	Map_SetDataPtr( map, name, NULL);
//		sprintf( name, "s_clc_dsnc_%d", line);	Map_SetDataPtr( map, name, NULL);
//		sprintf( name, "n_digit_%d", line);		Map_SetDataPtr( map, name, NULL);
//		sprintf( name, "d_digit_val_%d", line);	Map_SetDataPtr( map, name, NULL);
//		sprintf( name, "d_clc_unit_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "spt_bomg_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "d_spt_bymg_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "d_spt_slmg_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "fwd_bomg_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "d_fwd_bymg_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "d_fwd_slmg_%d", line);	Map_SetDataPtr( map, name, NULL);
		line++;
	}

	line = 0;
	for( i = 0; i < cust_mrgn->n_std_cnt; i++)
	{
		sprintf( name, "ss_pair_id_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		std_pair = &cust_mrgn->stdmgst[ i];

		sprintf( name, "ss_pair_id_%d", line);	Map_SetDataPtr( map, name, &std_pair->s_pair_id);
//		sprintf( name, "ss_clc_dsnc_%d", line);	Map_SetDataPtr( map, name, &std_pair->s_clc_dsnc);
//		sprintf( name, "sn_digit_%d", line);		Map_SetDataPtr( map, name, &std_pair->n_digit);
//		sprintf( name, "sd_digit_val_%d", line);	Map_SetDataPtr( map, name, &std_pair->d_digit_val);
//		sprintf( name, "sd_clc_unit_%d", line);	Map_SetDataPtr( map, name, &std_pair->d_clc_unit);
		sprintf( name, "sspt_bomg_%d", line);	Map_SetDataPtr( map, name, &std_pair->s_spt_bomg_dcd);
		sprintf( name, "sd_spt_bymg_%d", line);	Map_SetDataPtr( map, name, &std_pair->d_spt_bymg);
		sprintf( name, "sd_spt_slmg_%d", line);	Map_SetDataPtr( map, name, &std_pair->d_spt_slmg);
		sprintf( name, "sfwd_bomg_%d", line);	Map_SetDataPtr( map, name, &std_pair->s_fwd_bomg_dcd);
		sprintf( name, "sd_fwd_bymg_%d", line);	Map_SetDataPtr( map, name, &std_pair->d_fwd_bymg);
		sprintf( name, "sd_fwd_slmg_%d", line);	Map_SetDataPtr( map, name, &std_pair->d_fwd_slmg);

		line++;
	}

	while( 1)
	{
		sprintf( name, "ss_pair_id_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "ss_pair_id_%d", line);	Map_SetDataPtr( map, name, NULL);
//		sprintf( name, "ss_clc_dsnc_%d", line);	Map_SetDataPtr( map, name, NULL);
//		sprintf( name, "sn_digit_%d", line);		Map_SetDataPtr( map, name, NULL);
//		sprintf( name, "sd_digit_val_%d", line);	Map_SetDataPtr( map, name, NULL);
//		sprintf( name, "sd_clc_unit_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sspt_bomg_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sd_spt_bymg_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sd_spt_slmg_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sfwd_bomg_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sd_fwd_bymg_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sd_fwd_slmg_%d", line);	Map_SetDataPtr( map, name, NULL);
		line++;
	}

	/*
	Map_SetField( map, "name", myfunc, smq->aa);
	Map_SetDataPtr( map, "name", smq->aa);
	*/

	return 1;
	
}

/** ***************************************************************************
**  @func       int Mon_()
**  @param      MAP	*map
**  @return     성공    - +
**  @retval     실패    - -
**  @brief
**	time stemp convert
***************************************************************************** */
int Mon_TimeSet( MAP *map, MAP_FIELD *field)
{
	time_t	*cur_time;

	cur_time = field->ptr;
	time( cur_time);
	sprintf( field->data, "%s", TtoS( *cur_time));

	return 1;
}

/** ***************************************************************************
**  @func       int Mon_()
**  @param      MAP	*map
**  @return     성공    - +
**  @retval     실패    - -
**  @brief
**	time stemp convert
***************************************************************************** */
int Mon_TimeStr( MAP *map, MAP_FIELD *field)
{
	time_t	*cur_time;

	cur_time = field->ptr;
	sprintf( field->data, "%s", TtoS( *cur_time));

	return 1;
}


