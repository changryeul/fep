#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : Client 요구조회 처리
#   File    : pa_6000_mp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

#define     DATA_SIZE   2048
#include    "buf_struct.h"

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     DATA_TIME   60 * 1000
#define     READ_MAX    1

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int                         R_Cnt;
char                        ApType[10];
char                        W_Fmt[8192];
FILE_BUFF_FORMAT            R_Fmt[READ_MAX];

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    PA_6000_MP (void);
void    Analyze_Data (void);

void    Set_Accno(char *);              // 900001(계좌정보)
void    Set_Risk_Limit(char *);         // 900002(펀드별/리스크시장별 한도)
void    Set_Jango(char *);              // 900003(원장장고)
void    Set_Lock_Item(char *);          // 900004(매매제한)
void    Set_Indv_Rate(char *);          // 900005(증거금율)
void    Set_BusDay(char *);             // 900006(영업일자)
void    Set_Cd(char *);                 // 900007(CD금리)
void    All_Sise_rtn(char *);           // 700001(전체시장A0)
void    Item_Sise_rtn(char *);          // 700002(요청에 의한 특정종목 A0)
void    Str_Info(char *);               // 700100(전략용)
void    Write_Data (void);

/*----------------------------------------------------------------------*/
int     main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    Init_Proc (argc, argv);
    PA_6000_MP ();
    Exit_Process ();
}   /* End of main ()   */

/*----------------------------------------------------------------------*/
void    PA_6000_MP (void)
/*----------------------------------------------------------------------*/
{
    int     rt;

    sprintf (ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
    LtoU (ApType, strlen (ApType));

    while (START_S != JOB_END)
    {
        Stat_Save ();

        while (START_S != JOB_END)
        {
            Stat_Save ();
            memset (R_Fmt, 0, sizeof (FILE_BUFF_FORMAT) * READ_MAX);

            R_Cnt = F_R (PS_R_1, (void *)R_Fmt, READ_MAX);

            if (R_Cnt < 0)
            {
                Log (SAM_FATAL, "cannot read File[%s,%d:%s]",
                    IFN(D_K,P_K,0), SYS_NO, SYS_STR);
                sleep (1);
                Exit_Process ();
            }
            else if (R_Cnt == 0)
                break;

            Log (USR_OK, "RD [%s:%d][%d]",
                IFN(D_K,P_K,0), R_Cnt, IFR(D_K,P_K,0,0));
            Analyze_Data ();
        }

        rt = Poll_File (DATA_TIME);

        if (rt == 1)
            Log (USR_OK, "poll timeout <%d>", INT_SEQ);
        else if (rt == -1)
            continue;
    }
}   /* End of PA_6000_MP () */

/*************************************************************************
    Function        : . Analyze_Data
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . analyze and divide data
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Analyze_Data (void)
/*----------------------------------------------------------------------*/
{
    int     i;
    char    m_time[24];
    SEARCH_HEADER   *hd;

    memset (m_time, 0, sizeof (m_time));

/*  900001 : 계좌정보
    900002 : 계좌별/시장별 한도설정값
    900003 : 계좌별/시장별/종목별 잔고정보
    900004 : 매매제한종목(추천종목)
    ......
    700001 : A0배치정보
    700100 : 자동전략 상태조회
*/ 

    for (i = 0; i < R_Cnt; i ++)
    {
        memset (W_Fmt, 0x20, sizeof (W_Fmt));
        memcpy (&W_Fmt, &R_Fmt[i], sizeof (BUFF_RW_HEAD));
        memcpy (&W_Fmt[sizeof (BUFF_RW_HEAD)],  
                                    &R_Fmt[i].Data, SEARCH_HEADER_LEN);

        hd = (SEARCH_HEADER *)R_Fmt[i].Data;

        /* Set */
        if (memcmp (hd->TrCode, "900",   3) == 0)
        {
            /* Set 계좌정보 */
            if (memcmp (hd->TrCode, "900001",   6) == 0)
                Set_Accno(R_Fmt[i].Data);
            /* Set 계좌별/시장별 한도설정 */
            else if (memcmp (hd->TrCode, "900002",   6) == 0)
                Set_Risk_Limit(R_Fmt[i].Data);
            /* Set 계좌별/시장별/종목별 잔고설정 */
            else if (memcmp (hd->TrCode, "900003",   6) == 0)
                Set_Jango(R_Fmt[i].Data);
            /* 매매제한 종목 */
            else if (memcmp (hd->TrCode, "900004",   6) == 0)
                Set_Lock_Item(R_Fmt[i].Data);
            /* 증거금율 */
            else if (memcmp (hd->TrCode, "900005",   6) == 0)
                Set_Indv_Rate(R_Fmt[i].Data);
            /* 영업일자 */
            else if (memcmp (hd->TrCode, "900006",   6) == 0)
                Set_BusDay(R_Fmt[i].Data);
            /* CD금리 */
            else if (memcmp (hd->TrCode, "900007",   6) == 0)
                Set_Cd(R_Fmt[i].Data);
            else
            {
                Log (USR_WARN, "01. TrCode Different [%50.50s]", hd->TrCode);

                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "TRCD", 4);
                Write_Data ();
            }
        }
        else if (memcmp (hd->TrCode, "700",   3) == 0)
        {
            /* A0시세조회 */
            if (memcmp (hd->TrCode, "700001",   6) == 0)
                All_Sise_rtn(R_Fmt[i].Data);
            else if (memcmp (hd->TrCode, "700002",  6) == 0)
                Item_Sise_rtn(R_Fmt[i].Data);
            else if (memcmp (hd->TrCode, "700100",  6) == 0)
                Str_Info(R_Fmt[i].Data);
            else
            {
                Log (USR_WARN, "02. TrCode Different [%50.50s]", hd->TrCode);

                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "TRCD", 4);
                Write_Data ();
            }
        }
        else
        {
            Log (USR_WARN, "03. TrCode Different [%50.50s]", hd->TrCode);

            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "TRCD", 4);
            Write_Data ();
        }

        Add_Count (PS_R_1, 1);
        INT_SEQ ++;
        Log (USR_OK, "file write[%s:%d]", 
                                    OFN(D_K,P_K,0), OFW(D_K,P_K,0,0));
        Set_TR_Time ();
    }

    return;
}   /* End of Analyze_Data ()   */

