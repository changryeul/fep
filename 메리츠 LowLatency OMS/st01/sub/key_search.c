/*------------------------------------------------------------------------
#	Module	: key search
#	File	: key_search.c
------------------------------------------------------------------------*/
#include	"fep_sub.h" 

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
int		Key_Search (int, int , char *);
int		CmpExpcode (const void *, const void *);
int		CmpLongcode (const void *, const void *);
int		BsearchMode(char *, char *, int , int , int (*compar)(), int );

/*************************************************************************
	Function		: . 
	Parameters IN	: . market		: 시장구분 Market Gbn
					  . kind			: 키구분 KS_EXPCODE
	Parameters OUT	: . index		: index, 음수이면 에러, 0이상 이면 정상
	Return Code		: . char * (string converted)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Key_Search (int mk_gbn, int kind, char *keybuf)
/*----------------------------------------------------------------------*/
{
	int			totcnt=0;									/* 총종목수	*/
	int			pos;
	KS_EXPCODE  *key1;										/* KEY 1	*/
	KS_LONGCODE *key2;										/* KEY 2	*/

	/*------------------------------------------------------------------*/
    /* 1. Key영역을 검색하여 시작 위치를 구한다.                        */
    /*------------------------------------------------------------------*/
	if (kind == KEY_EXPCODE)
	{
		if (mk_gbn == MK_JF)								// 지수선물
		{
			totcnt = Shm_Futures[0].total_item_cnt;
			key1 = (KS_EXPCODE *)(&Shm_Item[0].F_Key);
		}
		else if (mk_gbn == MK_JO)							// 지수옵션
		{
			totcnt = Shm_Options[0].total_item_cnt;
			key1 = (KS_EXPCODE *)(&Shm_Item[0].O_Key);
		}
		else if (mk_gbn == MK_SF)							// 주식선물
		{
			totcnt = Shm_SFutures[0].total_item_cnt;
			key1 = (KS_EXPCODE *)(&Shm_Item[0].SF_Key);
		}
		else if (mk_gbn == MK_SO)							// 주식옵션
		{
			totcnt = Shm_SOptions[0].total_item_cnt;
			key1 = (KS_EXPCODE *)(&Shm_Item[0].SO_Key);
		}
		else if (mk_gbn == MK_KOSPI)						// 유가증권
		{
			totcnt = Shm_Stock[0].total_item_cnt;
			key1 = (KS_EXPCODE *)(&Shm_Item[0].S_Key);
		}
		else if (mk_gbn == MK_KOSDAQ)							// 코스닥
		{
			totcnt = Shm_Kosdaq[0].total_item_cnt;
			key1 = (KS_EXPCODE *)(&Shm_Item[0].K_Key);
		}
#if 0
		else if (mk_gbn == MK_ELW)							// ELW
		{
			totcnt = Shm_Elw[0].total_item_cnt;
			key1 = (KS_EXPCODE *)(&Shm_Item[0].ELW_Key);
		}
#endif
		else if (mk_gbn == MK_K300)							// KRX300
		{
			totcnt = Shm_K300[0].total_item_cnt;
			key1 = (KS_EXPCODE *)(&Shm_Item[0].K300_Key);
		}
		else if (mk_gbn == MK_K150F)						// KOSDAQ150F
		{
			totcnt = Shm_K150F[0].total_item_cnt;
			key1 = (KS_EXPCODE *)(&Shm_Item[0].K150F_Key);
		}
		else if (mk_gbn == MK_K150O)						// KOSDAQ150O
		{
			totcnt = Shm_K150O[0].total_item_cnt;
			key1 = (KS_EXPCODE *)(&Shm_Item[0].K150O_Key);
		}
		else if (mk_gbn == MK_MF)							// 미니선물
		{
			totcnt = Shm_MF[0].total_item_cnt;
			key1 = (KS_EXPCODE *)(&Shm_Item[0].MF_Key);
		}
		else if (mk_gbn == MK_MO)							// 미니옵션
		{
			totcnt = Shm_MO[0].total_item_cnt;
			key1 = (KS_EXPCODE *)(&Shm_Item[0].MO_Key);
		}
		else if (mk_gbn == MK_JISU)							// 지수
		{
			totcnt = Shm_Jisu[0].total_item_cnt;
			key1 = (KS_EXPCODE *)(&Shm_Item[0].J_Key);
		}
		else
			return NOTOK;

		pos = BsearchMode((char *)key1, keybuf, totcnt, sizeof(KS_EXPCODE), CmpExpcode, EQ);

		if (pos < 0 || key1[pos].idx < 0)
			return NOTOK;
		else
		{
			return	key1[pos].idx;
		}
	}
	else if (kind == KEY_LONGCODE)
	{
		if (mk_gbn == MK_CME)
		{
			totcnt = Shm_CME[0].total_item_cnt;
			key2 = (KS_LONGCODE *)(&Shm_Item[0].CME_Key);
		}
		else if (mk_gbn == MK_SGX)
		{
			totcnt = Shm_SGX[0].total_item_cnt;
			key2 = (KS_LONGCODE *)(&Shm_Item[0].SGX_Key);
		}
		else if (mk_gbn == MK_ERX)
		{
			totcnt = Shm_ERX[0].total_item_cnt;
			key2 = (KS_LONGCODE *)(&Shm_Item[0].ERX_Key);
		}
		else if (mk_gbn == MK_HKE)
		{
			totcnt = Shm_HKE[0].total_item_cnt;
			key2 = (KS_LONGCODE *)(&Shm_Item[0].HKE_Key);
		}

		pos = BsearchMode((char *)key2, keybuf, totcnt, sizeof(KS_LONGCODE), CmpLongcode, EQ);
		if (pos < 0 || key2[pos].idx < 0)
			return NOTOK;
		else
		{
			return	key2[pos].idx;
		}
	}
	else
		return NOTOK;

}	/* End of LtoU ()	*/

