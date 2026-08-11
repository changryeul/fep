/* ***************************************************************************
**  @file       slp.h
**  @date       2024/02/01
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  double linked list position manager library
**  double linked list를 포지션으로 관리 - 공유 메모리용
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "cfg.h"
#include "etc.h"
#include "slp.h"

extern int	Continue;

/** ***************************************************************************
**  @fn         SLP *Slp_()
**  @param      none
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp_sub library
***************************************************************************** */
int Slp_LoadCurr( SLP *slp, char *cfg_name)
{
	int		i;
	char	*ptr;
	CFG		*cfg;

	SLP_CURR	*curr;

	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", cfg_name);
		goto error;
	}

	Cfg_Set( cfg, "CurrentInt");

	Slp_Lock( slp);

	for( i = 0; i < SLP_MAX_CURR; i++)
	{
		curr = &slp->status->curr[ i];
		memset( curr, 0x00, sizeof( SLP_CURR));
		curr->num = -1;
	}

	ptr = ( char *)Cfg_GetFirstNamePtr( cfg, "CurrentInt");
	while( ptr != NULL)
	{
		curr = Slp_LoadCurrSub( slp, ptr);
		if( curr == NULL)
		{
			LogCri( "Slp_LoadCurrentSub error.");
			goto error_1;
		}
		if( ( curr->num < 0) || ( curr->num >= SLP_MAX_CURR)) 
		{
			LogCri( "Current load error. num=[%d] str=[%s]", curr->num, curr->str);
		}
		else
		{
			memcpy( &slp->status->curr[ curr->num], curr, sizeof( SLP_CURR));
		}

		ptr = Cfg_GetNextNamePtr( cfg, "CurrentInt");
	}

	Slp_Unlock( slp);

	Cfg_Close( cfg);
	return 1;

	error_1:
		Slp_Unlock( slp);
		Cfg_Close( cfg);
	error:
		return -1;
}

/** ***************************************************************************
**  @fn         SLP *Slp_()
**  @param      none
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp_sub library
***************************************************************************** */
SLP_CURR *Slp_LoadCurrSub( SLP *slp, char *line)
{
	int			stat = 0;
	char		rec[ 512];
	char		*token = ":";
	char		*ptr = rec, *end = ptr;

	static SLP_CURR	_curr, *curr = &_curr;

	memcpy( rec, line, strlen( line) +1);

	while( end != NULL)
	{
		end = strpbrk( ptr, token);
		if( end) *end = 0;

		switch( stat)
		{
			case 0:		/* num */
				curr->num = atoi( ptr);
				LogRaw( "[%2d]", curr->num);
				break;
			case 1:		/* str */
				memcpy( curr->str, ptr, strlen( ptr) +1);
				TRIM( curr->str);
				LogRaw( "[%s]", curr->str);
				break;
			case 2:		/* point */
				curr->point = atoi( ptr);
				LogRaw( "[%1d]", curr->point);
				break;
			case 3:		/* comment */
				memcpy( curr->comment, ptr, strlen( ptr) +1);
				TRIM( curr->comment);
				LogRaw( "[%s]", curr->comment);
				break;
			default:
				break;
		}
		ptr = end +1;
		stat++;
	}
	LogRaw( "\n");

	return curr;
}

/** ***************************************************************************
**  @fn         SLP *Slp_()
**  @param      none
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  통화 string(3 byte)을 통화 int로 변환
***************************************************************************** */
int Slp_GetCurrInt( SLP *slp, char *str)
{
	int			num;
	SLP_CURR	*curr;

	for( num = 0; num < SLP_MAX_CURR; num++)
	{
		curr = &slp->status->curr[ num];
		if( curr->num < 0) break;
		if( !memcmp( curr->str, str, 3))
		{
			return num;
		}
	}

	return -1;
}