/*************************************************************************
    Function        : . Set_Accno(Set계좌), 900001
    Parameters IN   : . p_buf   : received data
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . 잔고정보 data SHM 
*************************************************************************/
/*----------------------------------------------------------------------*/
void   Set_Accno(char *p_buf)
/*----------------------------------------------------------------------*/
{
    Shm_Risk[0].Batch_Cnt[0].W_cnt++;

    int     s_k;
    char    accno[20], mk_flag[2];

    /* 계좌일련번호 */
    s_k = AtoIf (&p_buf[SEARCH_HEADER_LEN], 2);
    /* 계좌번호 */
    memset (accno, 0, sizeof (accno));
    memcpy (accno, &p_buf[SEARCH_HEADER_LEN+2], 12);
    /* 현선물 구분 */
    memset (mk_flag, 0, sizeof (mk_flag));
    memcpy (mk_flag, &p_buf[SEARCH_HEADER_LEN+2+12], 1);

    if (s_k < 0 || s_k >= ACC_NO_CNT    ||
        (memcmp (mk_flag, "1", 1) != 0 && memcmp (mk_flag, "2", 1) != 0))
    {
        Log (USR_WARN, "900001 입력에 오류가 있습니다. 계좌일련번호[%d] 현선물구분[%1.1s]", s_k, mk_flag);

        memcpy (&W_Fmt[sizeof (BUFF_RW_HEAD)],  p_buf,  IFS(D_K,P_K,0));
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "INER", 4);
        Write_Data ();
        return;
    }
    else
    {
        /* 계좌번호 */
        memcpy (Shm_Risk[0].Mst_Acc[s_k].acc_no, accno, 12);
        /* 현선물구분 */
        memcpy (Shm_Risk[0].Mst_Acc[s_k].mk_gbn, mk_flag, 1);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "0000", 4);
    }   

    Write_Data ();
    Shm_Risk[0].Batch_Cnt[0].R_cnt++;

    return ;
}   /* End of Set_Accno() */

/*************************************************************************
    Function        : . Set_Risk_Limit(Set), 900002
    Parameters IN   : . p_buf   : received data
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . 리스크시장별 한도설정 data SHM 
*************************************************************************/
/*----------------------------------------------------------------------*/
void   Set_Risk_Limit(char *p_buf)
/*----------------------------------------------------------------------*/
{
    Shm_Risk[0].Batch_Cnt[1].W_cnt++;

    int     i, s_k, risk_cd, m_risk;

    IN_900002 *dat = (IN_900002 *)&p_buf[SEARCH_HEADER_LEN];

    for (i = 0; i < ACC_NO_CNT; i++)
    {
        if (memcmp (dat->Accno, Shm_Risk[0].Mst_Acc[i].acc_no, 12) == 0)
        {
            s_k = i;
            break;
        }

        if (i >= ACC_NO_CNT-1)
        {
            memcpy (&W_Fmt[sizeof (BUFF_RW_HEAD)],  p_buf,  IFS(D_K,P_K,0));
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "ACNO", 4);
            Write_Data ();
            Log (USR_WARN, "900002 입력된 계좌번호에 오류가 있습니다. 계좌번호[%12.12s]", dat->Accno);
            return;
        }
    }
    risk_cd = AtoIf (dat->Risk_Cd, 2);
    if (risk_cd == 11)          // 코스피200선물
        m_risk = 0;
    else if (risk_cd == 12)     // 코스피200콜옵션
        m_risk = 1;
    else if (risk_cd == 13)     // 코스피200풋옵션
        m_risk = 2;
    else if (risk_cd == 14)     // 주식선물
        m_risk = 3;
    else if (risk_cd == 15)     // 주식콜옵션
        m_risk = 4;
    else if (risk_cd == 16)     // 주식풋옵션
        m_risk = 5;
    else if (risk_cd == 20)     // 코스닥150선물
        m_risk = 6;
    else if (risk_cd == 44)     // 코스닥150콜옵션
        m_risk = 7;
    else if (risk_cd == 45)     // 코스닥150풋옵션
        m_risk = 8;
    else if (risk_cd == 46)     // KRX300선물
        m_risk = 9;
    else if (risk_cd == 51)     // 현물주식
        m_risk = 10;
    else if (risk_cd == 34)     // 미니선물
        m_risk = 11;
    else if (risk_cd == 35)     // 미니콜옵션
        m_risk = 12;
    else if (risk_cd == 36)     // 미니풋옵션
        m_risk = 13;
    else
    {
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)],   p_buf,  IFS(D_K,P_K,0));
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "MKCD", 4);
        Write_Data ();
        Log (USR_WARN, "900002 입력된 리스크시장구분에 오류가 있습니다. 리스크시장구분[%2.2s]", dat->Risk_Cd);
        return;
    }

    /* 1회주문계약수 체크여부 */
    memcpy (Shm_Risk[0].O_M_Fund[m_risk][s_k].qty_gbn, dat->One_Limit_Cnt_Gbn,   1);
    /* 1회주문계약수 */
    Shm_Risk[0].O_M_Fund[m_risk][s_k].qty =     AtoLf(dat->One_Limit_Cnt,       12);
    /* 1회주문금액 체크여부 */
    memcpy (Shm_Risk[0].O_M_Fund[m_risk][s_k].money_gbn, dat->One_Limit_Gum_Gbn, 1);
    /* 1회주문금액 */
    Shm_Risk[0].O_M_Fund[m_risk][s_k].money =   AtoDf(dat->One_Limit_Gum,       12);
    /* 1회주문Tick 체크여부 */
    memcpy (Shm_Risk[0].O_M_Fund[m_risk][s_k].tick_gbn, dat->One_Limit_Tick_Gbn, 1);
    /* 1회주문Tick */
    Shm_Risk[0].O_M_Fund[m_risk][s_k].tick =    AtoIf(dat->One_Limit_Tick_Gbn,   8);
