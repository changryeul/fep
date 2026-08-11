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
#include "smq.h"

#include "mds.h"

#include "main.h"
#include "task.h"

extern int	Continue;
int			TnrNo = 0;			/* MdFold 테너 No */
int			QuotNo = 0;			/* MdQuot 테너 No */
extern char	TnrStr[10][ 4];

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
int Mon_Main()
{
	int			id;
	MAP			*map;
	MAP_MENU	*menu;

	int			field, line;

	map = Map_Open( Param->main_map);
	if( map == NULL)
	{
		LogMsg( "Map_Open error. name=[%s]", Param->main_map);
		goto error;
	}


	Mon_MainInit( map);

	menu = Map_MenuOpen( map, '1');
	if( menu == NULL) goto error_1;

	Map_DisplayMap( map);
	Map_Message( map, "loop start");

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
			Mon_MainInit( map);
			Map_DisplayMap( map);
			Map_Message( map, "Reload screen.");
			continue;
		}

		switch( id)
		{
			case 0: /* timeout */
				continue;
			case -1:	/* stop monitor */
				break;
			default:
				LogDbg( "call Mon_Market. map=[%p] line=[%d]", map, line);
				Mon_Market( map, line);
				Map_DisplayMap( map);
				Map_Message( map, " ");
				continue;
		}
		break;
	}

	Map_CursorOn( map);
	Map_Close( map);
	return 1;

	error_1:
		Map_Close( map);
	error:
		LogCri( "Mon_Main error. return 0.");
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
int Mon_MainInit( MAP *map)
{
	int			i, line = 0;
	char		name[ 32];
	void		*ptr;

	Map_SetField( map, "time",	Mon_TimeSet, &Mon->cur_time);

	for( i = 0; MdsMem[ i].name[ 0] != 0; i++)
	{
		sprintf( name, "market_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "market_%d", line);	Map_SetDataPtr( map, name, &MdsMem[ line].full_name);
		sprintf( name, "key_%d", line);		Map_SetDataPtr( map, name, &MdsMem[ line].key);
		line++;
	}
	while( 1)
	{
		sprintf( name, "market_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "market_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "key_%d", line);			Map_SetDataPtr( map, name, NULL);
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
int Mon_Market( MAP *main_map, int market_no)
{
	int			id, field, line;
	MAP			*map;
	MAP_MENU	*menu;

	TnrNo = 0;
	map = Map_Open( Param->market_map);
	if( map == NULL)
	{
		LogMsg( "Map_Open error. name=[%s]", Param->market_map);
		goto error;
	}

	Mon_MarketInit( map, market_no);

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

		switch( menu->key)
		{
			case 'r':
			case 'R':
				Mon_MarketInit( map, market_no);
				Map_DisplayMap( map);
				continue;
			case WIN_KEY_RIGHT:
				TnrNo++;
				if( TnrNo > 8) TnrNo = 8;
				Mon_MarketInit( map, market_no);
				break;
			case WIN_KEY_LEFT:
				TnrNo--;
				if( TnrNo < 0) TnrNo = 0;
				Mon_MarketInit( map, market_no);
				break;
		}

		switch( id)
		{
			case 0: /* timeout */
				continue;
			case -1:	/* stop monitor */
				break;
			default:
				Mon_MdFold( map, market_no, line);
				LogDbg( "line=[%d]", line);
				Mon_MarketInit( map, market_no);
				Map_DisplayMap( map);
				Map_DisplayField( map);
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
		LogCri( "Mon_Main error. return 0.");
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
int Mon_MarketInit( MAP *map, int market_no)
{
	int			i, line = 0;
	char		name[ 32];
	void		*ptr;
	MDARCH		*arch;
	MDFOLD		*fold;

	arch = MdsMem[ market_no].base;
	fold = MdsMem[ market_no].base + sizeof( MDARCH);
	
	Map_SetField( map, "time",	Mon_TimeSet, &Mon->cur_time);
	
#if 1
	Map_SetDataPtr( map, "exid"				, &arch->xchg.exid           );
	Map_SetDataPtr( map, "exnm"				, &arch->xchg.exnm           );
	Map_SetDataPtr( map, "excode"			, &arch->xchg.excode         );
	Map_SetDataPtr( map, "TZ"				, &arch->xchg.TZ             );
	Map_SetDataPtr( map, "maxcnt"			, &arch->xchg.maxcnt         );
	Map_SetDataPtr( map, "arbitrage"		, &arch->xchg.arbitrage      );
	Map_SetDataPtr( map, "sendclient_flag"	, &arch->xchg.sendclient_flag);
	Map_SetDataPtr( map, "db_pnum"			, &arch->xchg.db_pnum        );
	Map_SetDataPtr( map, "recv.name"		, &arch->xchg.recv.name      );
	Map_SetDataPtr( map, "recv.ipad"		, &arch->xchg.recv.ipad      );
	Map_SetDataPtr( map, "recv.port"		, &arch->xchg.recv.port      );
	Map_SetDataPtr( map, "apsnd.cust"		, &arch->xchg.apsnd.cust     );
	Map_SetDataPtr( map, "apsnd.cast"		, &arch->xchg.apsnd.cast     );
	Map_SetDataPtr( map, "apsnd.best"		, &arch->xchg.apsnd.best     );
	Map_SetDataPtr( map, "apsnd.neta"		, &arch->xchg.apsnd.neta     );
	Map_SetDataPtr( map, "apsnd.ipad"		, &arch->xchg.apsnd.ipad     );
	Map_SetDataPtr( map, "apsnd.port"		, &arch->xchg.apsnd.port     );
	Map_SetDataPtr( map, "quenm"			, &arch->xchg.quenm          );
	Map_SetDataPtr( map, "dirp"				, &arch->xchg.dirp           );
	Map_SetDataPtr( map, "llog"				, &arch->xchg.llog           );
	Map_SetDataPtr( map, "logf"				, &arch->xchg.logf           );
#endif

	Map_SetDataPtr( map, "sizefold"			, &arch->sizefold            );
	Map_SetField  ( map, "rtim"    			, Mon_TimeStr, &arch->rtim                );
	Map_SetDataPtr( map, "rsum"    			, &arch->rsum                );
	Map_SetDataPtr( map, "tymd"    			, &arch->tymd                );
	Map_SetDataPtr( map, "mrec"    			, &arch->mrec                );
	Map_SetDataPtr( map, "nrec"    			, &arch->nrec                );

	Map_SetDataPtr( map, "tnr_no", &TnrStr[ TnrNo]);

	// for( i = 0; fold[ i].symb[ 0] != 0; i++)
	for( i = 0; i < arch->nrec; i++)
	{
		sprintf( name, "symb_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "symb_%d", line);		Map_SetDataPtr( map, name, &fold[ line].symb);
		sprintf( name, "f_time_%d", line);		Map_SetField( 	map, name,	Mon_TimeMds, &fold[ line]);
//		sprintf( name, "kymd_%d", line);		Map_SetDataPtr( map, name, &fold[ line].kymd);
//		sprintf( name, "khms_%d", line);		Map_SetDataPtr( map, name, &fold[ line].khms);
		sprintf( name, "swap_bid_%d", line);	Map_SetDataPtr( map, name, &fold[ line].mdquot[ TnrNo].swap_bid_man);
		sprintf( name, "swap_ask_%d", line);	Map_SetDataPtr( map, name, &fold[ line].mdquot[ TnrNo].swap_ask_man);
		sprintf( name, "bid_%d", line);			Map_SetDataPtr( map, name, &fold[ line].mdquot[ TnrNo].bid.last);
		sprintf( name, "ask_%d", line);			Map_SetDataPtr( map, name, &fold[ line].mdquot[ TnrNo].ask.last);
		sprintf( name, "mid_%d", line);			Map_SetDataPtr( map, name, &fold[ line].mdquot[ TnrNo].mid.last);
		sprintf( name, "tick_seqn_%d", line);	Map_SetDataPtr( map, name, &fold[ line].mdquot[ TnrNo].tick_seqn);
		sprintf( name, "fillprc_%d", line);		Map_SetDataPtr( map, name, &fold[ line].fillprc);
		line++;
	}

	while( 1)
	{
		sprintf( name, "symb_%d", line);
		ptr = Map_GetFieldPtr( map, name);
		if( ptr == NULL) break;

		sprintf( name, "symb_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "kymd_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "khms_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "tnr_no_%d", line);		Map_SetDataPtr( map, name, NULL);
		sprintf( name, "swap_bid_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "swap_ask_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "bid_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "ask_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "mid_%d", line);			Map_SetDataPtr( map, name, NULL);
		sprintf( name, "tick_seqn_%d", line);	Map_SetDataPtr( map, name, NULL);
		sprintf( name, "fillprc_%d", line);		Map_SetDataPtr( map, name, NULL);

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
int Mon_MdFold( MAP *main_map, int market_no, int fold_no)
{
	int			id, field, line;
	MAP			*map;
	MAP_MENU	*menu;

	TnrNo = 0;
	QuotNo = 0;

	map = Map_Open( Param->fold_map);
	if( map == NULL)
	{
		LogMsg( "Map_Open error. name=[%s]", Param->fold_map);
		goto error;
	}

	Mon_MdFoldInit( map, market_no, fold_no, QuotNo);

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
		LogDbg( "id=[%d] key=[%d]", id, menu->key);

		switch( menu->key)
		{
			case 'r':
			case 'R':
				Mon_MdFoldInit( map, market_no, fold_no, QuotNo);
				Map_DisplayMap( map);
				continue;
			case WIN_KEY_ENTER:
				QuotNo = id % 10;
				Mon_MdFoldInit( map, market_no, fold_no, QuotNo);
				break;
#if 0
			case WIN_KEY_RIGHT:
				TnrNo++;
				if( TnrNo > 8) TnrNo = 8;
				Mon_MarketInit( map, market_no);
				break;
			case WIN_KEY_LEFT:
				TnrNo--;
				if( TnrNo < 0) TnrNo = 0;
				Mon_MarketInit( map, market_no);
				break;
#endif
		}

		switch( id)
		{
			case 0: /* timeout */
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
		LogCri( "Mon_Main error. return 0.");
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
int Mon_MdFoldInit( MAP *map, int market_no, int fold_no, int quot_no)
{
	int			i, line = 0;
	char		name[ 32];
	void		*ptr;
	MDARCH		*arch;
	MDFOLD		*fold;
	mdquot_t	*quot;

	arch = MdsMem[ market_no].base;
	fold = MdsMem[ market_no].base + sizeof( MDARCH) + ( sizeof( MDFOLD) * fold_no);
	quot = &fold->mdquot[ quot_no];

	LogDbg( "symb=[%s]", fold->symb);
	
	Map_SetField( map, "time",	Mon_TimeSet, &Mon->cur_time);
	
	Map_SetDataPtr( map, "f_symb"       , &fold->symb       );
	Map_SetDataPtr( map, "f_seqn"       , &fold->seqn       );
	Map_SetDataPtr( map, "f_tymd"       , &fold->tymd       );
	Map_SetDataPtr( map, "f_kymd"       , &fold->kymd       );
	Map_SetDataPtr( map, "f_khms"       , &fold->khms       );
	Map_SetDataPtr( map, "f_zdiv"       , &fold->zdiv       );
	Map_SetDataPtr( map, "f_custzdiv"   , &fold->custzdiv   );
	Map_SetDataPtr( map, "f_swapzdiv"   , &fold->swapzdiv   );
	Map_SetDataPtr( map, "f_mrktzdiv"   , &fold->mrktzdiv   );
	Map_SetDataPtr( map, "f_feed"       , &fold->feed       );
	Map_SetDataPtr( map, "f_trdf"       , &fold->trdf       );
	Map_SetDataPtr( map, "f_open_hms"   , &fold->open_hms   );
	Map_SetDataPtr( map, "f_crossrate"  , &fold->crossrate  );
	Map_SetDataPtr( map, "f_fillid"     , &fold->fillid     );
	Map_SetDataPtr( map, "f_fillseq"    , &fold->fillseq    );
	Map_SetDataPtr( map, "f_fillprc"    , &fold->fillprc    );
	Map_SetDataPtr( map, "f_fillqty"    , &fold->fillqty    );
	Map_SetDataPtr( map, "f_ks"         , &fold->ks         );
	Map_SetDataPtr( map, "f_close_hms"  , &fold->close_hms  );
	Map_SetDataPtr( map, "f_best_flag"  , &fold->best_flag  );
	Map_SetField( map, "f_ks_time_usd",	Mon_TimeStr, &fold->kswitch_time_usd);
	Map_SetField( map, "f_ks_time_etc",	Mon_TimeStr, &fold->kswitch_time_etc);

	
	Map_SetDataPtr( map, "swap_bid"      , &quot->swap_bid        );
	Map_SetDataPtr( map, "swap_ask"      , &quot->swap_ask        );
	Map_SetDataPtr( map, "swap_bid_man"  , &quot->swap_bid_man    );
	Map_SetDataPtr( map, "swap_ask_man"  , &quot->swap_ask_man    );

	Map_SetDataPtr( map, "tick_seqn"     , &quot->tick_seqn       );
	Map_SetDataPtr( map, "tick_key"      , &quot->tickkey         );

	Map_SetDataPtr( map, "tenor"         , &TnrStr[ quot_no]);
	
	Map_SetDataPtr( map, "bid.excode"    , &quot->bid.excode      );
	Map_SetDataPtr( map, "bid.open"      , &quot->bid.open        );
	Map_SetDataPtr( map, "bid.high"      , &quot->bid.high        );
	Map_SetDataPtr( map, "bid.lowp"      , &quot->bid.lowp        );
	Map_SetDataPtr( map, "bid.last"      , &quot->bid.last        );
	Map_SetDataPtr( map, "bid.lastvol"   , &quot->bid.lastvol     );
	Map_SetDataPtr( map, "bid.last_valid", &quot->bid.last_valid  );
	Map_SetDataPtr( map, "bid.base"      , &quot->bid.base        );
	Map_SetDataPtr( map, "bid.best"      , &quot->bid.best        );
	Map_SetDataPtr( map, "bid.bestvol"   , &quot->bid.bestvol     );
	Map_SetDataPtr( map, "bid.sign"      , &quot->bid.sign        );
	Map_SetDataPtr( map, "bid.diff"      , &quot->bid.diff        );
	Map_SetDataPtr( map, "bid.rate"      , &quot->bid.rate        );
	Map_SetDataPtr( map, "bid.dirf"      , &quot->bid.dirf        );
	Map_SetField(   map, "bid.open_tm"   , Mon_TimeQuotDbl, &quot->bid.open_tm     );
	Map_SetField(   map, "bid.high_tm"   , Mon_TimeQuotDbl, &quot->bid.high_tm     );
	Map_SetField(   map, "bid.lowp_tm"   , Mon_TimeQuotDbl, &quot->bid.lowp_tm     );
	Map_SetField(   map, "bid.last_tm"   , Mon_TimeQuotDbl, &quot->bid.last_tm     );

	Map_SetDataPtr( map, "ask.excode"    , &quot->ask.excode      );
	Map_SetDataPtr( map, "ask.open"      , &quot->ask.open        );
	Map_SetDataPtr( map, "ask.high"      , &quot->ask.high        );
	Map_SetDataPtr( map, "ask.lowp"      , &quot->ask.lowp        );
	Map_SetDataPtr( map, "ask.last"      , &quot->ask.last        );
	Map_SetDataPtr( map, "ask.lastvol"   , &quot->ask.lastvol     );
	Map_SetDataPtr( map, "ask.last_valid", &quot->ask.last_valid  );
	Map_SetDataPtr( map, "ask.base"      , &quot->ask.base        );
	Map_SetDataPtr( map, "ask.best"      , &quot->ask.best        );
	Map_SetDataPtr( map, "ask.bestvol"   , &quot->ask.bestvol     );
	Map_SetDataPtr( map, "ask.sign"      , &quot->ask.sign        );
	Map_SetDataPtr( map, "ask.diff"      , &quot->ask.diff        );
	Map_SetDataPtr( map, "ask.rate"      , &quot->ask.rate        );
	Map_SetDataPtr( map, "ask.dirf"      , &quot->ask.dirf        );
	Map_SetField(   map, "ask.open_tm"   , Mon_TimeQuotDbl, &quot->ask.open_tm     );
	Map_SetField(   map, "ask.high_tm"   , Mon_TimeQuotDbl, &quot->ask.high_tm     );
	Map_SetField(   map, "ask.lowp_tm"   , Mon_TimeQuotDbl, &quot->ask.lowp_tm     );
	Map_SetField(   map, "ask.last_tm"   , Mon_TimeQuotDbl, &quot->ask.last_tm     );

	Map_SetDataPtr( map, "mid.excode"    , &quot->mid.excode      );
	Map_SetDataPtr( map, "mid.open"      , &quot->mid.open        );
	Map_SetDataPtr( map, "mid.high"      , &quot->mid.high        );
	Map_SetDataPtr( map, "mid.lowp"      , &quot->mid.lowp        );
	Map_SetDataPtr( map, "mid.last"      , &quot->mid.last        );
	Map_SetDataPtr( map, "mid.lastvol"   , &quot->mid.lastvol     );
	Map_SetDataPtr( map, "mid.last_valid", &quot->mid.last_valid  );
	Map_SetDataPtr( map, "mid.base"      , &quot->mid.base        );
	Map_SetDataPtr( map, "mid.best"      , &quot->mid.best        );
	Map_SetDataPtr( map, "mid.bestvol"   , &quot->mid.bestvol     );
	Map_SetDataPtr( map, "mid.sign"      , &quot->mid.sign        );
	Map_SetDataPtr( map, "mid.diff"      , &quot->mid.diff        );
	Map_SetDataPtr( map, "mid.rate"      , &quot->mid.rate        );
	Map_SetDataPtr( map, "mid.dirf"      , &quot->mid.dirf        );
	Map_SetField(   map, "mid.open_tm"   , Mon_TimeQuotDbl, &quot->mid.open_tm     );
	Map_SetField(   map, "mid.high_tm"   , Mon_TimeQuotDbl, &quot->mid.high_tm     );
	Map_SetField(   map, "mid.lowp_tm"   , Mon_TimeQuotDbl, &quot->mid.lowp_tm     );
	Map_SetField(   map, "mid.last_tm"   , Mon_TimeQuotDbl, &quot->mid.last_tm     );

	
	Map_SetDataPtr( map, "c_exnm"      , &quot->mdintr.exnm           );
	Map_SetDataPtr( map, "c_symn"      , &quot->mdintr.symb           );
	Map_SetDataPtr( map, "c_kymd"      , &quot->mdintr.kymd           );
	Map_SetDataPtr( map, "c_khms"      , &quot->mdintr.khms           );
	Map_SetDataPtr( map, "c_tymd"      , &quot->mdintr.tymd           );
	Map_SetDataPtr( map, "c_seqn"      , &quot->mdintr.seqn           );
	Map_SetDataPtr( map, "c_tenor"     , &quot->mdintr.tenor          );
	Map_SetDataPtr( map, "c_tickkey"   , &quot->mdintr.tickkey        );
	Map_SetDataPtr( map, "c_tenor_idx" , &quot->mdintr.tenor_idx      );

	Map_SetDataPtr( map, "c_bid.open"      , &quot->mdintr.bid.open        );
	Map_SetDataPtr( map, "c_bid.high"      , &quot->mdintr.bid.high        );
	Map_SetDataPtr( map, "c_bid.lowp"      , &quot->mdintr.bid.lowp        );
	Map_SetDataPtr( map, "c_bid.last"      , &quot->mdintr.bid.clos        );
	Map_SetDataPtr( map, "c_bid.last_valid", &quot->mdintr.bid.last_valid  );
	Map_SetDataPtr( map, "c_bid.base"      , &quot->mdintr.bid.base        );
	Map_SetField(   map, "c_bid.open_tm"   , Mon_TimeQuotDbl, &quot->mdintr.bid.open_tm     );
	Map_SetField(   map, "c_bid.high_tm"   , Mon_TimeQuotDbl, &quot->mdintr.bid.high_tm     );
	Map_SetField(   map, "c_bid.lowp_tm"   , Mon_TimeQuotDbl, &quot->mdintr.bid.lowp_tm     );
	Map_SetField(   map, "c_bid.last_tm"   , Mon_TimeQuotDbl, &quot->mdintr.bid.clos_tm     );

	Map_SetDataPtr( map, "c_ask.open"      , &quot->mdintr.ask.open        );
	Map_SetDataPtr( map, "c_ask.high"      , &quot->mdintr.ask.high        );
	Map_SetDataPtr( map, "c_ask.lowp"      , &quot->mdintr.ask.lowp        );
	Map_SetDataPtr( map, "c_ask.last"      , &quot->mdintr.ask.clos        );
	Map_SetDataPtr( map, "c_ask.last_valid", &quot->mdintr.ask.last_valid  );
	Map_SetDataPtr( map, "c_ask.base"      , &quot->mdintr.ask.base        );
	Map_SetField(   map, "c_ask.open_tm"   , Mon_TimeQuotDbl, &quot->mdintr.ask.open_tm     );
	Map_SetField(   map, "c_ask.high_tm"   , Mon_TimeQuotDbl, &quot->mdintr.ask.high_tm     );
	Map_SetField(   map, "c_ask.lowp_tm"   , Mon_TimeQuotDbl, &quot->mdintr.ask.lowp_tm     );
	Map_SetField(   map, "c_ask.last_tm"   , Mon_TimeQuotDbl, &quot->mdintr.ask.clos_tm     );

	Map_SetDataPtr( map, "c_mid.open"      , &quot->mdintr.mid.open        );
	Map_SetDataPtr( map, "c_mid.high"      , &quot->mdintr.mid.high        );
	Map_SetDataPtr( map, "c_mid.lowp"      , &quot->mdintr.mid.lowp        );
	Map_SetDataPtr( map, "c_mid.last"      , &quot->mdintr.mid.clos        );
	Map_SetDataPtr( map, "c_mid.last_valid", &quot->mdintr.mid.last_valid  );
	Map_SetDataPtr( map, "c_mid.base"      , &quot->mdintr.mid.base        );
	Map_SetField(   map, "c_mid.open_tm"   , Mon_TimeQuotDbl, &quot->mdintr.mid.open_tm     );
	Map_SetField(   map, "c_mid.high_tm"   , Mon_TimeQuotDbl, &quot->mdintr.mid.high_tm     );
	Map_SetField(   map, "c_mid.lowp_tm"   , Mon_TimeQuotDbl, &quot->mdintr.mid.lowp_tm     );
	Map_SetField(   map, "c_mid.last_tm"   , Mon_TimeQuotDbl, &quot->mdintr.mid.clos_tm     );

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
int Mon_MdQuotInit( MAP *map, int market_no, int fold_no, int quot_no)
{
	int			i, line = 0;
	char		name[ 32];
	void		*ptr;
	MDARCH		*arch;
	MDFOLD		*fold;
	mdquot_t	*quot;

	LogDbg( "QuotInit [%d] [%d] [%d]", market_no, fold_no, quot_no);

	arch = MdsMem[ market_no].base;
	fold = MdsMem[ market_no].base + sizeof( MDARCH) + ( sizeof( MDFOLD) * fold_no);
	quot = &fold->mdquot[ quot_no];

	LogDbg( "symb=[%s]", fold->symb);
	
	Map_SetField( map, "time",	Mon_TimeSet, &Mon->cur_time);
	
	LogDbg( "bid.excode =[%.1s]", &quot->bid.excode);
	Map_SetDataPtr( map, "bid.excode"    , &quot->bid.excode      );
	Map_SetDataPtr( map, "bid.open"      , &quot->bid.open        );
	Map_SetDataPtr( map, "bid.high"      , &quot->bid.high        );
	Map_SetDataPtr( map, "bid.lowp"      , &quot->bid.lowp        );
	Map_SetDataPtr( map, "bid.last"      , &quot->bid.last        );

	Map_SetDataPtr( map, "bid.lastvol"   , &quot->bid.lastvol     );
	Map_SetDataPtr( map, "bid.last_valid", &quot->bid.last_valid  );
	Map_SetDataPtr( map, "bid.base"      , &quot->bid.base        );
	Map_SetDataPtr( map, "bid.best"      , &quot->bid.best        );
	Map_SetDataPtr( map, "bid.bestvol"   , &quot->bid.bestvol     );
	Map_SetDataPtr( map, "bid.sign"      , &quot->bid.sign        );
	Map_SetDataPtr( map, "bid.diff"      , &quot->bid.diff        );
	Map_SetDataPtr( map, "bid.rate"      , &quot->bid.rate        );
	Map_SetDataPtr( map, "bid.dirf"      , &quot->bid.dirf        );

	Map_SetField(   map, "bid.open_tm"   , Mon_TimeQuotDbl, &quot->bid.open_tm     );
	Map_SetField(   map, "bid.high_tm"   , Mon_TimeQuotDbl, &quot->bid.high_tm     );
	Map_SetField(   map, "bid.lowp_tm"   , Mon_TimeQuotDbl, &quot->bid.lowp_tm     );
	Map_SetField(   map, "bid.last_tm"   , Mon_TimeQuotDbl, &quot->bid.last_tm     );






	Map_SetField( map, "f_ks_time_usd",	Mon_TimeStr, &fold->kswitch_time_usd);
	Map_SetField( map, "f_ks_time_etc",	Mon_TimeStr, &fold->kswitch_time_etc);

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
int Mon_TimeMds( MAP *map, MAP_FIELD *field)
{
	MDFOLD		*fold;
	struct tm	tm_buf;
	int			msec;
	time_t		curr_time, check_time;

	fold = (MDFOLD *)field->ptr;

	tm_buf.tm_year  = fold->kymd/10000 - 1900;
	tm_buf.tm_mon   = (fold->kymd%10000)/100 -1;
	tm_buf.tm_mday  = fold->kymd%100;
	tm_buf.tm_hour  = fold->khms/10000000;
	tm_buf.tm_min   = (fold->khms%10000000)/100000;
	tm_buf.tm_sec   = (fold->khms%100000)/1000;
	msec         = fold->khms%1000;

	sprintf( field->data, "%04d/%02d/%02d-%02d:%02d:%02d.%03d", 
		tm_buf.tm_year + 1900, 
		tm_buf.tm_mon +1, 
		tm_buf.tm_mday, 
		tm_buf.tm_hour,
		tm_buf.tm_min,
		tm_buf.tm_sec,
		msec
		);

	check_time = mktime( &tm_buf);
	time( &curr_time);

	if( curr_time - check_time > 1)
	{
		field->attr[ MAP_ATTR_FORE] = MAP_ATTR_WHITE;
		field->attr[ MAP_ATTR_BACK] = MAP_ATTR_BLACK;
		field->attr[ MAP_ATTR_BRIGHT] = '0';
		field->attr[ MAP_ATTR_BOLD] = '0';
	}
	else
	{
		field->attr[ MAP_ATTR_FORE] = MAP_ATTR_WHITE;
		field->attr[ MAP_ATTR_BACK] = MAP_ATTR_NONE;
		field->attr[ MAP_ATTR_BRIGHT] = '1';
		field->attr[ MAP_ATTR_BOLD] = '0';
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
int Mon_TimeQuotDbl( MAP *map, MAP_FIELD *field)
{
	double	d;
	time_t	cur_time;
	char	str_time[ 32];

	d = *( double *)field->ptr;
	cur_time = ( time_t)( d);

	// cur_time = ( time_t)( *field->ptr / 1000000.0);
	sprintf( str_time, "%s", TtoS( cur_time));
	sprintf( field->data, "%s", &str_time[ 11]);

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
int Mon_TimeDbl( MAP *map, MAP_FIELD *field)
{
	double	d;
	time_t	cur_time;

	d = *( double *)field->ptr;
	cur_time = ( time_t)( d);

	// cur_time = ( time_t)( *field->ptr / 1000000.0);
	sprintf( field->data, "%s", TtoS( cur_time));

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


