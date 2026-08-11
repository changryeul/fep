#define     _GLOBAL
/*------------------------------------------------------------------------
#   File    : px_showsise.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"

extern void     Sub_SHM(void);
extern void     Mem_SHM(int, int);

void    Sise_SHM(void);
char    *Get_DateTime(char *);

KS_EXPCODE  Key;
KS_LONGCODE LKey;

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    int     i, j, rt, max;
    int     mk_gbn, acc_seq, curr_cnt, tot_cnt;
    char    d_time[20], wbuf[1024], Tmp[32];

    if (argc != 3 && argc != 4 && argc != 5) {
        printf("=============================================\n");
        printf(" Ex) px_showsise_mp 1 A0                     \n");
        printf(" Ex) px_showsise_mp miche 1(mk_gbn) 0(ac_seq)\n");
        printf("=============================================\n");
        printf(" Ex) px_showsise_mp 1 Search KR7005930003    \n");
        printf(" Ex) px_showsise_mp 1 700 100                \n");
        printf(" Ex) px_showsise_mp 1 900 1                  \n");
        printf("---------------------------------------------\n");
        printf("     Market Gbn : 1        /* Note         */\n");
        printf("                  2        /* FinFut       */\n");
        printf("---------------------------------------------\n");
        printf(" TR Gbn : CURR, A0, A3, G7, B7, M4, S_SISE, KEY, Search... \n");
        printf("=============================================\n");
        exit(FAIL);
    }

    Sub_SHM();
    Mem_SHM(1, 0);
    Sise_SHM();

    if (memcmp(argv[1], "1", 1) == 0) {  // Note
        printf("Show Sise NOTE Market_Gbn[%s] total_item_cnt[%d]\n",
                argv[1], Shm_Note[0].total_item_cnt);

        if (memcmp(argv[2], "Search", 6) == 0) {
            memset(&Key, 0, sizeof (Key));
            memset(Tmp, 0, sizeof (Tmp));
            sprintf(Tmp, "%-12.12s", argv[3]);
            memcpy(Key.expcode, Tmp, strlen(Tmp));
            rt = 0;
            rt = Key_Search(NOTE_MK_KTS, KEY_EXPCODE, (char *)&Key);
            if (rt > 0) {
                printf("A0 rt[%d][%s]\n", rt, Shm_Note[rt].A0.tr_code);  // tr_gbn대신 tr_code(11)를 사용
                printf("Key rt[%d] idx[%d] code[%12.12s]\n",
                        rt, Shm_Item[0].N_Key[rt].idx, Shm_Item[0].N_Key[rt].expcode);
            }
            else
                printf("Not Found ItemCode[%12.12s]\n", argv[3]);
        }
        else {
            for (i = 0; i <= Shm_Note[0].total_item_cnt; i++) {
                if (memcmp(argv[2], "A0", 2) == 0)
                    printf("i[%d][%s]\n", i, Shm_Note[i].A0.tr_code);  // 채권종목정보는 시세에서 받지않고 RDS에서 받는다. 그래서 tr_gbn이 없다. tr_gbn대신 tr_code(11)를 사용한다.
                else
                    if (memcmp(argv[2], "A3", 2) == 0)
                    printf("i[%d][%s]\n", i, Shm_Note[i].A3.tr_gbn);
                else
                    if (memcmp(argv[2], "G7", 2) == 0)
                    printf("i[%d][%s]\n", i, Shm_Note[i].G7.tr_gbn);
                else
                    if (memcmp(argv[2], "B6", 2) == 0)
                    printf("i[%d][%s]\n", i, Shm_Note[i].B6.tr_gbn);
                else
                    if (memcmp(argv[2], "KEY", 3) == 0)
                    printf("i[%d] idx[%d] code[%12.12s]\n",
                            i, Shm_Item[0].N_Key[i].idx, Shm_Item[0].N_Key[i].expcode);
                else
                    if (memcmp(argv[2], "S_SISE", 6) == 0)
                    printf("i[%d][%s]\n", i, Shm_Risk[0].S_Sise[1][i].m_item_cd);
            }
        }
    }
    else
        if (memcmp(argv[1], "2", 1) == 0) {  // 금융파생(채권/달러)
        printf("Show Sise 금융파생 Market_Gbn[%s] total_item_cnt[%d]\n",
                argv[1], Shm_FinFut[0].total_item_cnt);

    if (memcmp(argv[2], "Search", 6) == 0) {
        memset(&Key, 0, sizeof (Key));
        memset(Tmp, 0, sizeof (Tmp));
        sprintf(Tmp, "%-12.12s", argv[3]);
        memcpy(Key.expcode, Tmp, strlen(Tmp));
        rt = 0;
        rt = Key_Search(DEV_MK_FIF, KEY_EXPCODE, (char *)&Key);
        if (rt > 0) {
            printf("A0  rt[%d][%s]\n", rt, Shm_FinFut[rt].A0.tr_gbn);
            printf("KEY rt[%d] idx[%d] code[%12.12s]\n",
                    rt, Shm_Item[0].D_Key[rt].idx, Shm_Item[0].D_Key[rt].expcode);
        }
        else
            printf("Not Found ItemCode[%12.12s]\n", argv[3]);
    }
    else {
        for (i = 0; i <= Shm_FinFut[0].total_item_cnt; i++) {
            if (memcmp(argv[2], "A0", 2) == 0)
                printf("i[%d][%s]\n", i, Shm_FinFut[i].A0.tr_gbn);
            else
                if (memcmp(argv[2], "A3", 2) == 0)
                printf("i[%d][%s]\n", i, Shm_FinFut[i].A3.tr_gbn);
            else
                if (memcmp(argv[2], "G7", 2) == 0)
                printf("i[%d][%s]\n", i, Shm_FinFut[i].G7.tr_gbn);
            else
                if (memcmp(argv[2], "B6", 2) == 0)
                printf("i[%d][%s]\n", i, Shm_FinFut[i].B6.tr_gbn);
            else
                if (memcmp(argv[2], "KEY", 3) == 0)
                printf("i[%d] idx[%d] code[%12.12s]\n",
                        i, Shm_Item[0].D_Key[i].idx, Shm_Item[0].D_Key[i].expcode);
            else
                if (memcmp(argv[2], "S_SISE", 6) == 0)
                printf("i[%d][%s]\n", i, Shm_Risk[0].S_Sise[2][i].m_item_cd);
        }
    }
}
else
    if (memcmp(argv[1], "miche", 5) == 0) {
    curr_cnt = 0;
    mk_gbn   = AtoIf(argv[2], strlen(argv[2]));
    acc_seq  = AtoIf(argv[3], strlen(argv[3]));
    tot_cnt  = Shm_Mk_PreMatch[0].MeChe_Cnt[mk_gbn][acc_seq];
    printf("Show miche Market_Gbn[%d] Acc_Seq[%d] total_miche_cnt[%d]\n", mk_gbn, acc_seq, tot_cnt);

    for (i = 0; i < MAX_MICHE; i++) {
        if (Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt > 0) {
            curr_cnt++;
            printf("i[%d] Jan_Cnt[%d] Mk_gbn[%d] Item_Seq[%d]\n",
                    i, Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Jan_Cnt,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Mk_gbn,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Item_Seq);
            printf("       Item_Cd[%12.12s] OrderNo[%10.10s] OriginalOrderNo[%10.10s] OrderFlag[%1.1s] TradeFlag[%1.1s] Order_Cnt[%8.8s] Order_Price_dv[%.2f] Lp1[%s/%s] Lp2[%s/%s] MembershipItem[%20.20s]\n",
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Item_Cd,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderNo,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OriginalOrderNo,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].OrderFlag,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].TradeFlag,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_dv,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt_Lp01,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_Lp01,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Cnt_Lp02,
                    Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].Order_Price_Lp02,
                    &Shm_Mk_PreMatch[0].F_MiChe[mk_gbn][acc_seq][i].MembershipItem[30]);
        }

        if (curr_cnt >= tot_cnt)
            break;
    }
}
else
    if (memcmp(argv[1], "700", 3) == 0) {  // 700100 View
    if (memcmp(argv[2], "100", 3) == 0) {  // 700100 View
        for (i = 0; i < MAX_AUTO_PROC; i++) {
            printf("i[%02d] run_gbn[%1.1s] ApType[%5.5s] Str_No[%4.4s] Item_Code[%12.12s]\n",
                    i, Shm_Risk[0].Auto_Stat[i].Run_Gbn, Shm_Risk[0].Auto_Stat[i].ApType,
                    Shm_Risk[0].Auto_Stat[i].Str_No, Shm_Risk[0].Auto_Stat[i].Item_Code);
        }
    }
}
else
    if (memcmp(argv[1], "900", 3) == 0) {  // 900 Set
    if (memcmp(argv[2], "1", 1) == 0) {  // 900001 Set
        for (i = 0; i < ACC_NO_CNT; i++)
            printf("i[%02d] acc_no[%12.12s] mk_gbn[%1.1s]",
                    i, Shm_Risk[0].Mst_Acc[i].acc_no, Shm_Risk[0].Mst_Acc[i].mk_gbn);
    }
    else if (memcmp(argv[2], "2", 1) == 0) {  // 900002 Set
        for (i = 0; i < ACC_NO_CNT; i++) {
            for (j = 0; j < RISK_MK_CNT; j++) {
                printf("acc[%02d] RiskMk[%02d] qty_gbn[%1.1s] qty[%08ld] money_gbn[%1.1s] money[%12.0lf] tick_gbn[%1.1s] tick[%08ld]\n",
                        i, j, Shm_Risk[0].O_M_Fund[j][i].qty_gbn,
                        Shm_Risk[0].O_M_Fund[j][i].qty,
                        Shm_Risk[0].O_M_Fund[j][i].money_gbn,
                        Shm_Risk[0].O_M_Fund[j][i].money,
                        Shm_Risk[0].O_M_Fund[j][i].tick_gbn,
                        Shm_Risk[0].O_M_Fund[j][i].tick);
                printf("          do_cnt_gbn[%1.1s] do_cnt[%08ld] do_money_gbn[%1.1s] do_money[%12.0lf] su_cnt_gbn[%1.1s] su_cnt[%08ld] su_money_gbn[%1.1s] su_money[%12.0lf]\n",
                        Shm_Risk[0].T_M_Fund[j][i].do_cnt_gbn,
                        Shm_Risk[0].T_M_Fund[j][i].do_cnt,
                        Shm_Risk[0].T_M_Fund[j][i].do_money_gbn,
                        Shm_Risk[0].T_M_Fund[j][i].do_money,
                        Shm_Risk[0].T_M_Fund[j][i].su_cnt_gbn,
                        Shm_Risk[0].T_M_Fund[j][i].su_cnt,
                        Shm_Risk[0].T_M_Fund[j][i].su_money_gbn,
                        Shm_Risk[0].T_M_Fund[j][i].su_money);
            }
        }
    }
    else if (memcmp(argv[2], "3", 1) == 0) {  // 900003 Set
        if (argc != 5) {
            printf(" Ex) px_showsise_mp 900 3 5(mk_gbn) 1(acc_seq)\n");
            exit(0);
        }

        mk_gbn   = AtoIf(argv[3], strlen(argv[3]));
        acc_seq  = AtoIf(argv[4], strlen(argv[4]));

        printf("mk_gbn[%d] acc_seq[%d]\n", mk_gbn, acc_seq);
        for (i = 0; i <= Shm_Risk[0].Max_Seq[mk_gbn]; i++) {
            if (Shm_Risk[0].ProFit[mk_gbn][i][acc_seq].item_getcnt != 0)
                printf("종목코드[%12.12s] 잔고[%ld]\n",
                        Shm_Risk[0].S_Sise[mk_gbn][i].m_item_cd,
                        Shm_Risk[0].ProFit[mk_gbn][i][acc_seq].item_getcnt);
        }
    }
    else if (memcmp(argv[2], "4", 1) == 0) {  // 900004 Set
        if (argc != 4) {
            printf(" Ex) px_showsise_mp 900 4 5(mk_gbn)\n");
            exit(0);
        }

        mk_gbn   = AtoIf(argv[3], strlen(argv[3]));

        printf("mk_gbn[%d]\n", mk_gbn);

        for (i = 0; i <= Shm_Risk[0].Max_Seq[mk_gbn]; i++) {
            if (Shm_Risk[0].S_Sise[mk_gbn][i].dont_trade > 0)
                printf("내부매매가능여부[%d]\n", Shm_Risk[0].S_Sise[mk_gbn][i].dont_trade);
        }
    }
    else if (memcmp(argv[2], "5", 1) == 0) {  // 900005 Set
        printf("기초자산코드 [%f]\n", Shm_Risk[0].indv_rate[8]);
    }
    else if (memcmp(argv[2], "6", 1) == 0) {  // 900006 Set
        printf("비지니스Day[%8.8s]\n", Shm_Risk[0].business_day);
    }
    else if (memcmp(argv[2], "7", 1) == 0) {  // 900007 Set
        printf("CD금리[%f]\n", Shm_Risk[0].cd_rate);
    }
}
else
    if (memcmp(argv[1], "Batch", 5) == 0) {  // 900 Set
    for (i = 0; i < 7; i++)
        printf("i[%02d] Wcnt[%d] Rcnt[%d]\n", i, Shm_Risk[0].Batch_Cnt[i].W_cnt, Shm_Risk[0].Batch_Cnt[i].R_cnt);
}

}   /* End of main ()   */

/*************************************************************************
    End of Program (px_getatm.c)
*************************************************************************/