/* 20220204 */
	/* 틱 최대 30틱 */
	if (Shm_Risk[0].O_M_Fund[m_risk][s_k].tick <= 0)
		Shm_Risk[0].O_M_Fund[m_risk][s_k].tick = 30;
	/* 금액 최대 3억 */
	if (Shm_Risk[0].O_M_Fund[m_risk][s_k].money <= 0)
		Shm_Risk[0].O_M_Fund[m_risk][s_k].money = 300000000;		// 300,000,000
	/* 계약수 최대 300계약 */
	if (Shm_Risk[0].O_M_Fund[m_risk][s_k].qty <= 0)
		Shm_Risk[0].O_M_Fund[m_risk][s_k].qty = 300;
/* 20220204 */

    /* 누적매도계약수 체크여부 */
    memcpy (Shm_Risk[0].T_M_Fund[m_risk][s_k].do_cnt_gbn, dat->Tot_Limit_DoCnt_Gbn,  1);
    /* 누적매도계약수 */
    Shm_Risk[0].T_M_Fund[m_risk][s_k].do_cnt =      AtoLf(dat->Tot_Limit_DoCnt,     12);
    /* 누적매도금액 체크여부 */
    memcpy (Shm_Risk[0].T_M_Fund[m_risk][s_k].do_money_gbn,dat->Tot_Limit_DoGum_Gbn, 1);
    /* 누적매도금액 */
    Shm_Risk[0].T_M_Fund[m_risk][s_k].do_money =    AtoDf(dat->Tot_Limit_DoGum,     12);
    /* 누적매수계약수 체크여부 */
    memcpy (Shm_Risk[0].T_M_Fund[m_risk][s_k].su_cnt_gbn, dat->Tot_Limit_SuCnt_Gbn,  1);
    /* 누적매수계약수 */
    Shm_Risk[0].T_M_Fund[m_risk][s_k].su_cnt =      AtoLf(dat->Tot_Limit_SuCnt,     12);
    /* 누적매수금액 체크여부 */
    memcpy (Shm_Risk[0].T_M_Fund[m_risk][s_k].su_money_gbn,dat->Tot_Limit_SuGum_Gbn, 1);
    /* 누적매수금액 */
    Shm_Risk[0].T_M_Fund[m_risk][s_k].su_money =    AtoDf(dat->Tot_Limit_SuGum,     12);
    
    memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)],   p_buf,  IFS(D_K,P_K,0));
    memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "0000", 4);

    Write_Data ();
    Shm_Risk[0].Batch_Cnt[1].R_cnt++;

    return ;
}   /* End of Set_Risk_Limit() */

/*************************************************************************
    Function        : . Set_Jango(Set), 900003
    Parameters IN   : . p_buf   : received data
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . 종목별 잔고정보 data SHM 
*************************************************************************/
/*----------------------------------------------------------------------*/
void   Set_Jango(char *p_buf)
/*----------------------------------------------------------------------*/
{
    Shm_Risk[0].Batch_Cnt[2].W_cnt++;

    int     i, s_k, rt, mk_gbn, item_seq;

    IN_900003 *dat = (IN_900003 *)&p_buf[SEARCH_HEADER_LEN];

    /* 계좌번호 체크 */
    for (i = 0; i < ACC_NO_CNT; i++)
    {
        if (memcmp (dat->Accno, Shm_Risk[0].Mst_Acc[i].acc_no, 12) == 0)
        {
            s_k = i;
            break;
        }

        if (i >= ACC_NO_CNT-1)
        {
            memcpy (&W_Fmt[sizeof (BUFF_RW_HEAD)],  p_buf,  IFS(D_K,P_K,0));
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "ACNO", 4);
            Write_Data ();
            Log (USR_WARN, "900003 입력된 계좌번호에 오류가 있습니다. 계좌번호[%12.12s]", dat->Accno);
            return;
        }
    }

    /* 시장구분 체크 */
    mk_gbn = AtoIf (dat->Mk_Gbn, 2);
    if (mk_gbn != 1     &&      // 지수선물
        mk_gbn != 2     &&      // 지수옵션
        mk_gbn != 3     &&      // 주식선물
        mk_gbn != 4     &&      // 주식옵션
        mk_gbn != 5     &&      // 유가증권
        mk_gbn != 6     &&      // 코스닥
        mk_gbn != 8     &&      // KRX300
        mk_gbn != 9     &&      // Kosdaq150선물
        mk_gbn != 10    &&      // Kosdaq150옵션
        mk_gbn != 11    &&      // 미니선물
        mk_gbn != 12)           // 미니옵션 
    {
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)],   p_buf,  IFS(D_K,P_K,0));
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "MKCD", 4);
        Write_Data ();
        Log (USR_WARN, "900003 입력된 시세시장구분에 오류가 있습니다. 시세시장구분[%2.2s]", dat->Mk_Gbn);
        return;
    }

    item_seq = AtoIf (dat->Item_Seq, 9);
    if (item_seq <= 0   ||
        item_seq >  Shm_Risk[0].Max_Seq[mk_gbn])
    {
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)],   p_buf,  IFS(D_K,P_K,0));
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "ISEQ", 4);
        Write_Data ();
        Log (USR_WARN, "900003 입력된 종목일련번호에 오류가 있습니다. 종목일련번호[%9.9s]", dat->Item_Seq);
        return;
    }

    /* 종목장부수량 */
    Shm_Risk[0].ProFit[mk_gbn][item_seq][s_k].item_getcnt       = AtoLf (dat->JanGo, 12);
    /* 장부금액, 현물만 사용 */
    Shm_Risk[0].ProFit[mk_gbn][item_seq][s_k].item_getmoney     = AtoLf (dat->JanGum, 12);
		// 파생은 장부금액 받은걸 평균단가로 계산해서 매매시 누적계산하는방식.
    if (Shm_Risk[0].ProFit[mk_gbn][item_seq][s_k].item_getcnt != 0)
		Shm_Risk[0].ProFit[mk_gbn][item_seq][s_k].item_get_avg_price = (double)AtoLf (dat->JanGum, 12) / (double)AtoLf (dat->JanGo, 12);
	else
		Shm_Risk[0].ProFit[mk_gbn][item_seq][s_k].item_get_avg_price = 0;
    /* 차입수량(전략에서 사용) */
    Shm_Risk[0].ProFit[mk_gbn][item_seq][s_k].item_borrow_cnt   = AtoLf (dat->Borrow_Cnt, 12);
    /* 매수누적체결수량(전략에서 사용) */
    Shm_Risk[0].ProFit[mk_gbn][item_seq][s_k].item_totsu_che    = AtoLf (dat->Total_Su_Che, 12);
    /* 매도누적체결수량(전략에서 사용) */
    Shm_Risk[0].ProFit[mk_gbn][item_seq][s_k].item_totdo_che    = AtoLf (dat->Total_Do_Che, 12);

