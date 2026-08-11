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
#include <ctype.h>

#include "map.h"
#include "log.h"
#include "etc.h"

#include "blp.h"
#include "mon.h"

#include "main.h"
#include "task.h"

extern int	Continue;

typedef struct _mon_
{
    int     alarm;
    time_t  cur_time;
}   MON;

MON		_Mon = 
{
	0,
	0
};
MON		*Mon = &_Mon;


/** ***************************************************************************
**  @fu         int CmdInsert( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int Mon_Main( BLP *blp, char *map_name)
{
	int			id;
	int			cnt = 0;
	char		*title;
	MAP			*map;
	MAP_MENU	*menu;
	MAP_FIELD	*field_ptr;
	char		name[ 512];

	int			field, line;
	int			start_pos;

	if( map_name != NULL)
	{
		sprintf( Param->map_name, "%s", map_name);
	}

	map = Map_Open( Param->map_name);
	if( map == NULL)
	{
		LogMsg( "Map_Open error. name=[%s]", Param->map_name);
		goto error;
	}


	Mon_MainInit( map, blp);

	menu = Map_MenuOpen( map, '1');
	if( menu == NULL) goto error_1;

	Map_DisplayMap( map);
	Map_Message( map, "loop start");

	while( Continue)
	{
		/* 종목 추가 check */
		if( blp->map->tbl[ 0].id != cnt) 
		{
			Mon_MainInit( map, blp);
			Map_MenuScan( map, menu);
			cnt = blp->map->tbl[ 0].id;
			continue;
		}

		Map_CursorOff( map);
		Map_DisplayField( map);
		Map_CursorOn( map);
		id = Map_Menu( map, menu, Param->timeout);
		field =  id / 1000;
		line  =  id % 1000;

		switch( id)
		{
			case 0:		/* timeout */
				continue;
			case -1:	/* stop monitor */
				break;
			default:
				if( field == 201 || field == 202)
				{
					sprintf( name, "sis_stg_%d", line);
					field_ptr = Map_GetFieldPtr( map, name);
					Mon_ProcessFileVi( map, field_ptr->data, menu->key, field);
					Map_DisplayMap( map);
					continue;
				}
				else
				{
					Map_Message( map, "id=[%d] field=[%d]", id, field);
					Mon_Hoga( map, blp, line +1);
					Map_DisplayMap( map);
					continue;
				}
		}
		break;
	}

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
int Mon_MainInit( MAP *map, BLP *blp)
{
	int				i, line = 0;
	int				cnt;
	char			name[ 32];
	void			*ptr;
	BLP_TBL	*tbl = &blp->map->tbl[ 0];

	cnt = tbl->id;		/* 첫번째 테이블의 id가 종목 갯수 */

	Map_SetField( map, "time",	Mon_TimeSet, &Mon->cur_time);

    /************************************************/
    /* Blp Index                                    */
    /************************************************/
	for( i = 0; i < cnt; i++)
	{
		sprintf( name, "sis_no_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		tbl = &blp->map->tbl[ i +1];	/* 첫번째는 안씀 */
		if(  tbl->item_code[ 0] == 0) continue;
		sprintf( name, "sis_no_%d", line);		Map_SetDataPtr( map, name, &tbl->id);
		sprintf( name, "sis_pos_%d", line);		Map_SetDataPtr( map, name, &tbl->item_pos);
		sprintf( name, "sis_mode_%d", line);	Map_SetField( map, name, Mon_TblMode, tbl);
		sprintf( name, "sis_stat_%d", line);	Map_SetField( map, name, Mon_TblStat, tbl);
		sprintf( name, "sis_cnt_%d", line);		Map_SetDataPtr( map, name, &tbl->proc_cnt);
		sprintf( name, "sis_seq_%d", line);		Map_SetDataPtr( map, name, &tbl->seq_no);
		sprintf( name, "sis_bi_%d", line);		Map_SetDataPtr( map, name, &tbl->board_id);
		sprintf( name, "sis_item_%d", line);	Map_SetDataPtr( map, name, &tbl->item_code);
		sprintf( name, "sis_time_%d", line);	Map_SetField  ( map, name, Mon_TimeProc, &tbl->proc_time);
		sprintf( name, "sis_ab_%d", line);	    Map_SetDataPtr( map, name, &tbl->ask_base);
		sprintf( name, "sis_bb_%d", line);	    Map_SetDataPtr( map, name, &tbl->bid_base);
		sprintf( name, "sis_bb_%d", line);	    Map_SetDataPtr( map, name, &tbl->bid_base);
		sprintf( name, "sis_stg_%d", line);	    Map_SetDataPtr( map, name, &tbl->stg_id);
		line++;
	}
	while( 1)
	{
		sprintf( name, "sis_no_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "sis_no_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sis_pos_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sis_cnt_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sis_seq_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sis_bi_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sis_item_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sis_time_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sis_ab_%d", line);	    Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sis_bb_%d", line);	    Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sis_stg_%d", line);	    Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sis_t_%d", line);	    Map_SetDataPtr( map, name, NULL);
		sprintf( name, "sis_v_%d", line);	    Map_SetDataPtr( map, name, NULL);
		line++;
	}
	return 1;
}


