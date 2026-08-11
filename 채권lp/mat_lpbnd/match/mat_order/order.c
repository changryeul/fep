/** ***************************************************************************
**  @file       main.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  프로그램 초기화/프로세싱/종료
**  파라메터 세팅 및 환경파일 로드
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>

#include "mat.h"
#include "smq.h"
#include "order.h"

#include "proc.h"

#include "main.h"

/** ***************************************************************************
**  @fn         int TermProcess( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @return     프로그램 수행 결과
**  @retval     음수 실패 종료
**  @retval     양수 성공 종료
**  @exception
**  @remark
**  @brief
**  프로세스 종료 루틴 수행
***************************************************************************** */
int OrderProcess( ORDER_RECV *recv)
{
	int			rtn, i;
	char		f_name[ 512];
	static FILE	*fp = NULL;
	static int	line = 0;

	char		rec[ 8192];
	char		buf[ 1024];
	char		*ptr, *dest = ( char *)recv;
	int			sz, stat = 0, field = 0;
	int			buf_sz = 0, dest_sz = 0;

	if( fp == NULL)
	{
		LogDbg( "File open ... name=[%s]", Param->file_name);
		sprintf( f_name, "%s", Param->file_name);
		fp = fopen( f_name, "r");
		if( fp == NULL)
		{
			LogErr( "fopen error. name=[%s]", f_name);
			return -1;
		}
	}

	while( Continue)
	{
		ptr = fgets( rec, 8192, fp);
		if( ptr == NULL)
		{
			LogMsg( "End of file. fp=[%p] ptr=[%p]", fp, ptr);
			if( Param->loop)
			{
				Continue = 0;
				return 0;
			}
			fseek( fp, 0L, SEEK_SET);
			line = 0;
			field = 0;
			continue;
		}
		line++;

		sz = strlen( rec);
		if( sz <= 0)
		{
			LogDbg( "sz=[%d] rec=[%s]", sz, rec);
			sleep( 1);
			continue;
		}


		for( i = 0; i < sz; i++)
		{
			switch( rec[ i])
			{
				case '[':	
					stat = 1; 
					buf_sz = 0;
					break;
				case ']':	
					stat = 0; 
					buf[ buf_sz] = 0;
					switch( Param->type)
					{
						case 0:	/* line format 안씀 */
							break;
						case 1:	/* [ ] format */
							rtn = OrderBraceProcess( recv, buf, buf_sz);
							if( rtn < 0)
							{
								LogDbg( "convert error. line=[%d] field=[%d], rec=[%.50s]", line, field, rec);
								sleep( 1);
								return -1;
							}
							break;
						case 2:	/* struct print format */
							rtn = OrderStructProcess( recv, buf, buf_sz);
							if( rtn < 0)
							{
								LogDbg( "convert error. line=[%d] field=[%d], rec=[%.50s]", line, field, rec);
								sleep( 1);
								return -1;
							}
					}
					break;
				default:
					if( stat)	buf[ buf_sz++] = rec[ i];
					break;
			}
		}
		if( buf_sz <= 0)
		{
			LogDbg( "buf_sz=[%d] rec=[%s]", sz, rec);
			sleep( 1);
			continue;
		}
		buf[ buf_sz] = 0;

		LogDbg( "field=[%d] dest_sz=[%d] buf=[%s] ", field, dest_sz, buf);

#if 0
		switch( field)
		{
			case 0:		/* title */
				if( memcmp( buf, " ORDER_RECV ", 12)) return -1;
				break;
			case 36:	/* title */
				if( memcmp( buf, " ORDER_RECV ", 12)) 
				{
					LogDbg( "convert error. line=[%d] field=[%d], rec=[%.50s]", line, field, rec);
					sleep( 1);
					return -1;
				}
				return sizeof( ORDER_RECV);
			default:
				memcpy( &dest[ dest_sz], buf, buf_sz);
				dest_sz += buf_sz;
				break;
				
/* 	char	MsgType				[1  ];			주문타입                       */
/* 												D-신규,G-정정,F-취소           */
/* 	char	CustID				[30 ];			고객번호 (내부사용자ID)        */
/* 	char	OrgnGb				[1  ];			원천구분                       */
/* 												(1-고객거래,2-내부거래)        */
/* 	char	BkNo				[10 ];			북번호                         */
/* 	char	ClOrdID				[24 ];			주문번호                       */
/* 	char	OrigClOrdID			[24 ];			원주문번호                     */
/* 	char	Currency			[3  ];			기준통화코드(정보성)           */
/* 	char	OrderQty			[20 ];			주문수량                       */
/* 	char	OrdType				[1  ];			주문유형                       */
/* 												(1-시장가,2-지정가,3-예약주문) */
/* 	char	Price				[20 ];			주문가격                       */
/* 												(SWAP인 경우는 SWAP_RATE)      */
/* 	char	SlipCmpPrice		[20 ];			주문시점가격(시장가의 경우)    */
/* 	char	SlipPip				[20 ];			Slipage Pip(가격이격:체결범위) */
/* 	char	Side				[1  ];			매매구분                       */
/* 												(1-BUY(Sell&Buy), 2-SELL(Buy&Sell)) */
/* 	char	TimeInForce			[1  ];			주문유효시간                   */
/* 												0-For Day                      */
/* 												1-For Good Till Cancel         */
/* 												3-For Immediate Or Cancel(IOC) */
/* 												4-For Fill or Kill(FOK)        */
/* 												6-For Good Till Date(GTD)      */
/* 	char	TransactTime		[20 ];			처리시각                       */
/* 	char	SettType			[1  ];			FX상품구분코드                 */
/* 												: 1-TOD,2-TOM,3-SP(현물환),4-FWD,5-SWAP,8-MAR */
/* 	char	Symbol				[7  ];			FX상품코드     : USD/KRW       */
/* 	char	ValueDate1			[8  ];			결제시작일자                   */
/* 	char	ValueDate2			[8  ];			결제종료일자                   */
/* 	char	NearSettType		[3  ];			근일물상품구분코드             */
/* 												: 1-TOD,2-TOM, 3-SP(현물환),4-FWD,5-SWAP,8-MAR */
/* 	char	NearLegSide			[1  ];			NEAR매매구분(1-Buy,2-Sell)     */
/* 	char	NearLegSettlDate	[8  ];			NEAR-결제일자                  */
/* 	char	NearLegPrice		[20 ];			FWD는 FWD환율(고객가격)        */
/* 	char	NearLegPriceSprd	[20 ];			FWD는 FWD환율 스프레드         */
/* 	char	FarSettType			[1  ];			원일물상품구분코드             */
/* 												: 1-TOD,2-TOM,3-SP(현물환),4-FWD,5-SW,6-바로   */
/* 	char	FarLegSide			[1  ];			FAR매매구분(1-Buy, 2-Sell)     */
/* 	char	FarLegSettlDate		[8  ];			FAR-결제일자                   */
/* 	char	FarLegPrice			[20 ];			FWD는 FWD환율(고객가격)        */
/* 	char	FarLegPriceSprd		[20 ];			FWD는 FWD환율 스프레드         */
/* 	char	tran_ptrncd			[1  ];			거래유형코드                   */
/* 												(1-일반,2-MAR,3-RFQ)           */
/* 	char	GrpOrdnNo			[11 ];			그룹주문번호                   */
/* 	char	GrpOrdnCnt			[10 ];			그룹주문건수                   */
/* 	char	GrpOrdnSeq			[10 ];			그룹주문순번                   */
/* 	char	Filler				[645];			Filler                         */
/* 	char	Eof					[1  ];			EOF                            */
		}
		field++;
#endif
	}
	
	return sizeof( ORDER_RECV);
}