Log (USR_OK, "900003 Set mk_gbn[%d] item_seq[%d] s_k[%d] item_getcnt[%ld] item_getmoney[%ld] item_borrow_cnt[%ld] item_totsu_che[%ld] item_totdo_che[%ld]",
mk_gbn, item_seq, s_k, Shm_Risk[0].ProFit[mk_gbn][item_seq][s_k].item_getcnt, Shm_Risk[0].ProFit[mk_gbn][item_seq][s_k].item_getmoney, Shm_Risk[0].ProFit[mk_gbn][item_seq][s_k].item_borrow_cnt, Shm_Risk[0].ProFit[mk_gbn][item_seq][s_k].item_totsu_che, Shm_Risk[0].ProFit[mk_gbn][item_seq][s_k].item_totdo_che);

    memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)],   p_buf,  IFS(D_K,P_K,0));
    memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "0000", 4);

    Write_Data ();
    Shm_Risk[0].Batch_Cnt[2].R_cnt++;

    return ;
}   /* End of Set_Jango() */

/*************************************************************************
    Function        : . Set_Lock_Item(Set), 900004
    Parameters IN   : . p_buf   : received data
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . 내부거래불가종목 data SHM 
*************************************************************************/
/*----------------------------------------------------------------------*/
void   Set_Lock_Item(char *p_buf)
/*----------------------------------------------------------------------*/
{
    Shm_Risk[0].Batch_Cnt[3].W_cnt++;

    int     i, s_k, rt, mk_gbn, item_seq;

    IN_900004 *dat = (IN_900004 *)&p_buf[SEARCH_HEADER_LEN];

    /* 시장구분 체크 */
    mk_gbn = AtoIf (dat->Mk_Gbn, 2);
    if (mk_gbn != 1     &&      // 지수선물
        mk_gbn != 2     &&      // 지수옵션
        mk_gbn != 3     &&      // 주식선물
        mk_gbn != 4     &&      // 주식옵션
        mk_gbn != 5     &&      // 유가증권
        mk_gbn != 6     &&      // 코스닥
        mk_gbn != 8     &&      // KRX300
        mk_gbn != 9     &&      // Kosdaq150선물
        mk_gbn != 10    &&      // Kosdaq150옵션
        mk_gbn != 11    &&      // 미니선물
        mk_gbn != 12)           // 미니옵션 
    {
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)],   p_buf,  IFS(D_K,P_K,0));
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "MKCD", 4);
        Write_Data ();
        Log (USR_WARN, "900004 입력된 시세시장구분에 오류가 있습니다. 시세시장구분[%2.2s]", dat->Mk_Gbn);
        return;
    }

    item_seq = AtoIf (dat->Item_Seq, 9);
    if (item_seq <= 0   ||
        item_seq >  Shm_Risk[0].Max_Seq[mk_gbn])
    {
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)],   p_buf,  IFS(D_K,P_K,0));
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "ISEQ", 4);
        Write_Data ();
        Log (USR_WARN, "900004 입력된 종목일련번호에 오류가 있습니다. 종목일련번호[%9.9s]", dat->Item_Seq);
        return;
    }

    /* 내부주문불가 종목 설정(1)및 해제(0)*/
    Shm_Risk[0].S_Sise[mk_gbn][item_seq].dont_trade = AtoIf (dat->Dont_Trade, 1);

    memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)],   p_buf,  IFS(D_K,P_K,0));
    memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "0000", 4);

    Write_Data ();
    Shm_Risk[0].Batch_Cnt[3].R_cnt++;

    return ;
}   /* End of Set_Lock_Item() */

/*************************************************************************
    Function        : . Set_Indv_Rate(Set), 900005
    Parameters IN   : . p_buf   : received data
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . 증거금율 data SHM, 전략에서 사용
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Set_Indv_Rate(char *p_buf)
/*----------------------------------------------------------------------*/
{
    Shm_Risk[0].Batch_Cnt[4].W_cnt++;

    if (memcmp (&p_buf[SEARCH_HEADER_LEN], "08", 2) != 0)
    {
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)],   p_buf,  IFS(D_K,P_K,0));
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "ISEQ", 4);
        Write_Data ();
        Log (USR_WARN, "900005 입력된 기초자산코드에 오류가 있습니다. 기초자산코드[%8.8s]", &p_buf[SEARCH_HEADER_LEN]);
        return;
    }

    /* 기초자산코드 */
    Shm_Risk[0].indv_rate[8] = AtoDf (&p_buf[SEARCH_HEADER_LEN+2], 8);

    memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)],   p_buf,  IFS(D_K,P_K,0));
    memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "0000", 4);

    Write_Data ();
    Shm_Risk[0].Batch_Cnt[4].R_cnt++;

    return ;
}   /* End of Set_Indv_Rate() */

