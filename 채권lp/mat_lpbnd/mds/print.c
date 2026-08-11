#include <time.h>
#include "log.h"
#include "etc.h"
#include "mds_conv.h"

int XCHG_Print( XCHG* ptr)
{
    LogRaw( "%s", "----[ XCHG ]----------------------------------------------------------------------------\n");
    LogRaw( "exchange id                   exid                   4    0 = [%d]\n", 	ptr->exid);
    LogRaw( "short name                    exnm                   8    4 = [%.8s]\n", 	ptr->exnm);
    LogRaw( "S MBS/ K MBS/E MBS/ C MBS/ B  excode                 1   12 = [%.1s]\n", 	ptr->excode);
    LogRaw( "time zone                     TZ                    60   13 = [%.60s]\n", 	ptr->TZ);
    LogRaw( "maximum no of symbol for sha  maxcnt                 4   73 = [%d]\n", 	ptr->maxcnt);
    LogRaw( "재정환율계산여부              arbitrage              4   77 = [%d]\n", 	ptr->arbitrage);
    LogRaw( "클라이어언트 시세 전용 여부   sendclient_flag        4   81 = [%d]\n", 	ptr->sendclient_flag);
    LogRaw( "number of db update threads   db_pnum                4   85 = [%d]\n", 	ptr->db_pnum);
    LogRaw( "port id (=product name)       recv_name             16   89 = [%.16s]\n", 	ptr->recv_name);
    LogRaw( "ip address                    recv_ipad             20  105 = [%.20s]\n", 	ptr->recv_ipad);
    LogRaw( "port number                   recv_port              4  125 = [%d]\n", 	ptr->recv_port);
    LogRaw( "CUST 전송여부                 apsnd_cust             4  129 = [%d]\n", 	ptr->apsnd_cust);
    LogRaw( "AP 전송여부                   apsnd_cast             4  133 = [%d]\n", 	ptr->apsnd_cast);
    LogRaw( "BEST 전송여부                 apsnd_best             4  137 = [%d]\n", 	ptr->apsnd_best);
    LogRaw( "local address                 apsnd_neta            20  141 = [%.20s]\n", 	ptr->apsnd_neta);
    LogRaw( "multicasting IP-address       apsnd_ipad            20  161 = [%.20s]\n", 	ptr->apsnd_ipad);
    LogRaw( "                              apsnd_port             4  181 = [%d]\n", 	ptr->apsnd_port);
    LogRaw( "시세 수신 큐이름 (exchange.c  quenm                128  185 = [%.128s]\n", 	ptr->quenm);
    LogRaw( "data file path                dirp                 128  313 = [%.128s]\n", 	ptr->dirp);
    LogRaw( "log level                     llog                   4  441 = [%d]\n", 	ptr->llog);
    LogRaw( "log path                      logf                 128  445 = [%.128s]\n", 	ptr->logf);
    LogRaw( "%s", "----------------------------------------------------------------------------[ XCHG ]----\n");

    return sizeof( XCHG);
}

int MDARCH_Print( MDARCH* ptr)
{
    LogRaw( "%s", "----[ MDARCH ]--------------------------------------------------------------------------\n");
	XCHG_Print( &ptr->xchg);
    LogRaw( "                              sizefold               4    0 = [%d]\n", 	ptr->sizefold);
    LogRaw( "receive time stamp            rtim                   8    4 = [%s]\n",  TtoS( ptr->rtim));
    LogRaw( "daily total                   rsum                   4   12 = [%d]\n", 	ptr->rsum);
    LogRaw( "영업일                        tymd                   4   16 = [%d]\n", 	ptr->tymd);
    LogRaw( "max record                    mrec                   4   20 = [%d]\n", 	ptr->mrec);
    LogRaw( "current record numbers of wh  nrec                   4   24 = [%d]\n", 	ptr->nrec);
    LogRaw( "%s", "--------------------------------------------------------------------------[ MDARCH ]----\n");

    return sizeof( MDARCH);
}

int kswitch_t_Print( kswitch_t* ptr)
{
    LogRaw( "%s", "----[ kswitch_t ]-----------------------------------------------------------------------\n");
    LogRaw( "관리자ID                      userid                16    0 = [%.16s]\n", 	ptr->userid);
    LogRaw( "USD/KRW 브로커                usdkrw_exnm            8   16 = [%.8s]\n", 	ptr->usdkrw_exnm);
    LogRaw( "전체 O:OFF 1:ON               all_flag               4   24 = [%d]\n", 	ptr->all_flag);
    LogRaw( "개별 0:OFF 1:ON               run_flag               4   28 = [%d]\n", 	ptr->run_flag);
    LogRaw( "USD/KRW:Available bid/offer   value                  8   32 = [%f]\n", 	ptr->value);
    LogRaw( "%s", "-----------------------------------------------------------------------[ kswitch_t ]----\n");

    return sizeof( kswitch_t);
}