/** ***************************************************************************
**  @fn         int TermProcess( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @return     프로그램 수행 결과
**  @retval     음수 실패 종료
**  @retval     양수 성공 종료
**  @exception
**  @remark
**  @brief
**  프로세스 종료 루틴 수행
***************************************************************************** */
int OrderBraceProcess( char *dst, char *src, int sz)
{
	static int		field = 0;
	static int		dst_sz = 0;

	LogDbg( "field=[%3d] dst_sz=[%d] src=[%3d:%s]", field, dst_sz, sz, src);

	memcpy( &dst[ dst_sz], src, sz);
	dst_sz += sz;

	return 0;
}

/** ***************************************************************************
**  @fn         int TermProcess( int argc, char *argv[])
**  @param      int argc     - 프로그램 인수 갯수
**  @param      char *argv[] - 프로그램 인수
**  @return     프로그램 수행 결과
**  @retval     음수 실패 종료
**  @retval     양수 성공 종료
**  @exception
**  @remark
**  @brief
**  프로세스 종료 루틴 수행
***************************************************************************** */
int OrderStructProcess( char *dst, char *src, int sz)
{
	static int		field = 0;
	static int		dst_sz = 0;

	LogDbg( "field=[%3d] dst_sz=[%d] src=[%3d:%s]", field, dst_sz, sz, src);

	switch( field)
	{
		case 0:		/* start title */
			if( memcmp( src, " ORDER_RECV ", 12)) return -1;
			dst_sz = 0;
			break;
		default:
			if( !memcmp( src, " ORDER_RECV ", 12)) 
			{
				field = 0;
				return dst_sz;
			}
			memcpy( &dst[ dst_sz], src, sz);
			dst_sz += sz;
			break;
				
/* 	char	MsgType				[1  ];			주문타입                       */
/* 												D-신규,G-정정,F-취소           */
/* 	char	CustID				[30 ];			고객번호 (내부사용자ID)        */
/* 	char	OrgnGb				[1  ];			원천구분                       */
/* 												(1-고객거래,2-내부거래)        */
/* 	char	BkNo				[10 ];			북번호                         */
/* 	char	ClOrdID				[24 ];			주문번호                       */
/* 	char	OrigClOrdID			[24 ];			원주문번호                     */
/* 	char	Currency			[3  ];			기준통화코드(정보성)           */
/* 	char	OrderQty			[20 ];			주문수량                       */
/* 	char	OrdType				[1  ];			주문유형                       */
/* 												(1-시장가,2-지정가,3-예약주문) */
/* 	char	Price				[20 ];			주문가격                       */
/* 												(SWAP인 경우는 SWAP_RATE)      */
/* 	char	SlipCmpPrice		[20 ];			주문시점가격(시장가의 경우)    */
/* 	char	SlipPip				[20 ];			Slipage Pip(가격이격:체결범위) */
/* 	char	Side				[1  ];			매매구분                       */
/* 												(1-BUY(Sell&Buy), 2-SELL(Buy&Sell)) */
/* 	char	TimeInForce			[1  ];			주문유효시간                   */
/* 												0-For Day                      */
/* 												1-For Good Till Cancel         */
/* 												3-For Immediate Or Cancel(IOC) */
/* 												4-For Fill or Kill(FOK)        */
/* 												6-For Good Till Date(GTD)      */
/* 	char	TransactTime		[20 ];			처리시각                       */
/* 	char	SettType			[1  ];			FX상품구분코드                 */
/* 												: 1-TOD,2-TOM,3-SP(현물환),4-FWD,5-SWAP,8-MAR */
/* 	char	Symbol				[7  ];			FX상품코드     : USD/KRW       */
/* 	char	ValueDate1			[8  ];			결제시작일자                   */
/* 	char	ValueDate2			[8  ];			결제종료일자                   */
/* 	char	NearSettType		[3  ];			근일물상품구분코드             */
/* 												: 1-TOD,2-TOM, 3-SP(현물환),4-FWD,5-SWAP,8-MAR */
/* 	char	NearLegSide			[1  ];			NEAR매매구분(1-Buy,2-Sell)     */
/* 	char	NearLegSettlDate	[8  ];			NEAR-결제일자                  */
/* 	char	NearLegPrice		[20 ];			FWD는 FWD환율(고객가격)        */
/* 	char	NearLegPriceSprd	[20 ];			FWD는 FWD환율 스프레드         */
/* 	char	FarSettType			[1  ];			원일물상품구분코드             */
/* 												: 1-TOD,2-TOM,3-SP(현물환),4-FWD,5-SW,6-바로   */
/* 	char	FarLegSide			[1  ];			FAR매매구분(1-Buy, 2-Sell)     */
/* 	char	FarLegSettlDate		[8  ];			FAR-결제일자                   */
/* 	char	FarLegPrice			[20 ];			FWD는 FWD환율(고객가격)        */
/* 	char	FarLegPriceSprd		[20 ];			FWD는 FWD환율 스프레드         */
/* 	char	tran_ptrncd			[1  ];			거래유형코드                   */
/* 												(1-일반,2-MAR,3-RFQ)           */
/* 	char	GrpOrdnNo			[11 ];			그룹주문번호                   */
/* 	char	GrpOrdnCnt			[10 ];			그룹주문건수                   */
/* 	char	GrpOrdnSeq			[10 ];			그룹주문순번                   */
/* 	char	Filler				[645];			Filler                         */
/* 	char	Eof					[1  ];			EOF                            */
	}
	field++;
	return 0;
}