/*************************************************************************
    Function        : . Set_BusDay(Set), 900006
    Parameters IN   : . p_buf   : received data
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . 최근운영일(당일이 아니면 오류) data SHM 
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Set_BusDay(char *p_buf)
/*----------------------------------------------------------------------*/
{
    Shm_Risk[0].Batch_Cnt[5].W_cnt++;

    memcpy (Shm_Risk[0].business_day, &p_buf[SEARCH_HEADER_LEN], 8);
	Shm_Risk[0].diff_calday = AtoIf (&p_buf[SEARCH_HEADER_LEN+16], 2);

    memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)],   p_buf,  IFS(D_K,P_K,0));
    memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "0000", 4);

    Write_Data ();
    Shm_Risk[0].Batch_Cnt[5].R_cnt++;

    return ;
}   /* End of Set_BusDay() */

/*************************************************************************
    Function        : . Set_Cd(Set), 900007
    Parameters IN   : . p_buf   : received data
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . CD금리 data SHM 
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Set_Cd(char *p_buf)
/*----------------------------------------------------------------------*/
{
    Shm_Risk[0].Batch_Cnt[6].W_cnt++;

    /* CD금리 */
    Shm_Risk[0].cd_rate = AtoDf (&p_buf[SEARCH_HEADER_LEN], 8);

    memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)],   p_buf,  IFS(D_K,P_K,0));
    memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "0000", 4);

    Write_Data ();
    Shm_Risk[0].Batch_Cnt[6].R_cnt++;

    return ;
}   /* End of Set_Cd() */

#ifdef	HONG_DEV
/*************************************************************************
    Function        : . All_Com_Proc_Sts(Set), 900100
    Parameters IN   : . p_buf   : received data
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . 전략상태 data SHM, 전략에서 처리
*************************************************************************/
typedef struct {
    char    process_id[20];                     /* process id (name)	*/
    char    Scr_key[4];                         /* 화면키               */
    char    ErrCode[4];                         /* Error Code           */
    char    ApType_Cd[5];                       /* Aptype Code          */
                                                /* Process 이름 (50101) */
                                    /* 자동기동/자동종료/강제종료시만 사용  */
    char    Media_gbn[1];                       /* 매체구분(A/C/T)      */
    char    Filler[30];                         /* 예비                 */
}   COM_PROC_STS;

typedef struct {
    char    ErrCode[4];                         /* Error Code           */
    char    ErrMsg[100];                        /* Error Message		*/
	char	DataCnt[3];							/* 데이터건수			*/
	COM_PROC_STS	ComProcSts[10];
}   TR900100;

/*----------------------------------------------------------------------*/
void    All_Com_Proc_Sts(char *p_buf)
/*----------------------------------------------------------------------*/
{
	int     dk, pk;
	int			cnt = 0;
	TR900100	*p;
	COM_PROC_STS	*cps;
	char		sub[4], tmpbuf[100];

	memset(sub, 0x00, sizeof(sub));
	memcpy(sub, "PA", 2);

	memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)], p_buf, SEARCH_HEADER_LEN);
	p = (TR900100 *)&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN] + 4;

	LtoU (sub, 2);
	dk = sub[1] - 'A';

	if (INFO(dk).process_id[0] == 0)
	{
		memcpy(p->ErrCode, "9999", sizeof(p->ErrCode));
		memset(tmpbuf, 0x00, sizeof(tmpbuf));
		sprintf(tmpbuf, "process not registered");
		memcpy(p->ErrMsg, errmsg, strlen(tmpbuf));
    	Write_Data ();
    	return ;
	}

	for (pk = 0; pk < DAEMON(dk).p_count; pk ++)
	{
		if ((PROC(dk, pk).type == TY_TRS2))
		{
			if (PROC(dk, pk).l.t2.l[0])
			{
				memset(tmpbuf, 0x00, sizeof(tmpbuf));
				sprintf(tmpbuf, "%*.*s", PROC(dk, pk).process_id);
				strncpy(p->ComProcSts[cnt].process_id, PROC(dk, pk).process_id, sizeof(p->ComProcSts[cnt].process_id));
			}
		}
		else
			continue;
	}
}
#endif

/*************************************************************************
    Function        : . Str_Info(Set), 700100
    Parameters IN   : . p_buf   : received data
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . 전략상태 data SHM, 전략에서 처리
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Str_Info(char *p_buf)
/*----------------------------------------------------------------------*/
{
    int     i, cnt, pk;
    char    *p;
    char    pname[12];

    memset (pname, 0, sizeof(pname));
    memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)], p_buf, SEARCH_HEADER_LEN);
    p = &W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN] + 4;

    cnt = 0;
    for (i = 0; i < MAX_AUTO_PROC; i++)
    {
        if (memcmp (Shm_Risk[0].Auto_Stat[i].Run_Gbn, "1", 1) == 0)
        {
            sprintf (pname, "pa_%5.5smp", Shm_Risk[0].Auto_Stat[i].ApType);
            for (pk = 0; pk < DAEMON(D_K).p_count; pk ++)
            {
                if (memcmp (PROC(D_K,pk).process_id, pname, 10) == 0)
                {
                    if (PROC(D_K,pk).process_status == 1)
                    {
                        memcpy (p, Shm_Risk[0].Auto_Stat[i].ApType, 9);
                        p += 9;
                        memcpy (p, Shm_Strrg[0].Lp_St[i].sRunStop, 1);
                        p += 1;     
                        memcpy (p, Shm_Risk[0].Auto_Stat[i].Item_Code, 12);
                        p += 12;

                        cnt++;
                    }
                    else
                        break;
                }
            }
        }
    }

    ItoAf(cnt, &W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 4);
    Write_Data ();

    return ;
}   /* End of Str_Info() */