int CANDLE_Print( CANDLE* ptr)
{
    LogRaw( "%s", "----[ CANDLE ]--------------------------------------------------------------------------\n");
    LogRaw( "                              open                   8    0 = [%f]\n", 	ptr->open);
    LogRaw( "                              open_tm                8    8 = [%s]\n", 	TtoS(ptr->open_tm));
    LogRaw( "                              high                   8   16 = [%f]\n", 	ptr->high);
    LogRaw( "                              high_tm                8   24 = [%s]\n", 	TtoS(ptr->high_tm));
    LogRaw( "                              lowp                   8   32 = [%f]\n", 	ptr->lowp);
    LogRaw( "                              lowp_tm                8   40 = [%s]\n", 	TtoS(ptr->lowp_tm));
    LogRaw( "                              clos                   8   48 = [%f]\n", 	ptr->clos);
    LogRaw( "                              clos_tm                8   56 = [%s]\n", 	TtoS(ptr->clos_tm));
    LogRaw( "                              base                   8   64 = [%f]\n", 	ptr->base);
    LogRaw( "                              last_valid             8   72 = [%f]\n", 	ptr->last_valid);
    LogRaw( "%s", "--------------------------------------------------------------------------[ CANDLE ]----\n");

    return sizeof( CANDLE);
}

int MDCANDLE_Print( MDCANDLE* ptr)
{
    LogRaw( "%s", "----[ MDCANDLE ]------------------------------------------------------------------------\n");
    LogRaw( "                              exnm                   8    0 = [%.8s]\n", 	ptr->exnm);
    LogRaw( "                              symb                   8    8 = [%.8s]\n", 	ptr->symb);
    LogRaw( "                              kymd                   4   16 = [%d]\n", 	ptr->kymd);
    LogRaw( "                              khms                   4   20 = [%d]\n", 	ptr->khms);
    LogRaw( "                              tymd                   4   24 = [%d]\n", 	ptr->tymd);
    LogRaw( "                              seqn                   4   28 = [%d]\n", 	ptr->seqn);
    LogRaw( "                              tenor                  8   32 = [%.8s]\n", 	ptr->tenor);
	CANDLE_Print( &ptr->bid);
	CANDLE_Print( &ptr->ask);
	CANDLE_Print( &ptr->mid);
    LogRaw( "exnm(4) + symb(6) + 날짜(8)   tickkey               32   40 = [%.32s]\n", 	ptr->tickkey);
    LogRaw( "                              tenor_idx              4   72 = [%d]\n", 	ptr->tenor_idx);
    LogRaw( "%s", "------------------------------------------------------------------------[ MDCANDLE ]----\n");

    return sizeof( MDCANDLE);
}

int mdside_t_Print( mdside_t* ptr)
{
    LogRaw( "%s", "----[ mdside_t ]------------------------------------------------------------------------\n");
    LogRaw( "BID 원천 : SMBS/KMBS/EM       excode                 1    0 = [%.1s]\n", 	ptr->excode);
    LogRaw( "                              open                   8    1 = [%f]\n", 	ptr->open);
    LogRaw( "                              open_tm                8    9 = [%s]\n", 	TtoS( ptr->open_tm));
    LogRaw( "                              high                   8   17 = [%f]\n", 	ptr->high);
    LogRaw( "                              high_tm                8   25 = [%s]\n", 	TtoS( ptr->high_tm));
    LogRaw( "                              lowp                   8   33 = [%f]\n", 	ptr->lowp);
    LogRaw( "                              lowp_tm                8   41 = [%s]\n", 	TtoS( ptr->lowp_tm));
    LogRaw( "                              last                   8   49 = [%f]\n", 	ptr->last);
    LogRaw( "                              last_tm                8   57 = [%s]\n", 	TtoS( ptr->last_tm));
    LogRaw( "                              lastvol                8   65 = [%f]\n", 	ptr->lastvol);
    LogRaw( "last 중0이 아닌값             last_valid             8   73 = [%f]\n", 	ptr->last_valid);
    LogRaw( "전일종가                      base                   8   81 = [%f]\n", 	ptr->base);
    LogRaw( "                              best                   8   89 = [%f]\n", 	ptr->best);
    LogRaw( "                              bestvol                8   97 = [%f]\n", 	ptr->bestvol);
    LogRaw( "                              sign                   4  105 = [%d]\n", 	ptr->sign);
    LogRaw( "                              diff                   8  109 = [%f]\n", 	ptr->diff);
    LogRaw( "                              rate                   8  117 = [%f]\n", 	ptr->rate);
    LogRaw( "                              dirf                   4  125 = [%d]\n", 	ptr->dirf);
    LogRaw( "%s", "------------------------------------------------------------------------[ mdside_t ]----\n");

    return sizeof( mdside_t);
}

