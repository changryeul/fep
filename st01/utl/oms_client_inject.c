#define     _GLOBAL     /* FEP 전역(D_K/Shm_Mem/_FEP_* 등) 정의 (FEP 프로세스 관례) */
/*------------------------------------------------------------------------
#   oms_client_inject.c — 전략기동 클라 요청(500100) 주입기 (9000_mp full-loop Inc2)
#
#   'o' 부문 FEP 프로세스로서 OFN_1=po_9000_in(=po_9000_mp 의 IFN_1)에 500100 요청을
#   F_W 로 1건 기록한다. po_9000_mp 가 F_R(PS_R_1)로 읽어 Start_Client 로 전략 슬롯을
#   할당(cp+DTART)한다. 레코드 = FILE_BUFF_FORMAT, Data 앞=SEARCH_HEADER.
#     SEARCH_HEADER: TrCode[6]@0, Scr_key[4], ErrCode[4], ApType_Cd[5]@14 ...
#
#   env OMS_APTYPE=전략번호(5010~5999, 기본 5050=채권LP → Strategy_Letter 'b' → pb_5050_mp)
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#define     DATA_SIZE   2048        /* po_9000_mp 와 동일(F_W/F_R 레코드 크기 일치) */
#include    "buf_struct.h"

FILE_BUFF_FORMAT    W;

int     main(int argc, char *argv[])
{
    char   *aptype;
    int     rt;

    Init_Proc(argc, argv);

    aptype = getenv("OMS_APTYPE");
    if (aptype == NULL) aptype = "5050";

    memset(&W, ' ', sizeof (FILE_BUFF_FORMAT));
    memcpy(W.Data,        "500100", 6);        /* SEARCH_HEADER.TrCode      */
    memcpy(&W.Data[14],   aptype,   4);        /* SEARCH_HEADER.ApType_Cd   */
    W.LineFeed[0] = '\n';

    rt = F_W(TS_W1_1, (void *)&W, 1);
    if (rt != 1) {
        Log(SAM_FATAL, "oms_client_inject: F_W fail rt=%d", rt);
        Exit_Process();
    }
    Log(USR_OK, "oms_client_inject: 500100 ApType_Cd=[%.4s] -> po_9000_in", aptype);
    Exit_Process();
    return (0);
}