/*************************************************************************
    Function        : . Item_Sise_rtn(Set), 700002
    Parameters IN   : . p_buf   : received data
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . 종목기본정보조회 data SHM 
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Item_Sise_rtn(char *p_buf)
/*----------------------------------------------------------------------*/
{
    int     i, rt, mk_gbn, item_seq;
    char    c_seq[10], item_code[20];

    KS_EXPCODE  Key;
    KS_LONGCODE  LKey;
    
    memset (c_seq,      0, sizeof(c_seq));
    memset (item_code,  0, sizeof(item_code));

    mk_gbn = AtoIf (&p_buf[SEARCH_HEADER_LEN], 2);
    memcpy (item_code, &p_buf[SEARCH_HEADER_LEN+2], 12);

    /* Key Search */
    if (mk_gbn > 30)    // 해외
    {
        /* index search */
        memset (&LKey, 0, sizeof (LKey));
        sprintf (LKey.longcode, "%-12.12s        ", item_code);
        rt = 0;
        rt = Key_Search (mk_gbn, KEY_LONGCODE, (char*)&LKey);
        if (rt >= 0)
        {
            if (mk_gbn == 31)
            {
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "31", 2);
                sprintf (c_seq, "%09d", rt);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], c_seq, 9);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2 + 9],
                        Shm_CME[rt].A0.type, sizeof(OC_MASTER));
            }
            else if (mk_gbn == 33)
            {
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "33", 2);
                sprintf (c_seq, "%09d", rt);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], c_seq, 9);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2 + 9],
                        Shm_SGX[rt].A0.type, sizeof(OC_MASTER));
            }
            else if (mk_gbn == 34)
            {
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "34", 2);
                sprintf (c_seq, "%09d", rt);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], c_seq, 9);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2 + 9],
                        Shm_ERX[rt].A0.type, sizeof(OC_MASTER));
            }
            else if (mk_gbn == 35)
            {
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "35", 2);
                sprintf (c_seq, "%09d", rt);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], c_seq, 9);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2 + 9],
                        Shm_HKE[rt].A0.type, sizeof(OC_MASTER));
            }
            else
            {
                Log (USR_WARN, "01. No Market [%64.64s]", p_buf);

                memcpy(&W_Fmt[sizeof(BUFF_RW_HEAD)], p_buf, SEARCH_HEADER_LEN+14);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "NOMK", 4);
            }
        }
        else
        {
            Log (USR_WARN, "01. 종목코드를 찾을수 없습니다. 시장[%2.2s] 코드[%12.12s]", &p_buf[SEARCH_HEADER_LEN], &p_buf[SEARCH_HEADER_LEN+2]);

            memcpy(&W_Fmt[sizeof(BUFF_RW_HEAD)], p_buf, SEARCH_HEADER_LEN+14);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "NOCD", 4);
        }
    }
    else                // KRX
    {
        /* index search */
        memset (&Key, 0, sizeof (Key));
        sprintf (Key.expcode, "%-12.12s", item_code);
        rt = 0;
        rt = Key_Search (mk_gbn, KEY_EXPCODE, (char*)&Key);
        if (rt > 0)
        {
            if (mk_gbn == 1)
            {
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "01000", 5);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
                        Shm_Futures[rt].A0.seq_no, sizeof(Shm_Futures[0].A0.seq_no));
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
                        Shm_Futures[rt].A0.tr_gbn, sizeof(SIF_A0014));
            }
            else if (mk_gbn == 2)
            {
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "02000", 5);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
                        Shm_Options[rt].A0.seq_no, sizeof(Shm_Options[0].A0.seq_no));
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
                        Shm_Options[rt].A0.tr_gbn, sizeof(SIF_A0014));
            }
            else if (mk_gbn == 3)
            {
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "03000", 5);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
                        Shm_SFutures[rt].A0.seq_no, sizeof(Shm_SFutures[0].A0.seq_no));
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
                        Shm_SFutures[rt].A0.tr_gbn, sizeof(SIF_A0014));
            }
            else if (mk_gbn == 4)
            {
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "04000", 5);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
                        Shm_SOptions[rt].A0.seq_no, sizeof(Shm_SOptions[0].A0.seq_no));
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
                        Shm_SOptions[rt].A0.tr_gbn, sizeof(SIF_A0014));
            }
            else if (mk_gbn == 5)
            {
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "05", 2);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], 
                        Shm_Stock[rt].A0.seq_no, sizeof(Shm_Stock[0].A0.seq_no));
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2 + 9],
                        Shm_Stock[rt].A0.tr_gbn, sizeof(STOCK_A0011));
            }
            else if (mk_gbn == 6)
            {
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "06", 2);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], 
                        Shm_Kosdaq[rt].A0.seq_no, sizeof(Shm_Kosdaq[0].A0.seq_no));
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2 + 9],
                        Shm_Kosdaq[rt].A0.tr_gbn, sizeof(STOCK_A0011));
            }
            else if (mk_gbn == 8)
            {
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "08000", 5);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
                        Shm_K300[rt].A0.seq_no, sizeof(Shm_K300[0].A0.seq_no));
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
                        Shm_K300[rt].A0.tr_gbn, sizeof(SIF_A0014));
            }
            else if (mk_gbn == 9)
            {
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "09000", 5);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
                        Shm_K150F[rt].A0.seq_no, sizeof(Shm_K150F[0].A0.seq_no));
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
                        Shm_K150F[rt].A0.tr_gbn, sizeof(SIF_A0014));
            }
            else if (mk_gbn == 10)
            {
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "10000", 5);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
                        Shm_K150O[rt].A0.seq_no, sizeof(Shm_K150O[0].A0.seq_no));
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
                        Shm_K150O[rt].A0.tr_gbn, sizeof(SIF_A0014));
            }
            else if (mk_gbn == 11)
            {
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "11000", 5);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
                        Shm_MF[rt].A0.seq_no, sizeof(Shm_MF[0].A0.seq_no));
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
                        Shm_MF[rt].A0.tr_gbn, sizeof(SIF_A0014));
            }
            else if (mk_gbn == 12)
            {
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "12000", 5);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
                        Shm_MO[rt].A0.seq_no, sizeof(Shm_MO[0].A0.seq_no));
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
                        Shm_MO[rt].A0.tr_gbn, sizeof(SIF_A0014));
            }
            else
            {
                Log (USR_WARN, "02. No Market [%64.64s]", p_buf);
                memcpy(&W_Fmt[sizeof(BUFF_RW_HEAD)], p_buf, SEARCH_HEADER_LEN+14);
                memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "NOMK", 4);
            }

        }
        else
        {
            Log (USR_WARN, "02. 종목코드를 찾을수 없습니다. 시장[%2.2s] 코드[%12.12s]", &p_buf[SEARCH_HEADER_LEN], &p_buf[SEARCH_HEADER_LEN+2]);
            memcpy(&W_Fmt[sizeof(BUFF_RW_HEAD)], p_buf, SEARCH_HEADER_LEN+14);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "NOCD", 4);
        }
    }
    Write_Data ();

    return ;
}   /* End of Item_Sise_rtn() */