/** ***************************************************************************
**  @fn         SLP *Slp_()
**  @param      none
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp_sub library
***************************************************************************** */
int Slp_LoadPair( SLP *slp, char *cfg_name)
{
	int		i;
	char	*ptr;
	CFG		*cfg;

	SLP_PAIR	*pair;

	cfg = Cfg_Open( cfg_name);
	if( cfg == NULL)
	{
		LogCri( "Cfg_Open error. name=[%s]", cfg_name);
		goto error;
	}

	Cfg_Set( cfg, "CurrentPair");

	Slp_Lock( slp);

	for( i = 0; i < SLP_MAX_PAIR; i++)
	{
		pair = &SLP_GET_IDX( slp, i)->pair;
		memset( pair, 0x00, sizeof( SLP_PAIR));
		pair->num = -1;
	}
	slp->status->idx_cnt = 0;

	ptr = ( char *)Cfg_GetFirstNamePtr( cfg, "CurrentPair");
	while( ptr != NULL)
	{
		pair = Slp_LoadPairSub( slp, ptr);
		if( pair == NULL)
		{
			LogCri( "Slp_LoadCurrSub error.");
			goto error_1;
		}
		if( ( pair->num < 0) || ( pair->num >= SLP_MAX_PAIR)) 
		{
			LogCri( "Current load error. num=[%d:%d] str=[%s]", pair->base, pair->cont, pair->symbol);
		}
		else
		{
			memcpy( &SLP_GET_IDX( slp, pair->num)->pair, pair, sizeof( SLP_PAIR));
		}
		slp->status->idx_cnt++;

		ptr = Cfg_GetNextNamePtr( cfg, "CurrentPair");
	}

	Slp_Unlock( slp);

	Cfg_Close( cfg);
	return 1;

	error_1:
		Slp_Unlock( slp);
		Cfg_Close( cfg);
	error:
		return -1;
}

/** ***************************************************************************
**  @fn         SLP *Slp_()
**  @param      none
**  @return     SLP 구조체 pointer
**  @retval     NULL 실패
**  @exception
**  @remark
**  @brief
**  slp_sub library
***************************************************************************** */
SLP_PAIR *Slp_LoadPairSub( SLP *slp, char *line)
{
	int			stat = 0;
	char		rec[ 512];
	char		*token = ":";
	char		*ptr = rec, *end = ptr;

	static SLP_PAIR	_pair, *pair = &_pair;

	memcpy( rec, line, strlen( line) +1);

	while( end != NULL)
	{
		end = strpbrk( ptr, token);
		if( end) *end = 0;

		switch( stat)
		{
			case 0:		/* symbol */
				memcpy( pair->symbol, ptr, strlen( ptr) +1);
				LogRaw( "[%s]", pair->symbol);
				break;
			case 1:		/* point */
				pair->point = atoi( ptr);
				LogRaw( "[%1d]", pair->point);
				break;
			case 2:		/* unit */
				pair->unit = atoi( ptr);
				LogRaw( "[%3d]", pair->unit);
				break;
			case 3:		/* num */
				pair->num = atoi( ptr) -1;
				LogRaw( "[%3d]", pair->num);
				break;
			case 4:		/* used */
				LogRaw( "[%s]", ptr);
				break;
			case 5:		/* comment */
				memcpy( pair->comment, ptr, strlen( ptr) +1);
				TRIM( pair->comment);
				LogRaw( "[%s]", pair->comment);
				break;
			default:
				break;
		}
		ptr = end +1;
		stat++;
	}
	pair->base = Slp_GetCurrInt( slp, &pair->symbol[ 0]);
	if( pair->base < 0) return NULL;
	pair->cont = Slp_GetCurrInt( slp, &pair->symbol[ 4]);
	if( pair->cont < 0) return NULL;
	LogRaw( "[%2d:%2d]", pair->base, pair->cont);
	LogRaw( "\n");

	return pair;
}

/** ***************************************************************************
**  @fn         int Proc_()
**  @param      int argc     - 프로그램 인수 갯수
**  @return     프로그램 수행 결과
**  @retval     음수 실패
**  @retval     양수 성공
**  @exception
**  @remark
**  @brief
**  convert ORDER to ORDER_SEND
***************************************************************************** */
int Slp_SiseConvert( SLP_SISE *dest, SLP_SISE *orig)
{
	memcpy( dest->type,			orig->type,			sizeof( dest->type));
	memcpy( dest->excode,		orig->excode,		sizeof( dest->excode));
	memcpy( dest->bidex,		orig->bidex,		sizeof( dest->bidex));
	memcpy( dest->askex,		orig->askex,		sizeof( dest->askex));
	memcpy( dest->symb,			orig->symb,			sizeof( dest->symb));
	memcpy( dest->id,			orig->id,			sizeof( dest->id));
	memcpy( dest->date,			orig->date,			sizeof( dest->date));
	memcpy( dest->time,			orig->time,			sizeof( dest->time));
	dest->usdbid		= orig->usdbid;
	dest->usdask		= orig->usdask;
	if( orig->bidprc != 0.0)	dest->bidprc		= orig->bidprc;
	if( orig->askprc != 0.0)	dest->askprc		= orig->askprc;
	dest->bidqty		= orig->bidqty;
	dest->askqty		= orig->askqty;
	dest->midprc		= orig->midprc;
	dest->fillprc		= orig->fillprc;
	dest->ctime			= orig->ctime;
	dest->price_time	= orig->price_time;

	return 1;
}

