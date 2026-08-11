#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "print.h"

/*** Module : print.c ***/
int	T_KRX_HEAD_Print( T_KRX_HEAD* ptr);
int	T_KRX_HEARTBEAT_Print( T_KRX_HEARTBEAT* ptr);
int	T_KRX_Q_LOGON_Print( T_KRX_Q_LOGON* ptr);
int	T_KRX_R_LOGON_Print( T_KRX_R_LOGON* ptr);
int	T_KRX_QR_OPEN_Print( T_KRX_QR_OPEN* ptr);
int	T_KRX_QR_OPEN_1_Print( T_KRX_QR_OPEN_1* ptr);
int	T_KRX_Q_RESEND_Print( T_KRX_Q_RESEND* ptr);
int	T_KRX_R_RESEND_Print( T_KRX_R_RESEND* ptr);
int	T_KRX_Q_SEQUENCE_Print( T_KRX_Q_SEQUENCE* ptr);
int	T_KRX_R_SEQUENCE_Print( T_KRX_R_SEQUENCE* ptr);
int	T_KRX_Q_LOGOUT_Print( T_KRX_Q_LOGOUT* ptr);
int	T_KRX_R_LOGOUT_Print( T_KRX_R_LOGOUT* ptr);
int	T_KRX_ORDER_Print( T_KRX_ORDER* ptr);
int	T_KRX_ORDER_FMT_Print( T_KRX_ORDER_FMT* ptr);
int	T_KRX_ORDER_400_Print( T_KRX_ORDER_400* ptr);
int	T_KRX_ORDER_RESP_Print( T_KRX_ORDER_RESP* ptr);
int	T_COMBINATION_ORDER_Print( T_COMBINATION_ORDER* ptr);
int	T_COMBINATION_ORDER_400_Print( T_COMBINATION_ORDER_400* ptr);
int	T_KILL_SWITCH_Print( T_KILL_SWITCH* ptr);
int	T_KILL_SWITCH_400_Print( T_KILL_SWITCH_400* ptr);
int	T_KRX_SETTLE_Print( T_KRX_SETTLE* ptr);
int	T_KRX_SETTLE_FMT_Print( T_KRX_SETTLE_FMT* ptr);
int	T_KRX_SETTLE_400_Print( T_KRX_SETTLE_400* ptr);
int	T_KRX_CONFIRM_Print( T_KRX_CONFIRM* ptr);
int	T_KRX_CONFIRM_FMT_Print( T_KRX_CONFIRM_FMT* ptr);
int	T_KRX_CONFIRM_400_Print( T_KRX_CONFIRM_400* ptr);
int	T_KILL_SWITCH_CONFIRM_Print( T_KILL_SWITCH_CONFIRM* ptr);
int	T_KILL_SWITCH_CONFIRM_400_Print( T_KILL_SWITCH_CONFIRM_400* ptr);
int	T_SETTLE_IF_END_Print( T_SETTLE_IF_END* ptr);
int	T_SETTLE_IF_END_400_Print( T_SETTLE_IF_END_400* ptr);
int	T_CUST_INFO_Print( T_CUST_INFO* ptr);
int	T_KRX_JANG_Print( T_KRX_JANG* ptr);
int	T_KRX_JANG_400_Print( T_KRX_JANG_400* ptr);
int	T_KRX_JANG_FMT_Print( T_KRX_JANG_FMT* ptr);
int	T_KRX_JANG_INFO_Print( T_KRX_JANG_INFO* ptr);
int	T_KRX_JANG_INFO_400_Print( T_KRX_JANG_INFO_400* ptr);
int	T_KRX_JANG_BASE_Print( T_KRX_JANG_BASE* ptr);
int	T_KRX_JANG_BASE_400_Print( T_KRX_JANG_BASE_400* ptr);
int	T_KRX_JANG_STOP_Print( T_KRX_JANG_STOP* ptr);
int	T_KRX_JANG_STOP_400_Print( T_KRX_JANG_STOP_400* ptr);
int	T_KRX_JANG_END_Print( T_KRX_JANG_END* ptr);
int	T_KRX_JANG_END_400_Print( T_KRX_JANG_END_400* ptr);
int	T_KRX_JANG_DIV_Print( T_KRX_JANG_DIV* ptr);
int	T_KRX_JANG_DIV_400_Print( T_KRX_JANG_DIV_400* ptr);
int	T_KRX_JANG_VI_Print( T_KRX_JANG_VI* ptr);
int	T_KRX_JANG_VI_400_Print( T_KRX_JANG_VI_400* ptr);
int	T_KRX_JANG_LMT_Print( T_KRX_JANG_LMT* ptr);
int	T_KRX_JANG_LMT_400_Print( T_KRX_JANG_LMT_400* ptr);
int	T_KRX_JANG_EXPN_Print( T_KRX_JANG_EXPN* ptr);
int	T_KRX_JANG_EXPN_400_Print( T_KRX_JANG_EXPN_400* ptr);
int	T_KRX_ORDER_FRGN_ADD_Print( T_KRX_ORDER_FRGN_ADD* ptr);
int	T_KRX_ORDER_FRGN_Print( T_KRX_ORDER_FRGN* ptr);
int	T_KRX_CONFIRM_FRGN_ADD_Print( T_KRX_CONFIRM_FRGN_ADD* ptr);