/*************************************************************************
    Function        : . All_Sise_rtn, 700001
    Parameters IN   : . p_buf   : received data
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . set sise data SHM (현재가)
*************************************************************************/
/*----------------------------------------------------------------------*/
void   All_Sise_rtn (char *p_buf)
/*----------------------------------------------------------------------*/
{
    int     i, s_k, flag, mk_gbn;
    char    c_seq[10];

    memset (c_seq, 0, sizeof(c_seq));
    flag = 0;

    mk_gbn = AtoIf (&p_buf[SEARCH_HEADER_LEN], 2);

    /* 1/2/3/4/5/6/8/9/10 시장순서데로 A0포맷으로 송신 */

    if (mk_gbn == 0 || mk_gbn == 1)
    {
        flag = 1;
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        for (i = 1; i <= Shm_Futures[0].total_item_cnt; i++)        // 1. 지수선물
        {
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "01000", 5);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
                    Shm_Futures[i].A0.seq_no, sizeof(Shm_Futures[0].A0.seq_no));
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
                    Shm_Futures[i].A0.tr_gbn, sizeof(SIF_A0014));
            Write_Data ();
        }
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "01", 2);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], "999999999", 9);
        Write_Data ();
    }

    if (mk_gbn == 0 || mk_gbn == 2)
    {
        flag = 1;
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        for (i = 1; i <= Shm_Options[0].total_item_cnt; i++)        // 2. 지수옵션
        {
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "02000", 5);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
                    Shm_Options[i].A0.seq_no, sizeof(Shm_Options[0].A0.seq_no));
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
                    Shm_Options[i].A0.tr_gbn, sizeof(SIF_A0014));
            Write_Data ();
        }
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "02", 2);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], "999999999", 9);
        Write_Data ();
    }

    if (mk_gbn == 0 || mk_gbn == 3)
    {
        flag = 1;
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        for (i = 1; i <= Shm_SFutures[0].total_item_cnt; i++)   // 3. 주식선물
        {
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "03000", 5);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
                    Shm_SFutures[i].A0.seq_no, sizeof(Shm_SFutures[0].A0.seq_no));
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
                    Shm_SFutures[i].A0.tr_gbn, sizeof(SIF_A0014));
            Write_Data ();
        }
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "03", 2);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], "999999999", 9);
        Write_Data ();
    }

    if (mk_gbn == 0 || mk_gbn == 4)
    {
        flag = 1;
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        for (i = 1; i <= Shm_SOptions[0].total_item_cnt; i++)   // 4. 주식옵션
        {
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "04000", 5);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
                    Shm_SOptions[i].A0.seq_no, sizeof(Shm_SOptions[0].A0.seq_no));
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
                    Shm_SOptions[i].A0.tr_gbn, sizeof(SIF_A0014));
            Write_Data ();
        }
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "04", 2);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], "999999999", 9);
        Write_Data ();
    }

    if (mk_gbn == 0 || mk_gbn == 5)
    {
        flag = 1;
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        for (i = 1; i <= Shm_Stock[0].total_item_cnt; i++)      // 5. 유가증권
        {
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "05", 2);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], 
                    Shm_Stock[i].A0.seq_no, sizeof(Shm_Stock[0].A0.seq_no));
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2 + 9],
                    Shm_Stock[i].A0.tr_gbn, sizeof(STOCK_A0011));
            Write_Data ();
        }
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "05", 2);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], "999999999", 9);
        Write_Data ();
    }

    if (mk_gbn == 0 || mk_gbn == 6)
    {
        flag = 1;
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        for (i = 1; i <= Shm_Kosdaq[0].total_item_cnt; i++)     // 6. 코스닥
        {
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "06", 2);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], 
                    Shm_Kosdaq[i].A0.seq_no, sizeof(Shm_Kosdaq[0].A0.seq_no));
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2 + 9],
                    Shm_Kosdaq[i].A0.tr_gbn, sizeof(STOCK_A0011));
            Write_Data ();
        }
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "06", 2);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], "999999999", 9);
        Write_Data ();
    }

    if (mk_gbn == 0 || mk_gbn == 8)
    {
        flag = 1;
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        for (i = 1; i <= Shm_K300[0].total_item_cnt; i++)       // 8. KRX300
        {
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "08000", 5);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
                        Shm_K300[i].A0.seq_no, sizeof(Shm_K300[0].A0.seq_no));
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
                    Shm_K300[i].A0.tr_gbn, sizeof(SIF_A0014));
            Write_Data ();
        }
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "08", 2);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], "999999999", 9);
        Write_Data ();
    }

    if (mk_gbn == 0 || mk_gbn == 9)
    {
        flag = 1;
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        for (i = 1; i <= Shm_K150F[0].total_item_cnt; i++)      // 9. K150 Futures
        {
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "09000", 5);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
                    Shm_K150F[i].A0.seq_no, sizeof(Shm_K150F[0].A0.seq_no));
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
                    Shm_K150F[i].A0.tr_gbn, sizeof(SIF_A0014));
            Write_Data ();
        }
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "09", 2);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], "999999999", 9);
        Write_Data ();
    }

    if (mk_gbn == 0 || mk_gbn == 10)
    {
        flag = 1;
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        for (i = 1; i <= Shm_K150O[0].total_item_cnt; i++)      // 10. K150 Options
        {
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "10000", 5);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
                    Shm_K150O[i].A0.seq_no, sizeof(Shm_K150O[0].A0.seq_no));
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
                    Shm_K150O[i].A0.tr_gbn, sizeof(SIF_A0014));
            Write_Data ();
        }
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "10", 2);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], "999999999", 9);
        Write_Data ();
    }
    if (mk_gbn == 0 || mk_gbn == 11)
    {
        flag = 1;
		memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
		for (i = 1; i <= Shm_MF[0].total_item_cnt; i++)     // 11. Mini Futures
		{
			memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "11000", 5);
			memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
					Shm_MF[i].A0.seq_no, sizeof(Shm_MF[0].A0.seq_no));
			memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
					Shm_MF[i].A0.tr_gbn, sizeof(SIF_A0014));
			Write_Data ();
		}
		memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
		memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "11", 2);
		memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], "999999999", 9);
		Write_Data ();
	}

    if (mk_gbn == 0 || mk_gbn == 12)
    {
        flag = 1;
		memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
		for (i = 1; i <= Shm_MO[0].total_item_cnt; i++)     // 12. Mini Options
		{
			memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "12000", 5);
			memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 5], 
					Shm_MO[i].A0.seq_no, sizeof(Shm_MO[0].A0.seq_no));
			memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2+3 + 6],
					Shm_MO[i].A0.tr_gbn, sizeof(SIF_A0014));
			Write_Data ();
		}
		memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
		memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "12", 2);
		memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], "999999999", 9);
		Write_Data ();
	}

    if (mk_gbn == 0 || mk_gbn == 31)
    {
        flag = 1;
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        for (i = 0; i < Shm_CME[0].total_item_cnt; i++)     // 31. CME
        {
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "31", 2);
            sprintf (c_seq, "%09d", i);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], c_seq, 9);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2 + 9],
                    Shm_CME[i].A0.type, sizeof(OC_MASTER));
            Write_Data ();
        }
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "31", 2);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], "999999999", 9);
        Write_Data ();
    }

    if (mk_gbn == 0 || mk_gbn == 33)
    {
        flag = 1;
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        for (i = 0; i < Shm_SGX[0].total_item_cnt; i++)     // 33. SGX
        {
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "33", 2);
            sprintf (c_seq, "%09d", i);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], c_seq, 9);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2 + 9],
                    Shm_SGX[i].A0.type, sizeof(OC_MASTER));
            Write_Data ();
        }
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "33", 2);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], "999999999", 9);
        Write_Data ();
    }

    if (mk_gbn == 0 || mk_gbn == 34)
    {
        flag = 1;
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        for (i = 0; i < Shm_ERX[0].total_item_cnt; i++)     // 34. ERX
        {
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "34", 2);
            sprintf (c_seq, "%09d", i);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], c_seq, 9);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2 + 9],
                    Shm_ERX[i].A0.type, sizeof(OC_MASTER));
            Write_Data ();
        }
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "34", 2);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], "999999999", 9);
        Write_Data ();
    }

    if (mk_gbn == 0 || mk_gbn == 35)
    {
        flag = 1;
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        for (i = 0; i < Shm_HKE[0].total_item_cnt; i++)     // 35. HKE
        {
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "35", 2);
            sprintf (c_seq, "%09d", i);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], c_seq, 9);
            memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2 + 9],
                    Shm_HKE[i].A0.type, sizeof(OC_MASTER));
            Write_Data ();
        }
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "35", 2);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], "999999999", 9);
        Write_Data ();
    }

    if (mk_gbn == 0)
    {
        /* 시세송신 끝 99 999999999 */
        memset (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], 0x20, sizeof(W_Fmt) - sizeof(BUFF_RW_HEAD) - SEARCH_HEADER_LEN);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN], "99", 2);
        memcpy (&W_Fmt[sizeof(BUFF_RW_HEAD) + SEARCH_HEADER_LEN + 2], "999999999", 9);
        Write_Data ();
    }

    if (flag == 0)
    {
        memcpy(&W_Fmt[sizeof(BUFF_RW_HEAD)], p_buf, SEARCH_HEADER_LEN+2);
        memcpy(&W_Fmt[sizeof(BUFF_RW_HEAD)+10], "9999", 4);         
        Write_Data ();
    }

    return ;
}   /* End of All_Sise_rtn( ) */

/**************************************************************************
    Function        : . Write_Data
    Parameters IN   : . p_flag  : write flag
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . write data to file
**************************************************************************/
/*-----------------------------------------------------------------------*/
void    Write_Data (void)
/*-----------------------------------------------------------------------*/
{
    int     rt;

    W_Fmt[sizeof(BUFF_RW_HEAD) + OFS(D_K,P_K,0)] = '\n';

    rt = F_W (TS_W1_1, (void *)&W_Fmt, 1);

    if (rt != 1)
    {
        Log (SAM_FATAL, "file write fail[%s]", OFN(D_K,P_K,0));
        Exit_Process ();
    }

    Log (USR_OK, "file write[%s:%d:%d]",
        OFN(D_K,P_K,0), OFW(D_K,P_K,0,0), rt);

    return;
}   /* End of Write_Data () */

/*************************************************************************
    End of Program (pa_6000_mp.c)
*************************************************************************/
