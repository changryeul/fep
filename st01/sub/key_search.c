/*------------------------------------------------------------------------
#   Module  : key search
#   File    : key_search.c
------------------------------------------------------------------------*/
#include    "fep_sub.h"

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/

/*************************************************************************
    Function        : .
    Parameters IN   : . market      : 시장구분 Market Gbn
                      . kind            : 키구분 KS_EXPCODE
    Parameters OUT  : . index       : index, 음수이면 에러, 0이상 이면 정상
    Return Code     : . char * (string converted)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Key_Search(int mk_gbn, int kind, char *keybuf)
/*----------------------------------------------------------------------*/
{
    int             totcnt=0;                               /* 총종목수 */
    int             pos;
    KS_EXPCODE      *key1;                                  /* KEY 1    */
    KS_NOTE_EXPCODE *key2;                                  /* KEY 2,채권*/

    /*------------------------------------------------------------------*/
    /* 1. Key영역을 검색하여 시작 위치를 구한다.                        */
    /*------------------------------------------------------------------*/
    if (mk_gbn == NOTE_MK_KTS)                          /* NOTE_MK_KTS */ {
        totcnt = Shm_Note[0].total_item_cnt;
        key2 = (KS_NOTE_EXPCODE *)(&Shm_Item[0].N_Key);

        pos = BsearchMode((char *)key2, keybuf, totcnt, sizeof(KS_EXPCODE), CmpExpcode, EQ);

        if (pos < 0 || key2[pos].idx < 0)
            return NOTOK;
        else {
            return  key2[pos].idx;
        }
    }
    else if (mk_gbn == DEV_MK_FIF)                      /* DEV_MK_FIF */ {
        totcnt = Shm_FinFut[0].total_item_cnt;
        key1 = (KS_EXPCODE *)(&Shm_Item[0].D_Key);

        pos = BsearchMode((char *)key1, keybuf, totcnt, sizeof(KS_EXPCODE), CmpExpcode, EQ);

        if (pos < 0 || key1[pos].idx < 0)
            return NOTOK;
        else {
            return  key1[pos].idx;
        }
    }
    else
        return NOTOK;

}   /* End of Key_Search () */

/******************************************************************************
 * * FUNCTION:    CmpIDXMasterShcode
 * * DESCRIPTION: Qrery key 비교함수.
 * * PARAMETERS:
 * *              const void *a - Compare memory A
 * *              const void *b - Compare memory B
 * * RETURNED:    compare value
 * ******************************************************************************/
int CmpExpcode(const void *a, const void *b) {
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
int CmpLongcode(const void *a, const void *b) {
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
int BsearchMode(char *base, char *sd, int recnum, int recsize, int(*compar)(), int mode) {
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
    End of Program (key_search.c)
*************************************************************************/
