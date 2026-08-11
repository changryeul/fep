#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef CALC_H
#define	CALC_H	1

#include "log.h"

typedef struct _calc_
{
	char		**ts;					/* token stack */
	int			ts_cnt;

	char		**os;					/* operator stack */
	int			os_cnt;

	char		*input;
	int			i_cnt;
	int			i_sz;

	char		word[ 512];
	int			w_cnt;
}	CALC;

typedef struct _calc_operator_
{
	char	oper[ 4];					/* operator */
	int		s_prio;						/* start priority 0:low */
	int		e_prio;						/* end priority 0:low */
	int		(*act)( CALC *cal, char *oper);
}	CALC_OPER;

#endif /* CALC_H */

/***** Module : calc.c *****/
CALC*       Calc_Open();
int         Calc_Close( CALC *cal);
int         Calc_Process( CALC *cal, char *rec);
int         Calc_GetChar( CALC *cal, const char *call);
int         Calc_PutWord( CALC *cal, int c, const char *call);
int         Calc_GetWord( CALC *cal, char *data, int sz, const char *call);
int         Calc_PutToken( CALC *cal, char *data, int sz, const char *call);
int         Calc_GetToken( CALC *cal, char *rec, int sz, const char *call);
int         Calc_PutOper( CALC *cal, char *oper, int sz, const char *call);
int         Calc_GetOper( CALC *cal, char *rec, int sz, const char *call);
int         Calc_CheckOper( CALC *cal, char *oper1, char *oper2);
int         Calc_CalcOper( CALC *cal, char *oper);
int         Calc_ActPlus( CALC *cal, char *oper);
int         Calc_ActMius( CALC *cal, char *oper);
int         Calc_ActMult( CALC *cal, char *oper);
int         Calc_ActDevi( CALC *cal, char *oper);
int         Calc_ActOpen( CALC *cal, char *oper);
int         Calc_ActClos( CALC *cal, char *oper);
int         Calc_ActEqul( CALC *cal, char *oper);
int         Calc_Start( CALC *cal);
int         Calc_ProcOper( CALC *cal);
int         Calc_Print( CALC *cal);