/******************************************************************************
 * * FUNCTION:    CmpIDXMasterShcode
 * * DESCRIPTION: Qrery key 비교함수.
 * * PARAMETERS:
 * *              const void *a - Compare memory A
 * *              const void *b - Compare memory B
 * * RETURNED:    compare value
 * ******************************************************************************/
int CmpExpcode(const void *a, const void *b)
{
    KS_EXPCODE *aa, *bb;
    aa = (KS_EXPCODE *)a;
	bb = (KS_EXPCODE *)b;
    return memcmp(aa->expcode, bb->expcode, sizeof(aa->expcode));
}

/******************************************************************************
 * * FUNCTION:    CmpIDXMasterShcode
 * * DESCRIPTION: Qrery key 비교함수.
 * * PARAMETERS:
 * *              const void *a - Compare memory A
 * *              const void *b - Compare memory B
 * * RETURNED:    compare value
 * ******************************************************************************/
int CmpLongcode(const void *a, const void *b)
{
    KS_LONGCODE *aa, *bb;
    aa = (KS_LONGCODE *)a;
	bb = (KS_LONGCODE *)b;
    return memcmp(aa->longcode, bb->longcode, sizeof(aa->longcode));
}

/*******************************************************************************
 * 설명      : Mode(LE,LT,EQ,GE,GT)를 가지고 해당 Position을 찾아냄.
 * Prototype : int BsearchMode(char *base, char *sd, int recnum, int recsize,
 *                 int (*compar)(), int mode);
 * Arguments :  char    *base       - 전체 Space
 *              char    *sd         - Key record
 *              int     recnum      - Record count
 *              int     recsize     - Record size
 *              int     (*compar)() - Compare Function
 *              int     mode        - LE,LT,EQ,GT,GE
 * Return    : Success - Search index, Fail - -1
 * ****************************************************************************/
int BsearchMode(char *base, char *sd, int recnum, int recsize, int (*compar)(), int mode)
{
    int fpos, lpos, cpos, cmp, itmp1, itmp2;

    if(recnum <= 0) return -1;

    if(recnum == 1) {
        cmp = compar(base,sd);
        if(cmp==0) {
            if((mode == EQ)||(mode == LE)||(mode == GE)) return 0;
        }
        else if(cmp < 0) {
            if((mode == LT)||(mode == LE)) return 0;
        }
        else {
            if((mode == GT)||(mode == GE)) return 0;
        }

        return -1;
    }

    fpos = 0;
    lpos = recnum - 1;
    cpos = (recnum - 1) / 2;

	while(1) {
        cmp = compar(base+(recsize*cpos),sd);
        if(cmp==0) {
            if((mode==EQ)||(mode==LE)||(mode==GE)) {
                if((cpos>=0)&&(cpos<recnum)) return cpos;
                else                         return -1;
            }
            else if(mode==GT) {
                if(((cpos+1)>=0)&&((cpos+1)<recnum)) return cpos+1;
                else                                 return -1;
            }
            else {
                if(((cpos-1)>=0)&&((cpos-1)<recnum)) return cpos-1;
                else                                 return -1;
            }
        }
        else if(cmp > 0) {
            if(cpos == lpos) {
                if((mode==LE)||(mode==LT)||(mode==EQ)) {
                    return -1;
                }
                else {
                    if((cpos>=0)&&(cpos<recnum)) return cpos;
                    else                         return -1;
                }
            }
            lpos = cpos;
            cpos = (fpos+lpos)/2;
        }
        else if(cmp < 0) {
            if(cpos == fpos) {
                cmp = compar(base+(recsize*lpos),sd);
                if(cmp == 0) {
                    if((mode==LE)||(mode==GE)||(mode==EQ)) {
                        if((lpos>=0)&&(lpos<recnum)) return lpos;
                        else                         return -1;
                    }
                    else if(mode==LT) {
                        if(((lpos-1)>=0)&&((lpos-1)<recnum)) return lpos-1;
                        else                                 return -1;
                    }
                    else {
						return -1;
                    }
                }
                else if(cmp > 0) {
                    if((mode==LE)||(mode==LT)) {
                        if(((lpos-1)>=0)&&((lpos-1)<recnum)) return lpos-1;
                        else                                 return -1;
                    }
                    else if((mode==GE)||(mode==GT)) {
                        if(((cpos+1)>=0)&&((cpos+1)<recnum)) return cpos+1;
                        else                                 return -1;
                    }
                    else {
                        return -1;
                    }
                }
                else {
                    if((mode==LE)||(mode==LT)) {
                        if(((cpos+1)>=0)&&((cpos+1)<recnum)) return cpos+1;
                        else                                 return -1;
                    }
                    else if((mode==GE)||(mode==GT)) {
                        return -1;
                    }
                    else {
                        return -1;
                    }
                }
            }
            fpos = cpos;
            cpos = (fpos+lpos)/2;
        }
    }
}
/*************************************************************************
	End of Program (ltou.c)
*************************************************************************/