typedef struct _packet_print_table_
{
	int			no;
	int			pos;
	char		data[ 512];
	int			(* act)();
	char		comment[ 512];
	
}	PACKET_PRINT_TABLE;

PACKET_PRINT_TABLE	PacketPrintTable[] =
{
	{	1,	11,		"TTRMIP31309",			T_KRX_JANG_EXPN_Print,			"장운영"}, 
	{	1,	11,		"TTRMIP31308",			T_KRX_JANG_LMT_Print,			"장운영"}, 
	{	1,	11,		"TTRMIP31307",			T_KRX_JANG_VI_Print,			"장운영"}, 
	{	1,	11,		"TTRMIP31306",			T_KRX_JANG_DIV_Print,			"장운영"}, 
	{	1,	11,		"TTRMIP31304",			T_KRX_JANG_END_Print,			"장운영"}, 
	{	1,	11,		"TTRMIP31303",			T_KRX_JANG_STOP_Print,			"장운영"}, 
	{	1,	11,		"TTRMIP31302",			T_KRX_JANG_BASE_Print,			"장운영"}, 
	{	1,	11,		"TTRMIP32301",			T_KRX_JANG_INFO_Print,			"장운영"}, 
	{	1,	11,		"TTRMIP31301",			T_KRX_JANG_Print,				"장운영"}, 
	{	1,	11,		"TCHEDP",				T_SETTLE_IF_END_Print,			"SETTLE END"}, 
	{	1,	11,		"TTRKOR",				T_KILL_SWITCH_CONFIRM_Print,	"SETTLE Kill Switch"}, 
	{	1,	11,		"TTRODP",				T_KRX_CONFIRM_Print,			"CONFIRM"}, 
	{	1,	11,		"TTRTDP",				T_KRX_SETTLE_Print,				"SETTLE"}, 
	{	1,	11,		"TCHKDR1",				T_KILL_SWITCH_Print,			"ORDER Kill Switch"}, 
	{	1,	11,		"TCHCDR1",				T_COMBINATION_ORDER_Print,		"ORDER Combination"}, 
	{	1,	11,		"TCHODR1",				T_KRX_ORDER_Print,				"ORDER"}, 
	{	1,	14,		"TCHODR00000",			T_KRX_ORDER_FMT_Print,			"HEAD + ORDER"}, 
	{	1,	14,		"SCHLIQ00000",			T_KRX_Q_LOGON_Print,			"HEAD + LOGON"}, 
	{	1,	14,		"SCHLIR00000",			T_KRX_R_LOGON_Print,			"HEAD + LOGON 응답"}, 
	{	1,	14,		"SCHOPQ00000",			T_KRX_QR_OPEN_Print,			"HEAD + 업무개시"}, 
	{	1,	14,		"SCHOPQ10000",			T_KRX_QR_OPEN_1_Print,			"HEAD + 업무개시 1"}, 
	{	1,	14,		"SCHOPR00000",			T_KRX_QR_OPEN_Print,			"HEAD + 업무개시"}, 
	{	1,	14,		"SCHOPR10000",			T_KRX_QR_OPEN_1_Print,			"HEAD + 업무개시 1"}, 
	{	1,	14,		"SCHHEQ00000",			T_KRX_HEARTBEAT_Print,			"HEAD + 회선시험"}, 
	{	1,	14,		"SCHHER00000",			T_KRX_HEARTBEAT_Print,			"HEAD + 회선시험 응답"}, 
	{	1,	14,		"SCHLOQ00000",			T_KRX_Q_LOGOUT_Print,			"HEAD + LOGOUT"}, 
	{	1,	14,		"SCHLOR00000",			T_KRX_R_LOGOUT_Print,			"HEAD + LOGOUT 응답"}, 
	{	1,	0,		"KMAPv2.0",				T_KRX_HEAD_Print,				"HEAD"}, 
	{ 	-1,	-1,		"",						NULL,							""}
};


PacketPrint( char *data)
{
	int					rtn;
	char				*ptr, buf[ 512];
	PACKET_PRINT_TABLE	*pp_tbl = &PacketPrintTable[ 0];
	PACKET_PRINT_TABLE	*pp_tbl_idx[ 512];
	int					idx = 0, pos;


	while( 1)
	{
		if( pp_tbl->no < 0) break;
		ptr = strstr( data, pp_tbl->data);
		if( ptr != NULL)
		{
			if( ptr - pp_tbl->pos < data)
			{
				pp_tbl++;
				continue;
			}
			pp_tbl_idx[ idx++] = pp_tbl;
		}
		pp_tbl++;
	}

	if( idx > 1)
	{
		pos = 0;
		while( pos < idx)
		{
			printf( "%3d ",		pos +1);
			printf( "%-20s ",	pp_tbl_idx[ pos]->data);
			printf( "%-30s ",	pp_tbl_idx[ pos]->comment);
			printf( "\n");
			pos++;
		}

		printf( "select view no: ");
		fgets( buf, 512, stdin);
		pos = atoi( buf);
	}
	else if( idx == 1)
	{
		pos = 1;
	}
	else
	{
		printf( "packet not found ... \n");
		return 1;
	}

	if( pos <= 0 || pos > idx) 
	{
		printf( "select error. input=[%d]\n", pos);
		return 1;
	}

	pp_tbl = pp_tbl_idx[ pos -1];
	ptr = strstr( data, pp_tbl->data);
	ptr -= pp_tbl->pos;
	pp_tbl->act( ptr);

	return 1;
}