/** ***************************************************************************
**  @fu         int CmdInsert( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int Mon_Hoga( MAP *main_map, BLP *blp, int tbl_no)
{
	int			id;
	char		*title;
	char		map_name[ 512];
	MAP			*map;
	MAP_MENU	*menu;
	MAP_FIELD	*field_ptr;

	int			field, line;
	int			start_pos;
	int			hedge_cnt = 0;
	BLP_TBL		*tbl = &blp->map->tbl[ tbl_no];

	sprintf( map_name, "%s", "hoga.map");
	map = Map_Open( map_name);
	if( map == NULL)
	{
		LogMsg( "Map_Open error. name=[%s]", map_name);
		goto error;
	}

	Mon_HogaInit( map, blp, tbl_no);

	menu = Map_MenuOpen( map, '1');
	if( menu == NULL) goto error_1;

	Map_DisplayMap( map);

	while( Continue)
	{
		Mon_HogaProc( map, blp, tbl_no);

		Map_CursorOff( map);
		Map_DisplayField( map);
		Map_CursorOn( map);
		id = Map_Menu( map, menu, Param->timeout);
		field =  id / 1000;
		line  =  id % 1000;
		if( toupper( menu->key) == 'R')
		{
			Mon_MainInit( map, blp);
			Map_DisplayMap( map);
			continue;
		}
		if( hedge_cnt != tbl->hedge_cnt)
		{
			Mon_HogaInit( map, blp, tbl_no);
			Map_MenuScan( map, menu);
			hedge_cnt = tbl->hedge_cnt;
			Map_DisplayMap( map);
			continue;
		}

		switch( id)
		{
			case 0:		/* timeout */
				continue;
			case -1:	/* stop monitor */
				break;
			case 300:	/* tail */
					field_ptr = Map_GetFieldPtr( map, "tbl_stg_id");
					Mon_ProcessFileVi( map, field_ptr->data, menu->key, 201);
					Map_DisplayMap( map);
					continue;
			case 301:	/* vim */
					field_ptr = Map_GetFieldPtr( map, "tbl_stg_id");
					Mon_ProcessFileVi( map, field_ptr->data, menu->key, 202);
					Map_DisplayMap( map);
					continue;
			default:
				Mon_Jang( map, blp, tbl, line +1);
				Map_DisplayMap( map);
				continue;
		}
		break;
	}

	Map_CursorOn( map);
	Map_Delete( map);
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
int Mon_HogaProc( MAP *map, BLP *blp, int tbl_no)
{
	int				i, line = 0;
	char			name[ 32];
	void			*ptr;
	BLP_TBL			*tbl = &blp->map->tbl[ tbl_no];
	BLP_HOGA_REC	*rec;
	BLP_ORD			*ord;

	line = 0;
    /************************************************/
    /* new hoga table                               */
    /************************************************/
	for( i = 0; i < BLP_MAX_HOGA; i++)
	{
		sprintf( name, "new_ab_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		rec = &tbl->rec[ line];

		if( rec->lp_no > 0)
		{
			ord = &tbl->order[ rec->lp_no -1];
			sprintf( name, "new_ord_stat_%d", line);	Map_SetField(   map, name, Mon_HogaStat, ord);
		}
		else
		{
			sprintf( name, "new_ord_stat_%d", line);	Map_SetField(   map, name, Mon_HogaStat, NULL);
		}

		line++;
	}

	return 1;
}

/** ***************************************************************************
**  @func       int Mon_()
**  @param      MAP	*map
**  @return     성공    - +
**  @retval     실패    - -
**  @brief
**	모니터링맵 초기화
***************************************************************************** */
int Mon_HogaInit( MAP *map, BLP *blp, int tbl_no)
{
	int				i, line = 0;
	int				cnt;
	char			name[ 32];
	void			*ptr;
	BLP_TBL			*tbl = &blp->map->tbl[ tbl_no];
	BLP_HOGA_REC	*rec;
	BLP_ORD			*ord;
	BLP_HEDGE		*hdg;
	BLP_ARG			*arg = &tbl->arg;
	BLP_TIME		*arg_time;
	char			*str[ 10] = { "오전장", "오후장", "마감장", NULL, NULL };

	Map_SetField( map, "time",	Mon_TimeSet, &Mon->cur_time);

	Map_SetDataPtr( map, "tbl_id", 			&tbl->id);
	Map_SetDataPtr( map, "tbl_item_pos", 	&tbl->item_pos);
	Map_SetDataPtr( map, "tbl_item_code", 	&tbl->item_code);
	Map_SetDataPtr( map, "tbl_proc_cnt", 	&tbl->proc_cnt);
	Map_SetField  ( map, "tbl_proc_time",	Mon_TimeProc, &tbl->proc_time);
	Map_SetField  ( map, "tbl_mode",		Mon_TblMode, tbl);
	Map_SetField  ( map, "tbl_mode_time",	Mon_TimeProc, &tbl->mode_time);
	Map_SetField  ( map, "tbl_mode_next",	Mon_TblModeNext, tbl);
	Map_SetField  ( map, "tbl_stat",		Mon_TblStat, tbl);

	Map_SetDataPtr( map, "tbl_unit", 		&arg->ord_qanty_unit);
	Map_SetDataPtr( map, "tbl_dsp",			&arg->dspratio);
	Map_SetDataPtr( map, "tbl_tick", 		&arg->tick_unit);
	Map_SetDataPtr( map, "tbl_trdr", 		&arg->trdr_uno);
	Map_SetDataPtr( map, "tbl_acc",			&arg->account_no);
	Map_SetDataPtr( map, "tbl_jong",		&arg->clsng_prc);
	Map_SetDataPtr( map, "tbl_jong_yul",	&arg->clsng_yild);
	Map_SetDataPtr( map, "tbl_stg_id",		&tbl->stg_id);

	Map_SetField( map, "new_time", Mon_TimeProc, &tbl->sise.tv);
	Map_SetField( map, "old_time", Mon_TimeProc, &tbl->sise.tv_old);


	line = 0;
    /************************************************/
    /* time table                                   */
    /************************************************/
	for( i = 0; i < BLP_MAX_MK_STAT; i++)
	{
		sprintf( name, "time_mk_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		arg_time = &tbl->arg.mk_time[ i];

		sprintf( name, "time_stat_%d", line);		Map_SetField( map, name, Mon_MkStat, arg);
		sprintf( name, "time_mk_%d", line);			Map_PrintName( map, name, "%s", str[ i]);
		sprintf( name, "time_start_%d", line);		Map_SetField( map, name, Mon_TimeHour, &arg_time->start);
		sprintf( name, "time_end_%d", line);		Map_SetField( map, name, Mon_TimeHour, &arg_time->end);
		sprintf( name, "time_ed_%d", line);			Map_SetDataPtr( map, name, &arg_time->exe_delay);
		sprintf( name, "time_rw_%d", line);			Map_SetDataPtr( map, name, &arg_time->rev_wait);
		sprintf( name, "time_sl_%d", line);			Map_SetDataPtr( map, name, &arg_time->submit_limit);
		sprintf( name, "time_es_%d", line);			Map_SetDataPtr( map, name, &arg_time->end_stat);
		sprintf( name, "time_prc1_%d", line);		Map_SetDataPtr( map, name, &arg_time->sped_prc[ 0]);
		sprintf( name, "time_prc2_%d", line);		Map_SetDataPtr( map, name, &arg_time->sped_prc[ 1]);
		sprintf( name, "time_prc3_%d", line);		Map_SetDataPtr( map, name, &arg_time->sped_prc[ 2]);
		sprintf( name, "time_vol1_%d", line);		Map_SetDataPtr( map, name, &arg_time->ord_qanty[ 0]);
		sprintf( name, "time_vol2_%d", line);		Map_SetDataPtr( map, name, &arg_time->ord_qanty[ 1]);
		sprintf( name, "time_vol3_%d", line);		Map_SetDataPtr( map, name, &arg_time->ord_qanty[ 2]);

		line++;
	}

	line = 0;
    /************************************************/
    /* new hoga table                               */
    /************************************************/
	for( i = 0; i < BLP_MAX_HOGA; i++)
	{
		sprintf( name, "new_ab_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		rec = &tbl->rec[ line];

		sprintf( name, "new_ab_%d", line);			Map_SetDataPtr( map, name, &rec->ab);
		sprintf( name, "new_no_%d", line);			Map_SetDataPtr( map, name, &rec->no);
		sprintf( name, "new_ho_%d", line);			Map_SetDataPtr( map, name, &rec->ho);
		sprintf( name, "new_s_ho_%d", line);		Map_SetDataPtr( map, name, &rec->s_ho);
		sprintf( name, "new_price_%d", line);		Map_SetDataPtr( map, name, &rec->price);
		sprintf( name, "new_volume_%d", line);		Map_SetDataPtr( map, name, &rec->volume);
		sprintf( name, "new_gap_%d", line);			Map_SetDataPtr( map, name, &rec->gap);
		sprintf( name, "new_gap_vol_%d", line);		Map_SetDataPtr( map, name, &rec->gap_vol);
		sprintf( name, "new_lp_no_%d", line);		Map_SetDataPtr( map, name, &rec->lp_no);

		if( rec->lp_no > 0)
		{
			ord = &tbl->order[ rec->lp_no -1];
			sprintf( name, "new_ord_stat_%d", line);	Map_SetField(   map, name, Mon_HogaStat, ord);
		}
		else
		{
			sprintf( name, "new_ord_stat_%d", line);	Map_SetField(   map, name, Mon_HogaStat, NULL);
		}

		line++;
	}
	while( 1)
	{
		sprintf( name, "new_ab_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "new_ab_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "new_no_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "new_ho_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "new_s_ho_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "new_price_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "new_volume_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "new_gap_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "new_gap_vol_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "new_lp_no_%d", line);		Map_SetDataPtr( map, name, NULL);

		/*
		sprintf( name, "old_ab_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_no_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_ho_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_s_ho_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_price_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_volume_%d", line);		Map_SetDataPtr( map, name, NULL);
		*/

		line++;
	}

#if 0
    /************************************************/
    /* old hoga                                     */
    /************************************************/
	line = 0;
	for( i = 0; i < BLP_MAX_HOGA; i++)
	{
		sprintf( name, "old_ab_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		old = &tbl->old[ line];

		sprintf( name, "old_ab_%d", line);			Map_SetDataPtr( map, name, &old->ab);
		sprintf( name, "old_no_%d", line);			Map_SetDataPtr( map, name, &old->no);
		sprintf( name, "old_ho_%d", line);			Map_SetDataPtr( map, name, &old->ho);
		sprintf( name, "old_s_ho_%d", line);		Map_SetDataPtr( map, name, &old->s_ho);
		sprintf( name, "old_price_%d", line);		Map_SetDataPtr( map, name, &old->price);
		sprintf( name, "old_volume_%d", line);		Map_SetDataPtr( map, name, &old->volume);
		line++;
	}
	while( 1)
	{
		sprintf( name, "old_ab_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "old_ab_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_no_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_ho_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_s_ho_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_price_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_volume_%d", line);		Map_SetDataPtr( map, name, NULL);
		line++;
	}
#endif

	line = 0;
    /************************************************/
    /* lp order                                     */
    /************************************************/
	for( i = 0; i < BLP_MAX_LP; i++)
	{
		sprintf( name, "ord_no_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		ord = &tbl->order[ i];
		sprintf( name, "ord_name_%d", line);		Map_PrintName( map, name, "%2d", i + 1);
		sprintf( name, "ord_stat_%d", line);		Map_SetField( map, name, Mon_OrdStat, &ord->ord_stat);
		sprintf( name, "ord_resp_%d", line);		Map_SetField( map, name, Mon_OrdResp, &ord->res_stat);
		sprintf( name, "ord_no_%d", line);			Map_SetDataPtr( map, name, &ord->ord_no);
		sprintf( name, "ord_org_no_%d", line);		Map_SetDataPtr( map, name, &ord->org_no);
		sprintf( name, "ord_ask_prc_%d", line);		Map_SetDataPtr( map, name, &ord->ask_prc);
		sprintf( name, "ord_ask_vol_%d", line);		Map_SetDataPtr( map, name, &ord->ask_vol);
		sprintf( name, "ord_ask_exe_%d", line);		Map_SetDataPtr( map, name, &ord->ask_exe_vol);
		sprintf( name, "ord_bid_prc_%d", line);		Map_SetDataPtr( map, name, &ord->bid_prc);
		sprintf( name, "ord_bid_vol_%d", line);		Map_SetDataPtr( map, name, &ord->bid_vol);
		sprintf( name, "ord_bid_exe_%d", line);		Map_SetDataPtr( map, name, &ord->bid_exe_vol);
		line++;
	}
	while( 1)
	{
		sprintf( name, "ord_no_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "ord_name_%d", line);		Map_PrintName( map, name, " ");
		sprintf( name, "ord_stat_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ord_no_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ord_ask_prc_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ord_ask_vol_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ord_ask_exe_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ord_bid_prc_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ord_bid_vol_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ord_bid_exe_%d", line);		Map_SetDataPtr( map, name, NULL);
		line++;
	}

	line = 0;
    /************************************************/
    /* hedge order                                  */
    /************************************************/
	for( i = 0; i < BLP_MAX_HEDGE; i++)
	{
		sprintf( name, "hdg_name_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;
		if( i >= tbl->hedge_cnt) break;

		hdg = &tbl->hedge[ i];
		sprintf( name, "hdg_name_%d", line);		Map_PrintName( map, name, "%2d", i + 1);
		sprintf( name, "hdg_stat_%d", line);		Map_SetField( map, name, Mon_OrdStat2, &hdg->ord_stat);
		sprintf( name, "hdg_resp_%d", line);		Map_SetField( map, name, Mon_OrdResp, &hdg->res_stat);
		sprintf( name, "hdg_ord_%d", line);			Map_SetDataPtr( map, name, &hdg->ord_no);
		sprintf( name, "hdg_org_%d", line);			Map_SetDataPtr( map, name, &hdg->org_no);
		sprintf( name, "hdg_side_%d", line);		Map_SetField( map, name, Mon_OrdSide, &hdg->side);
		sprintf( name, "hdg_prc_%d", line);			Map_SetDataPtr( map, name, &hdg->prc);
		sprintf( name, "hdg_vol_%d", line);			Map_SetDataPtr( map, name, &hdg->vol);
		sprintf( name, "hdg_exe_%d", line);			Map_SetDataPtr( map, name, &hdg->exe_vol);
		sprintf( name, "hdg_time_%d", line);		Map_SetField( map, name, Mon_TimeStr, &hdg->ord_time);
		sprintf( name, "hdg_gap_%d", line);			Map_SetDataPtr( map, name, &hdg->gap_time);
		line++;
	}
	while( 1)
	{
		sprintf( name, "hdg_name_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "hdg_name_%d", line);		Map_PrintName( map, name, " ");
		sprintf( name, "hdg_stat_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_resp_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_ord_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_org_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_side_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_prc_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_vol_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_exe_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_time_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_gap_%d", line);			Map_SetDataPtr( map, name, NULL);
		line++;
	}

	return 1;
}

/** ***************************************************************************
**  @fu         int CmdInsert( int argc, char *argv[])
**  @param      int argc - argument count
**  @param      char *argv[] - argument
**  @return     성공	- 1
**  @retval     실패	- -1
**  @brief      
**  함수 작성   
***************************************************************************** */
int Mon_Jang( MAP *main_map, BLP *blp, BLP_TBL *tbl, int tbl_no)
{
	int			id;
	char		*title;
	char		map_name[ 512];
	MAP			*map;
	MAP_MENU	*menu;

	int			field, line;
	int			start_pos;
	int			hedge_cnt = 0;

	sprintf( map_name, "%s", "jang.map");
	map = Map_Open( map_name);
	if( map == NULL)
	{
		LogMsg( "Map_Open error. name=[%s]", map_name);
		goto error;
	}

	Mon_JangInit( map, blp, tbl, tbl_no);

	menu = Map_MenuOpen( map, '1');
	if( menu == NULL) goto error_1;

	Map_DisplayMap( map);

	while( Continue)
	{
		Map_CursorOff( map);
		Map_DisplayField( map);
		Map_CursorOn( map);
		id = Map_Menu( map, menu, Param->timeout);
		field =  id / 1000;
		line  =  id % 1000;
		if( toupper( menu->key) == 'R')
		{
			Mon_MainInit( map, blp);
			Map_DisplayMap( map);
			continue;
		}

		switch( id)
		{
			case 0:		/* timeout */
				continue;
			case -1:	/* stop monitor */
				break;
			default:
				continue;
		}
		break;
	}

	Map_CursorOn( map);
	Map_Delete( map);
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
int Mon_JangInit( MAP *map, BLP *blp, BLP_TBL *tbl, int tbl_no)
{
	int				i, line = 0;
	int				cnt;
	char			name[ 32];
	void			*ptr;
	static int		jang_id;
	BLP_HOGA_REC	*rec;
	BLP_ORD			*ord;
	BLP_HEDGE		*hdg;
	BLP_ARG			*arg = &tbl->arg;
	BLP_TIME		*jang = &tbl->arg.mk_time[ tbl_no -1];
	char			*str[ 10] = { "오전장", "오후장", "마감장", NULL, NULL };


	jang_id = tbl_no;
	Map_PrintName( map, "jang_name", "[%s]", str[ tbl_no -1]);
	Map_SetField( map, "time",	Mon_TimeSet, &Mon->cur_time);

	Map_SetDataPtr( map, "jang_id", 		&tbl_no);
	Map_SetField  ( map, "jang_start",		Mon_TimeStr, &jang->start);
	Map_SetField  ( map, "jang_end",		Mon_TimeStr, &jang->end);
	Map_SetDataPtr( map, "jang_lp_time",	&jang->lp_time);
	Map_SetDataPtr( map, "jang_exe_delay",	&jang->exe_delay);
	Map_SetDataPtr( map, "jang_rev_wait",	&jang->rev_wait);
	Map_SetDataPtr( map, "jang_submit_limit",	&jang->submit_limit);
	Map_SetDataPtr( map, "jang_end_stat",	&jang->end_stat);

	Map_SetDataPtr( map, "jang_sped_prc1",	&jang->sped_prc[ 0]);
	Map_SetDataPtr( map, "jang_sped_prc2",	&jang->sped_prc[ 1]);
	Map_SetDataPtr( map, "jang_sped_prc3",	&jang->sped_prc[ 2]);

	Map_SetDataPtr( map, "jang_ord_qty1",	&jang->ord_qanty[ 0]);
	Map_SetDataPtr( map, "jang_ord_qty2",	&jang->ord_qanty[ 1]);
	Map_SetDataPtr( map, "jang_ord_qty3",	&jang->ord_qanty[ 2]);

	/*
	Map_SetDataPtr( map, "jang_item_pos", 	&tbl->item_pos);
	Map_SetDataPtr( map, "jang_item_code", 	&tbl->item_code);
	Map_SetDataPtr( map, "jang_proc_cnt", 	&tbl->proc_cnt);
	Map_SetField  ( map, "jang_mode",		Mon_TblMode, tbl);
	Map_SetField  ( map, "jang_mode_time",	Mon_TimeProc, &tbl->mode_time);
	Map_SetField  ( map, "jang_mode_next",	Mon_TblModeNext, tbl);
	Map_SetField  ( map, "jang_stat",		Mon_TblStat, tbl);

	Map_SetField( map, "new_time", Mon_TimeProc, &tbl->sise.tv);
	Map_SetField( map, "old_time", Mon_TimeProc, &tbl->sise.tv_old);
	*/


	line = 0;
    /************************************************/
    /* time table                                   */
    /************************************************/
#if 0
	for( i = 0; i < BLP_MAX_MK_STAT; i++)
	{
		sprintf( name, "time_mk_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		arg_time = &tbl->arg.mk_time[ i];

		sprintf( name, "time_stat_%d", line);		Map_SetField( map, name, Mon_MkStat, arg);
		sprintf( name, "time_mk_%d", line);			Map_PrintName( map, name, "%s", str[ i]);
		sprintf( name, "time_start_%d", line);		Map_SetField( map, name, Mon_TimeHour, &arg_time->start);
		sprintf( name, "time_end_%d", line);		Map_SetField( map, name, Mon_TimeHour, &arg_time->end);
		sprintf( name, "time_ed_%d", line);			Map_SetDataPtr( map, name, &arg_time->exe_delay);
		sprintf( name, "time_rw_%d", line);			Map_SetDataPtr( map, name, &arg_time->rev_wait);
		sprintf( name, "time_sl_%d", line);			Map_SetDataPtr( map, name, &arg_time->submit_limit);
		sprintf( name, "time_es_%d", line);			Map_SetDataPtr( map, name, &arg_time->end_stat);
		sprintf( name, "time_prc1_%d", line);		Map_SetDataPtr( map, name, &arg_time->sped_prc[ 0]);
		sprintf( name, "time_prc2_%d", line);		Map_SetDataPtr( map, name, &arg_time->sped_prc[ 1]);
		sprintf( name, "time_prc3_%d", line);		Map_SetDataPtr( map, name, &arg_time->sped_prc[ 2]);
		sprintf( name, "time_vol1_%d", line);		Map_SetDataPtr( map, name, &arg_time->ord_qanty[ 0]);
		sprintf( name, "time_vol2_%d", line);		Map_SetDataPtr( map, name, &arg_time->ord_qanty[ 1]);
		sprintf( name, "time_vol3_%d", line);		Map_SetDataPtr( map, name, &arg_time->ord_qanty[ 2]);

		line++;
	}

	line = 0;
    /************************************************/
    /* new hoga table                               */
    /************************************************/
	for( i = 0; i < BLP_MAX_HOGA; i++)
	{
		sprintf( name, "new_ab_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		rec = &tbl->rec[ line];

		sprintf( name, "new_ab_%d", line);			Map_SetDataPtr( map, name, &rec->ab);
		sprintf( name, "new_no_%d", line);			Map_SetDataPtr( map, name, &rec->no);
		sprintf( name, "new_ho_%d", line);			Map_SetDataPtr( map, name, &rec->ho);
		sprintf( name, "new_s_ho_%d", line);		Map_SetDataPtr( map, name, &rec->s_ho);
		sprintf( name, "new_price_%d", line);		Map_SetDataPtr( map, name, &rec->price);
		sprintf( name, "new_volume_%d", line);		Map_SetDataPtr( map, name, &rec->volume);
		sprintf( name, "new_gap_%d", line);			Map_SetDataPtr( map, name, &rec->gap);
		sprintf( name, "new_gap_vol_%d", line);		Map_SetDataPtr( map, name, &rec->gap_vol);
		sprintf( name, "new_lp_no_%d", line);		Map_SetDataPtr( map, name, &rec->lp_no);

		if( rec->lp_no > 0)
		{
			ord = &tbl->order[ rec->lp_no -1];
			sprintf( name, "new_ord_stat_%d", line);	Map_SetField(   map, name, Mon_HogaStat, ord);
		}
		else
		{
			sprintf( name, "new_ord_stat_%d", line);	Map_SetField(   map, name, Mon_HogaStat, NULL);
		}

		line++;
	}
	while( 1)
	{
		sprintf( name, "new_ab_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "new_ab_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "new_no_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "new_ho_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "new_s_ho_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "new_price_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "new_volume_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "new_gap_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "new_gap_vol_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "new_lp_no_%d", line);		Map_SetDataPtr( map, name, NULL);

		/*
		sprintf( name, "old_ab_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_no_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_ho_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_s_ho_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_price_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_volume_%d", line);		Map_SetDataPtr( map, name, NULL);
		*/

		line++;
	}
#endif

#if 0
    /************************************************/
    /* old hoga                                     */
    /************************************************/
	line = 0;
	for( i = 0; i < BLP_MAX_HOGA; i++)
	{
		sprintf( name, "old_ab_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		old = &tbl->old[ line];

		sprintf( name, "old_ab_%d", line);			Map_SetDataPtr( map, name, &old->ab);
		sprintf( name, "old_no_%d", line);			Map_SetDataPtr( map, name, &old->no);
		sprintf( name, "old_ho_%d", line);			Map_SetDataPtr( map, name, &old->ho);
		sprintf( name, "old_s_ho_%d", line);		Map_SetDataPtr( map, name, &old->s_ho);
		sprintf( name, "old_price_%d", line);		Map_SetDataPtr( map, name, &old->price);
		sprintf( name, "old_volume_%d", line);		Map_SetDataPtr( map, name, &old->volume);
		line++;
	}
	while( 1)
	{
		sprintf( name, "old_ab_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "old_ab_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_no_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_ho_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_s_ho_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_price_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "old_volume_%d", line);		Map_SetDataPtr( map, name, NULL);
		line++;
	}

	line = 0;
    /************************************************/
    /* lp order                                     */
    /************************************************/
	for( i = 0; i < BLP_MAX_LP; i++)
	{
		sprintf( name, "ord_no_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		ord = &tbl->order[ i];
		sprintf( name, "ord_name_%d", line);		Map_PrintName( map, name, "%2d", i + 1);
		sprintf( name, "ord_stat_%d", line);		Map_SetField( map, name, Mon_OrdStat, &ord->ord_stat);
		sprintf( name, "ord_resp_%d", line);		Map_SetField( map, name, Mon_OrdResp, &ord->res_stat);
		sprintf( name, "ord_no_%d", line);			Map_SetDataPtr( map, name, &ord->ord_no);
		sprintf( name, "ord_org_no_%d", line);		Map_SetDataPtr( map, name, &ord->org_no);
		sprintf( name, "ord_ask_prc_%d", line);		Map_SetDataPtr( map, name, &ord->ask_prc);
		sprintf( name, "ord_ask_vol_%d", line);		Map_SetDataPtr( map, name, &ord->ask_vol);
		sprintf( name, "ord_ask_exe_%d", line);		Map_SetDataPtr( map, name, &ord->ask_exe_vol);
		sprintf( name, "ord_bid_prc_%d", line);		Map_SetDataPtr( map, name, &ord->bid_prc);
		sprintf( name, "ord_bid_vol_%d", line);		Map_SetDataPtr( map, name, &ord->bid_vol);
		sprintf( name, "ord_bid_exe_%d", line);		Map_SetDataPtr( map, name, &ord->bid_exe_vol);
		line++;
	}
	while( 1)
	{
		sprintf( name, "ord_no_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "ord_name_%d", line);		Map_PrintName( map, name, " ");
		sprintf( name, "ord_stat_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ord_no_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ord_ask_prc_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ord_ask_vol_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ord_ask_exe_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ord_bid_prc_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ord_bid_vol_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ord_bid_exe_%d", line);		Map_SetDataPtr( map, name, NULL);
		line++;
	}

	line = 0;
    /************************************************/
    /* hedge order                                  */
    /************************************************/
	for( i = 0; i < BLP_MAX_HEDGE; i++)
	{
		sprintf( name, "hdg_name_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;
		if( i >= tbl->hedge_cnt) break;

		hdg = &tbl->hedge[ i];
		sprintf( name, "hdg_name_%d", line);		Map_PrintName( map, name, "%2d", i + 1);
		sprintf( name, "hdg_stat_%d", line);		Map_SetField( map, name, Mon_OrdStat2, &hdg->ord_stat);
		sprintf( name, "hdg_resp_%d", line);		Map_SetField( map, name, Mon_OrdResp, &hdg->res_stat);
		sprintf( name, "hdg_ord_%d", line);			Map_SetDataPtr( map, name, &hdg->ord_no);
		sprintf( name, "hdg_org_%d", line);			Map_SetDataPtr( map, name, &hdg->org_no);
		sprintf( name, "hdg_side_%d", line);		Map_SetField( map, name, Mon_OrdSide, &hdg->side);
		sprintf( name, "hdg_prc_%d", line);			Map_SetDataPtr( map, name, &hdg->prc);
		sprintf( name, "hdg_vol_%d", line);			Map_SetDataPtr( map, name, &hdg->vol);
		sprintf( name, "hdg_exe_%d", line);			Map_SetDataPtr( map, name, &hdg->exe_vol);
		sprintf( name, "hdg_time_%d", line);		Map_SetField( map, name, Mon_TimeStr, &hdg->ord_time);
		sprintf( name, "hdg_gap_%d", line);			Map_SetDataPtr( map, name, &hdg->gap_time);
		line++;
	}
	while( 1)
	{
		sprintf( name, "hdg_name_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "hdg_name_%d", line);		Map_PrintName( map, name, " ");
		sprintf( name, "hdg_stat_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_resp_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_ord_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_org_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_side_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_prc_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_vol_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_exe_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_time_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "hdg_gap_%d", line);			Map_SetDataPtr( map, name, NULL);
		line++;
	}
#endif

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
int Mon_HogaStat( MAP *map, MAP_FIELD *field)
{
	BLP_ORD		*ord = field->ptr;
	char		*ord_stat[ 10] = { "    ", "주문", "정정", "취소", NULL, NULL };
	char		*res_stat[ 10] = { "    ", "응답", "체결", "거부", NULL, NULL };

	if( ord == NULL)
		sprintf( field->data, "%s", "                              ");
	else
	{
		if( field->id % 1000 < 17)
			sprintf( field->data, "%s%s %10.0f %10.0f", 
				ord_stat[ ord->ord_stat], res_stat[ ord->res_stat], ord->ask_vol, ord->ask_exe_vol);
		else
			sprintf( field->data, "%s%s %10.0f %10.0f", 
				ord_stat[ ord->ord_stat], res_stat[ ord->res_stat], ord->bid_vol, ord->bid_exe_vol);
	}

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
int Mon_MkStat( MAP *map, MAP_FIELD *field)
{
	BLP_ARG		*arg = field->ptr;
	BLP_TIME	*mk_time;

	// Map_Message( map, "field->id=[%d]", field->id);
	// sleep( 1);

	if( arg->mk_stat == 0)
	{
		sprintf( field->data, "%s", " ");
		return 1;
	}

	if( arg->mk_stat == ( field->id % 10 +1))	sprintf( field->data, "%s", "*");
	else										sprintf( field->data, "%s", " ");

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
int Mon_OrdStat( MAP *map, MAP_FIELD *field)
{
	int		*stat = field->ptr;
	char	*str[ 10] = { "    ", "주문", "정정", "취소", NULL, NULL };

	if( *stat < 0 || *stat > 10) return 0;
	sprintf( field->data, "%s", str[ *stat]);
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
int Mon_OrdStat2( MAP *map, MAP_FIELD *field)
{
	int		*stat = field->ptr;
	char	*str[ 10] = { "    ", "주문", "취소", NULL, NULL };

	if( *stat < 0 || *stat > 10) return 0;
	sprintf( field->data, "%s", str[ *stat]);
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
int Mon_OrdResp( MAP *map, MAP_FIELD *field)
{
	int		*stat = field->ptr;
	char	*str[ 10] = { "    ", "응답", "체결", "거부", NULL, NULL };

	if( *stat < 0 || *stat > 10) return 0;
	if( str[ *stat] != NULL)	sprintf( field->data, "%s", str[ *stat]);
	else						sprintf( field->data, "%s", "    ");

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
int Mon_OrdSide( MAP *map, MAP_FIELD *field)
{
	int		*stat = field->ptr;
	char	*str[ 10] = { "    ", "매도", "매수", NULL, NULL };

	if( *stat < 0 || *stat > 3) return 0;
	sprintf( field->data, "%s", str[ *stat]);

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
int Mon_TblMode( MAP *map, MAP_FIELD *field)
{
	char			*str[ 10] = { "대기모드", "전략수행", "취소모드", "햇지모드", "종료", NULL, NULL };
	BLP_TBL	*tbl = field->ptr;

	if( tbl->mode < 0 || tbl->mode > 10) 
	{
		return 0;
	}
	sprintf( field->data, "%s", str[ tbl->mode]);
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
int Mon_TblModeNext( MAP *map, MAP_FIELD *field)
{
	char			*str[ 10] = { "없음", "전략수행", "취소모드", "햇지모드", "종료", NULL, NULL };
	BLP_TBL	*tbl = field->ptr;

	if( tbl->mode_next < 0 || tbl->mode_next > 10) 
	{
		return 0;
	}
	sprintf( field->data, "%s", str[ tbl->mode_next]);
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
int Mon_TblStat( MAP *map, MAP_FIELD *field)
{
	char			*str[ 10] = { "대기", "주문", "확인", NULL, NULL };
	BLP_TBL	*tbl = field->ptr;

	if( tbl->stat < 0 || tbl->stat > 10) return 0;
	sprintf( field->data, "%s", str[ tbl->stat]);
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
int Mon_TimeHour( MAP *map, MAP_FIELD *field)
{
	time_t	*cur_time;
	char	time_buf[ 32];

	cur_time = field->ptr;
	sprintf( time_buf, "%s", TtoS( *cur_time));
	sprintf( field->data, "%s", &time_buf[ 11]);

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

/** ***************************************************************************
**  @func       int Mon_()
**  @param      MAP	*map
**  @return     성공    - +
**  @retval     실패    - -
**  @brief
**	time stemp convert
***************************************************************************** */
int Mon_TimeProc( MAP *map, MAP_FIELD *field)
{
	struct timeval	*tp = field->ptr;
	time_t			*cur_time;

	sprintf( field->data, "%s.%06d", TtoS( tp->tv_sec), tp->tv_usec);

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
int Mon_GapTime( MAP *map, MAP_FIELD *field)
{
	time_t	cur_time;
	time_t	*set_time;
	time_t	gap_time;


	time( &cur_time);
	set_time = field->ptr;

	gap_time = ( cur_time - *set_time);

	if( gap_time >= 10000)	sprintf( field->data, "%4d", 9999);
	else					sprintf( field->data, "%4d", ( int)gap_time);

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
int Mon_Current( MAP *map, MAP_FIELD *field)
{
#if 0
	BLP_INDEX	*index;
	BLPSISE		*sise;
	double		price = 0.0;
	int			point = 0;
	time_t		cur_time;

	index = field->ptr;

	if( !memcmp( field->name,      "SiCuBi", 6))	{ sise = &index->sise_curr; price = sise->bidprc; point = index->point; }
	else if( !memcmp( field->name, "SiCuOf", 6))	{ sise = &index->sise_curr; price = sise->askprc; point = index->point; }
	else if( !memcmp( field->name, "SiBaBi", 6))	{ sise = &index->sise_base; price = sise->bidprc; }
	else if( !memcmp( field->name, "SiBaOf", 6))	{ sise = &index->sise_base; price = sise->askprc; }
	else if( !memcmp( field->name, "SiCoBi", 6))	{ sise = &index->sise_cont; price = sise->bidprc; point = 2; }
	else if( !memcmp( field->name, "SiCoOf", 6))	{ sise = &index->sise_cont; price = sise->askprc; point = 2; }
	else 											return 0;

	if( price == 0.0D)	
	{
		sprintf( field->data, "         ");
		return 0;
	}
	if( point == 0)
	{
		sprintf( field->data, "%10.5f", price);
	}
	else
	{
		/*
		sprintf( field->data, "%10.5f", price);
		*/
		sprintf( field->data, "%*.*f", 10, point, price);
	}


	if( Param->color)
	{
		time( &cur_time);
		if( sise->ctime == 0)				
		{
			field->attr[ MAP_ATTR_FORE] = MAP_ATTR_WHITE;
			field->attr[ MAP_ATTR_BACK] = MAP_ATTR_BLACK;
			field->attr[ MAP_ATTR_BRIGHT] = '0';
			field->attr[ MAP_ATTR_BOLD] = '0';
		}
		else
		if( cur_time - sise->ctime < 2)		
		{
			field->attr[ MAP_ATTR_FORE] = MAP_ATTR_CYAN;
			field->attr[ MAP_ATTR_BACK] = MAP_ATTR_NONE;
			field->attr[ MAP_ATTR_BRIGHT] = '1';
			field->attr[ MAP_ATTR_BOLD] = '1';
		}
		else
		if( cur_time - sise->ctime < 5)		
		{
			field->attr[ MAP_ATTR_FORE] = MAP_ATTR_WHITE;
			field->attr[ MAP_ATTR_BACK] = MAP_ATTR_NONE;
			field->attr[ MAP_ATTR_BRIGHT] = '1';
			field->attr[ MAP_ATTR_BOLD] = '1';
		}
		else
		if( cur_time - sise->ctime < 10)		
		{
			field->attr[ MAP_ATTR_FORE] = MAP_ATTR_WHITE;
			field->attr[ MAP_ATTR_BACK] = MAP_ATTR_NONE;
			field->attr[ MAP_ATTR_BRIGHT] = '1';
			field->attr[ MAP_ATTR_BOLD] = '0';
		}
		else
		if( cur_time - sise->ctime < 30)	
		{
			field->attr[ MAP_ATTR_FORE] = MAP_ATTR_WHITE;
			field->attr[ MAP_ATTR_BACK] = MAP_ATTR_BLACK;
			field->attr[ MAP_ATTR_BRIGHT] = '0';
			field->attr[ MAP_ATTR_BOLD] = '0';
		}
		else								
		{
			if( sise->price_time != 0)
			{
				if( cur_time - sise->ctime >= sise->price_time)
				{
					field->attr[ MAP_ATTR_FORE] = MAP_ATTR_RED;
					field->attr[ MAP_ATTR_BACK] = MAP_ATTR_BLACK;
					field->attr[ MAP_ATTR_BRIGHT] = '0';
					field->attr[ MAP_ATTR_BOLD] = '0';
				}
			}
			else
			{
				field->attr[ MAP_ATTR_FORE] = MAP_ATTR_BLACK;
				field->attr[ MAP_ATTR_BACK] = MAP_ATTR_NONE;
				field->attr[ MAP_ATTR_BRIGHT] = '1';
				field->attr[ MAP_ATTR_BOLD] = '0';
			}
		}
	}

#endif
	return 1;
}

int Mon_ProcessFileVi( MAP *main_map, char *name, int key, int field)
{
	int			rtn;
	time_t		cur_time;
	char		file_name[ 512];
	char		cmd[ 1024];

	if( field == 201)
	{
		sprintf( file_name, "/fsfxwin/fep/st03/LOG/PA/00000000/%.2s_%smp", "pa", &name[ 2]);
		sprintf( cmd, "tail -100f %s", file_name);
	}
	else
	if( field == 202)
	{
		sprintf( file_name, "/fsfxwin/fep/st03/LOG/PA/00000000/%.2s_%smp", "pa", &name[ 2]);
		// sprintf( cmd, "less +G %s", file_name);
		sprintf( cmd, "vim %s", file_name);
	}

	MapEnd();
	rtn = system( cmd);
	if( rtn < 0)
	{
		Map_Message( main_map, "cmd return rtn=[%d]", rtn);
		sleep( 10);
	}
	MapInit();

	return 1;
}

#if 0
int Mon_ProcessFile( MAP *main_map, MAT *mat, int pos, int side)
{
	int			rtn;
	FILE		*fp;
	time_t		cur_time;
	char		file_name[ 512];
	char		map_name[ 512];
	MAT_RECORD	*rec;

	Map_Message( main_map, "pos=[%d]", pos);

	rec = &mat->map->rec[ pos];

	time( &cur_time);
	sprintf( file_name, "/tmp/mat_%d_%ld.dat", pos, cur_time);
	fp = fopen( file_name, "w+");
	if( fp == NULL)
	{
		LogErr( "file open error. name=[%s]", file_name);
		return -1;
	}

	ORDER_PrintFile( ( ORDER *)&rec->ord, fp);
	fclose( fp);

	sprintf( map_name, "%s/map/%s", getenv( "MAT_CFG"), "file.map");
	rtn = Map_FileEdit( map_name, file_name);
	if( rtn < 0)
	{
		return rtn;
	}
	remove( file_name);

	return 1;
}

int Map_FileEdit( char *map_name, char *file_name)
{
	int			rtn;
	MAP			*map;
	MAP_EDIT	*edit;
	MAP_FIELD	*field;
	FILE		*fp;

	char		*ptr;
	char		rec[ 512];
	char		name[ 32];
	int			line = 0, rec_sz;

	map = Map_Open( map_name);
	if( map == NULL)
	{
		LogCri( "Map_Open error. name=[%s]", map_name);
		return 0;
	}

	edit = Map_EditOpen( map, '1', NULL);
	if( edit == NULL)
	{
		LogCri( "Map_EditOpen error.");
		goto error;
	}

	fp = fopen( file_name, "r");
	if( fp == NULL)
	{
		LogErr( "file open error. name=[%s]", file_name);
		goto error;
	}
	LogDbg( "fp=[%p]", fp);

	while( 1)
	{
		sprintf( name, "line_%d", line);
		field = Map_GetFieldPtr( map, name);
		if( field == NULL) 
		{
			LogDbg( "End of field. line=[%d]", line);
			line--;
			break;
		}

		ptr = fgets( rec, 512, fp);
		if( ptr == NULL) 
		{
			LogDbg( "EOF line=[%d]", line);
			break;
		}

		rec_sz = strlen( rec);
		memcpy( field->data, rec, rec_sz -1);
		field->data[ rec_sz -1] = 0;

		line++;
		if( line >= 512) break;
	}

	LogDbg( "line=[%d]", line);

	Map_DisplayMap( map);

	while( Continue)
	{
		Map_DisplayField( map);
		rtn = Map_Edit( map, edit);
		Map_Message( map, "key=[%d]", rtn);
		switch( rtn)
		{
			case 0:
				continue;
			default:
				break;
		}
		break;
	}

	fclose( fp);
#if 0
	while( line >= 0)
	{
		LogDbg( "line=[%2d] ptr=[%p]", line, file_line[ line]);
		free( file_line[ line]);
		line--;
	}
	free( file_line);
#endif

	Map_EditClose( map, edit);
	Map_Close( map);
	return 1;

#if 0
	error_1:
		fclose( fp);
#endif
	error:
		Map_Close( map);
		return 1;
}
#endif


/** ***************************************************************************
**  @func       int Mon_()
**  @param      MAP	*map
**  @return     성공    - +
**  @retval     실패    - -
**  @brief
**	time stemp convert
***************************************************************************** */
#if 0
int Mon_IndexChk( MAP *map, MAP_FIELD *field)
{
	BLP_INDEX	*index;
	double		bid, ask;
	static int	cnt = 0;

	index = field->ptr;
	bid = AtoD( index->sise_curr.entry[ 0].MDEntry_Px, sizeof( index->sise_curr.entry[ 0].MDEntry_Px));
	ask = AtoD( index->sise_curr.entry[ 1].MDEntry_Px, sizeof( index->sise_curr.entry[ 1].MDEntry_Px));

	if( bid <= 0) return 0;
	if( ask <= 0) return 0;

	if( bid > ask)
	{
		sprintf( field->data, "%s", "X");
		Map_Message( map, "[%d] [%.*s] [%.*s][%.*s]", cnt,
			sizeof( index->sise_curr.Symb), &index->sise_curr.Symb,
			sizeof( index->sise_curr.entry[ 0].MDEntry_Px), index->sise_curr.entry[ 0].MDEntry_Px,
			sizeof( index->sise_curr.entry[ 1].MDEntry_Px), index->sise_curr.entry[ 1].MDEntry_Px);
		LogDbg( "[%d] [%.*s] [%.*s][%.*s]", cnt,
			sizeof( index->sise_curr.Symb), &index->sise_curr.Symb,
			sizeof( index->sise_curr.entry[ 0].MDEntry_Px), index->sise_curr.entry[ 0].MDEntry_Px,
			sizeof( index->sise_curr.entry[ 1].MDEntry_Px), index->sise_curr.entry[ 1].MDEntry_Px);
		LogDbg( "[%d] [%.*s] [%.*s][%.*s]", cnt,
			sizeof( index->sise_base.Symb), &index->sise_base.Symb,
			sizeof( index->sise_base.entry[ 0].MDEntry_Px), index->sise_base.entry[ 0].MDEntry_Px,
			sizeof( index->sise_base.entry[ 1].MDEntry_Px), index->sise_base.entry[ 1].MDEntry_Px);
		LogDbg( "[%d] [%.*s] [%.*s][%.*s]", cnt,
			sizeof( index->sise_cont.Symb), &index->sise_cont.Symb,
			sizeof( index->sise_cont.entry[ 0].MDEntry_Px), index->sise_cont.entry[ 0].MDEntry_Px,
			sizeof( index->sise_cont.entry[ 1].MDEntry_Px), index->sise_cont.entry[ 1].MDEntry_Px);
		LogRaw( "\n---curr------------------------------------------------------------------------\n");
		BLP_FX_QUOTE_T_Print( &index->sise_curr);
		LogRaw( "\n---base------------------------------------------------------------------------\n");
		BLP_FX_QUOTE_T_Print( &index->sise_base);
		LogRaw( "\n---cont------------------------------------------------------------------------\n");
		BLP_FX_QUOTE_T_Print( &index->sise_cont);
		LogRaw( "\n---------------------------------------------------------------------------\n");
		cnt++;
	}
	else
	{
		sprintf( field->data, "%s", "O");
	}

	return 1;
}
#endif