int mdquot_t_Print( mdquot_t* ptr)
{
    LogRaw( "%s", "----[ mdquot_t ]------------------------------------------------------------------------\n");
    LogRaw( "                              swap_bid               8    0 = [%f]\n", 	ptr->swap_bid);
    LogRaw( "                              swap_ask               8    8 = [%f]\n", 	ptr->swap_ask);
    LogRaw( "수기입력                      swap_bid_man           8   16 = [%f]\n", 	ptr->swap_bid_man);
    LogRaw( "수기입력                      swap_ask_man           8   24 = [%f]\n", 	ptr->swap_ask_man);
    LogRaw( "                              tick_seqn              4   32 = [%d]\n", 	ptr->tick_seqn);
	/*
	mdside_t_Print( &ptr->bid);
	mdside_t_Print( &ptr->ask);
	mdside_t_Print( &ptr->mid);
	*/
    LogRaw( "                              tickkey               32   36 = [%.32s]\n", 	ptr->tickkey);
	/*
	MDCANDLE_Print( &ptr->mdintr);
	*/
    LogRaw( "%s", "------------------------------------------------------------------------[ mdquot_t ]----\n");

    return sizeof( mdquot_t);
}

int MDFOLD_Print( MDFOLD* ptr)
{
	int		i,j;
	char	name[10][4] = { "SPT", "TOD", "TOM", "W01", "M01", "M02", "M03", "M06", "Y01", "\0" };

    LogRaw( "%s", "----[ MDFOLD ]--------------------------------------------------------------------------\n");
    LogRaw( "                              symb                   8    0 = [%.8s]\n", 	ptr->symb);
    LogRaw( "MDFOLD OFFSET                 seqn                   4    8 = [%d]\n", 	ptr->seqn);
    LogRaw( "current trading day           tymd                   4   12 = [%d]\n", 	ptr->tymd);
    LogRaw( "last update date              kymd                   4   16 = [%d]\n", 	ptr->kymd);
    LogRaw( "last update time              khms                   4   20 = [%d]\n", 	ptr->khms);
    LogRaw( "직원 소수점자리수             zdiv                   4   24 = [%d]\n", 	ptr->zdiv);
    LogRaw( "고객 소수점자리수             custzdiv               4   28 = [%d]\n", 	ptr->custzdiv);
    LogRaw( "스왑 소수점자리수             swapzdiv               4   32 = [%d]\n", 	ptr->swapzdiv);
    LogRaw( "마켓 소수점자리수             mrktzdiv               4   36 = [%d]\n", 	ptr->mrktzdiv);
    LogRaw( "data feeder id number         feed                   4   40 = [%d]\n", 	ptr->feed);
    LogRaw( "tradable flag                 trdf                   4   44 = [%d]\n", 	ptr->trdf);
    LogRaw( "장시작시간(hhmmss);           open_hms               4   48 = [%d]\n", 	ptr->open_hms);
	/*
	for( i = 0; i < 10; i++) 
	{
		if( name[ i][ 0] == 0) break;
		LogRaw( "name[%d]=[%s] \n", i, name[i]);
		mdquot_t_Print( &ptr->mdquot[ i]);
	}
	*/
    LogRaw( "크로스레이트 여부             crossrate              4   52 = [%d]\n", 	ptr->crossrate);
    LogRaw( "                              fillid                32   56 = [%.32s]\n", 	ptr->fillid);
    LogRaw( "체결일련번호                  fillseq                4   88 = [%d]\n", 	ptr->fillseq);
    LogRaw( "체결가격                      fillprc                8   92 = [%f]\n", 	ptr->fillprc);
    LogRaw( "체결수량                      fillqty                8  100 = [%f]\n", 	ptr->fillqty);
    LogRaw( "장종료시간                    close_hms              4  108 = [%d]\n", 	ptr->close_hms);
    LogRaw( "USD/KRW 킬스위치 작동 시간    kswitch_time_usd       8  112 = [%s]\n",  TtoS( ptr->kswitch_time_usd));
    LogRaw( "기타통화 킬스위치 작동시간    kswitch_time_etc       8  120 = [%s]\n",  TtoS( ptr->kswitch_time_etc));
    LogRaw( "                              best_flag              4  128 = [%d]\n", 	ptr->best_flag);
    LogRaw( "                              filler               256  132 = [%.256s]\n", 	ptr->filler);
    LogRaw( "%s", "--------------------------------------------------------------------------[ MDFOLD ]----\n");

    return sizeof( MDFOLD);
}

