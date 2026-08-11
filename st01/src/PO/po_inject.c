#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : po_1200_mp 데이터흐름 E2E용 주입기 (테스트 전용) — PO 모듈
#   File    : po_inject.c
#
#   'o' 부문 프로세스로서 po_1200_mp의 입력 큐(OFN_1=po_1200_mp)에 크래프트된
#   현·파 회원처리호가(TTRODP11301, 매체구분=C, 전략=0000) 1건을 F_W로 기록한다.
#   → 동일 'o' 부문이라 큐 카운터(w_cnt/r_cnt)가 공유되어 po_1200_mp가 소비 가능.
#   (pc_→po_ 크로스부문 파일큐는 부문별 SHM 카운터라 미동작 → 동일부문 주입으로 실증)
#
#   레코드 Data 레이아웃 (pc_1200_tr Write_Data(1)와 동형):
#     [0:15]  "000000000000000" (4 ErrCode + 11 seq 패딩)
#     [15:]   TTRODP11301 전문(318B): 전문[11]=TrCode, 전문[202]=회원사용영역(60)
#             회원사용영역[30]=매체구분('C'=Client), [47:51]=전략번호("0000"=없음)
------------------------------------------------------------------------*/
#include    "fep_fepp.h"
#include    "pa_struct.h"
#include    "krx_ttrodp11301.h"
#include    "seam_queue.h"      /* 전역 SEAM 스테이징 큐(pc_1200_tr 대역 주입) */

#define     DATA_SIZE       400
#include    "buf_struct.h"

int     main(int argc, char *argv[])
{
    FILE_BUFF_FORMAT    W;
    char                *d;
    char                *kind;
    int                 rt;

    Init_Proc(argc, argv);
    SEAM_Init();        /* 전역 SEAM 스테이징 큐 확보 */

    memset(&W, 0x20, sizeof(W));
    d = W.Data;

    /* 4+11 응답 패딩 (ErrCode "0000" — REJ 아님) */
    memcpy(&d[0], "000000000000000", 15);

    kind = getenv("PO_INJECT_KIND");
    if (kind != NULL && strcmp(kind, "bond_order") == 0) {
        /* --- 채권 주문 TCHODR40001 → SEAM_ORDQ_BOND (forwarder P2 검증용) ---
           전략(po_) emit 대역: wire 레코드(header70 + Data). Data[11]=TrCode.
           forwarder(pb_1109_mp)가 SEAM_ORD_R로 소비→송신부 입력으로 중계. */
        memcpy(&d[0],  "00000000001", 11);             /* DataSeq                 */
        memcpy(&d[11], "TCHODR40001", 11);             /* TrCode(채권 일반호가254)*/
        memcpy(&d[22], "00", 2);                       /* Megrp                   */
        memcpy(&d[39], "0000000001", 10);              /* OrderNo(관측용)         */
        W.LineFeed[0] = '\n';
        rt = SEAM_ORD_W(SEAM_ORDQ_BOND, (void *)&W, SEAM_ORD_WIRE_RECSZ);
        if (rt != 1) { Log(SAM_FATAL, "po_inject: SEAM_ORD_W fail q=%d rt=%d", SEAM_ORDQ_BOND, rt); Exit_Process(); }
        Log(USR_OK, "po_inject: wrote 1 TCHODR40001(채권주문) to SEAM_ORDQ_BOND");
        Exit_Process();
    }

    if (kind != NULL && strcmp(kind, "bond_settle") == 0) {
        /* --- 채권 체결 TTRTDP42301 (po_1490_mp 체결원장: 미체결 잔량 감소용) ---
           KRX_NOTE_SETTLE_DATA 오프셋(전문 시작 = d[0], 패딩 없음):
             TrCode@11, OrderNo@39, Trading_Volumn@106(che_cnt=AtoIf(&[1],9)),
             TradeFlag@135, MembershipItem@232: 시장[+35] item_seq[+37] acc_seq[+42] */
        memcpy(&d[0],   "00000000002", 11);            /* DataSeq              */
        memcpy(&d[11],  "TTRTDP42301", 11);            /* TrCode(채권 체결)    */
        memcpy(&d[39],  "0000000001", 10);             /* OrderNo(미체결과 일치)*/
        memcpy(&d[106], "0000000100", 10);             /* Trading_Volumn=100(전량체결) */
        d[135] = '2';                                  /* TradeFlag 매수       */
        memset(&d[232], ' ', 60);                      /* MembershipItem       */
        d[232 + 35] = '0';                             /* 시장구분=0(채권)     */
        memcpy(&d[232 + 37], "00001", 5);              /* item_seq=1           */
        memcpy(&d[232 + 42], "01", 2);                 /* acc_seq=1            */
        W.LineFeed[0] = '\n';
        rt = SEAM_W(SEAM_Q_EXEC, (void *)&W, (int)sizeof(FILE_BUFF_FORMAT));
        if (rt != 1) { Log(SAM_FATAL, "po_inject: SEAM_W fail q=%d rt=%d", SEAM_Q_EXEC, rt); Exit_Process(); }
        Log(USR_OK, "po_inject: wrote 1 TTRTDP42301(채권체결 OrderNo=0000000001 수량100) to SEAM_Q_EXEC");
        Exit_Process();
    }

    if (kind != NULL && strcmp(kind, "bond_miche") == 0) {
        /* --- 채권 회원처리호가 TTRODP41301 (po_1290_mp 미체결 등록용) ---
           KRX_NOTE_SETTLE_RESP_DATA 오프셋(전문 시작 = d[15]):
             TrCode@11, OrderNo@39, ItemCode@59, TradeFlag@71,
             New_Modify_Cancel_gbn@72, AccountNo@73, OrderQuantity@85,
             MembershipItem@191: 매체[30] 시장[35] item_seq[37..42] acc_seq[42..44] */
        memcpy(&d[15 + 0],   "00000000001", 11);       /* DataSeq              */
        memcpy(&d[15 + 11],  "TTRODP41301", 11);       /* TrCode(채권일반확인) */
        memcpy(&d[15 + 39],  "0000000001", 10);        /* OrderNo              */
        memcpy(&d[15 + 59],  "KR6000000001", 12);      /* ItemCode(채권 예시)  */
        d[15 + 71] = '2';                              /* TradeFlag 매수       */
        d[15 + 72] = '1';                              /* 신규(1)              */
        memcpy(&d[15 + 73],  "100000000001", 12);      /* AccountNo            */
        memcpy(&d[15 + 85],  "0000000100", 10);        /* OrderQuantity=100    */
        memset(&d[15 + 191], ' ', 60);                 /* MembershipItem       */
        d[15 + 191 + 30] = 'C';                        /* 매체=Client(자사)    */
        d[15 + 191 + 35] = '0';                        /* 시장구분=0(채권)     */
        memcpy(&d[15 + 191 + 37], "00001", 5);         /* item_seq=1           */
        memcpy(&d[15 + 191 + 42], "01", 2);            /* acc_seq=1            */
        W.LineFeed[0] = '\n';
        rt = SEAM_W(SEAM_Q_RESP, (void *)&W, (int)sizeof(FILE_BUFF_FORMAT));
        if (rt != 1) { Log(SAM_FATAL, "po_inject: SEAM_W fail q=%d rt=%d", SEAM_Q_RESP, rt); Exit_Process(); }
        Log(USR_OK, "po_inject: wrote 1 TTRODP41301(채권회원처리호가 신규 매체=C 수량100) to SEAM_Q_RESP");
        Exit_Process();
    }

    if (kind != NULL && strcmp(kind, "deriv_settle") == 0) {
        /* --- 현·파 체결 TTRTDP21301 (po_1490_mp Analyze_Che_Deriv: 파생 미체결 감소용) ---
           전문 시작 = d[0](체결은 15패딩 없음). 오프셋 = TTRTDP21301_DATA struct 순서:
             TrCode@11, Order_Identification@36, Issue_Code@56, Trading_Price@79,
             Trading_Volumn@90, Ask_Bid_Type_Code@141, Account_Number@142,
             Member_Use_Area@172(시장35/종목37/계좌42) */
        memcpy(&d[0],   "00000000003", 11);            /* DataSeq                 */
        memcpy(&d[11],  "TTRTDP21301", 11);            /* TrCode(현·파 체결)      */
        memcpy(&d[36],  "0000000002", 10);             /* Order_Identification(미체결 일치) */
        memcpy(&d[56],  "KR4101SC0009", 12);           /* Issue_Code(파생)        */
        memcpy(&d[79],  "00000010000", 11);            /* Trading_Price           */
        memcpy(&d[90],  "0000000100", 10);             /* Trading_Volumn=100(전량)*/
        d[141] = '2';                                  /* Ask_Bid_Type_Code 매수  */
        memcpy(&d[142], "200000000002", 12);           /* Account_Number          */
        memset(&d[172], ' ', 60);                      /* Member_Use_Area         */
        d[172 + 35] = '1';                             /* 시장구분=1(파생)        */
        memcpy(&d[172 + 37], "00001", 5);              /* 종목 seq=1              */
        memcpy(&d[172 + 42], "01", 2);                 /* 계좌 seq=1              */
        W.LineFeed[0] = '\n';
        rt = SEAM_W(SEAM_Q_EXEC, (void *)&W, (int)sizeof(FILE_BUFF_FORMAT));
        if (rt != 1) { Log(SAM_FATAL, "po_inject: SEAM_W fail q=%d rt=%d", SEAM_Q_EXEC, rt); Exit_Process(); }
        Log(USR_OK, "po_inject: wrote 1 TTRTDP21301(파생 체결 OrderNo=0000000002 수량100 mk=1) to SEAM_Q_EXEC");
        Exit_Process();
    }

    if (kind != NULL && strcmp(kind, "deriv_autocxl") == 0) {
        /* --- 현·파 자동취소 TTRODP11303 (po_1290_mp 자동취소 미체결 감소용) ---
           레이아웃 = TTRODP11301_DATA(11301/11321/11303 동일). 자동취소는
           Real_Modify_Or_Cancel_Order_Quantity(@271) 수량만큼 미체결 잔량 감소. */
        memcpy(&d[15 + 0],   "00000000004", 11);       /* Message_Sequence_Number */
        memcpy(&d[15 + 11],  "TTRODP11303", 11);       /* Transaction_Code(자동취소) */
        memcpy(&d[15 + 36],  "0000000002", 10);        /* Order_Identification(등록 일치) */
        memcpy(&d[15 + 56],  "KR4101SC0009", 12);      /* Issue_Code_Symbol       */
        d[15 + 68] = '2';                              /* Ask_Bid_Type_Code 매수  */
        d[15 + 69] = '1';                              /* Modify_Or_Cancel 신규   */
        memcpy(&d[15 + 70],  "200000000002", 12);      /* Account_Number          */
        memcpy(&d[15 + 92],  "00000010000", 11);       /* Order_Price @92         */
        memcpy(&d[15 + 271], "0000000100", 10);        /* Real_Modify_Or_Cancel_Order_Quantity @271 =100 */
        memset(&d[15 + 202], ' ', 60);                 /* Member_Use_Area @202    */
        d[15 + 202 + 30] = 'C';
        d[15 + 202 + 35] = '1';                        /* 시장구분=1(파생)        */
        memcpy(&d[15 + 202 + 37], "00001", 5);
        memcpy(&d[15 + 202 + 42], "01", 2);
        W.LineFeed[0] = '\n';
        rt = SEAM_W(SEAM_Q_RESP, (void *)&W, (int)sizeof(FILE_BUFF_FORMAT));
        if (rt != 1) { Log(SAM_FATAL, "po_inject: SEAM_W fail q=%d rt=%d", SEAM_Q_RESP, rt); Exit_Process(); }
        Log(USR_OK, "po_inject: wrote 1 TTRODP11303(파생 자동취소 OrderNo=0000000002 수량100 mk=1) to SEAM_Q_RESP");
        Exit_Process();
    }

    if (kind != NULL && strcmp(kind, "deriv_miche") == 0) {
        /* --- 현·파 회원처리호가 TTRODP11301 (po_1290_mp Make_MiChe_Deriv 등록용) ---
           전문 시작 = d[15]. 오프셋 = TTRODP11301_DATA struct 순서(항목영문명).
             TrCode@11, Order_Identification@36, Issue_Code_Symbol@56,
             Ask_Bid_Type_Code@68, Modify_Or_Cancel_Type_Code@69, Account_Number@70,
             Order_Quantity@82, Order_Price@93, Order_Type_Code@104,
             Order_Condition_Code@105, Member_Use_Area@203(매체30/시장35/종목37/계좌42) */
        memcpy(&d[15 + 0],   "00000000001", 11);       /* Message_Sequence_Number */
        memcpy(&d[15 + 11],  "TTRODP11301", 11);       /* Transaction_Code(정상)  */
        memcpy(&d[15 + 36],  "0000000002", 10);        /* Order_Identification    */
        memcpy(&d[15 + 56],  "KR4101SC0009", 12);      /* Issue_Code_Symbol(파생) */
        d[15 + 68] = '2';                              /* Ask_Bid_Type_Code 매수  */
        d[15 + 69] = '1';                              /* Modify_Or_Cancel 신규   */
        memcpy(&d[15 + 70],  "200000000002", 12);      /* Account_Number          */
        memcpy(&d[15 + 82],  "0000000100", 10);        /* Order_Quantity=100      */
        memcpy(&d[15 + 92],  "00000010000", 11);       /* Order_Price @92         */
        d[15 + 103] = '2';                             /* Order_Type_Code @103 지정가 */
        d[15 + 104] = '0';                             /* Order_Condition @104 일반   */
        memset(&d[15 + 202], ' ', 60);                 /* Member_Use_Area @202(60)*/
        d[15 + 202 + 30] = 'C';                        /* 매체=Client(자사)       */
        d[15 + 202 + 35] = '1';                        /* 시장구분=1(파생)        */
        memcpy(&d[15 + 202 + 37], "00001", 5);         /* 종목 seq=1              */
        memcpy(&d[15 + 202 + 42], "01", 2);            /* 계좌 seq=1              */
        W.LineFeed[0] = '\n';
        rt = SEAM_W(SEAM_Q_RESP, (void *)&W, (int)sizeof(FILE_BUFF_FORMAT));
        if (rt != 1) { Log(SAM_FATAL, "po_inject: SEAM_W fail q=%d rt=%d", SEAM_Q_RESP, rt); Exit_Process(); }
        Log(USR_OK, "po_inject: wrote 1 TTRODP11301(파생 회원처리호가 신규 매체=C 수량100 mk=1) to SEAM_Q_RESP");
        Exit_Process();
    }

    /* --- 기본: 현·파 회원처리호가 TTRODP11301 (po_1200_mp 분배용) --- */
    memcpy(&d[15 + 0],  "00000000001", 11);        /* Message_Sequence_Number */
    memcpy(&d[15 + 11], "TTRODP11301", 11);        /* Transaction_Code        */
    memcpy(&d[15 + 22], "00", 2);                  /* Me_Grp_No               */
    memcpy(&d[15 + 24], "G1", 2);                  /* Board_Id                */
    memset(&d[15 + 202], ' ', 60);                 /* 회원사용영역(전문 202)  */
    d[15 + 202 + 30] = 'C';                        /* 매체구분 = Client       */
    memcpy(&d[15 + 202 + 47], "0000", 4);          /* 전략번호 = 없음         */

    W.LineFeed[0] = '\n';

    rt = SEAM_W(SEAM_Q_RESP, (void *)&W, (int)sizeof(FILE_BUFF_FORMAT));
    if (rt != 1) {
        Log(SAM_FATAL, "po_inject: SEAM_W fail q=%d rt=%d", SEAM_Q_RESP, rt);
        Exit_Process();
    }
    Log(USR_OK, "po_inject: wrote 1 TTRODP11301(매체=C) to SEAM_Q_RESP");

    Exit_Process();
}

/*************************************************************************
    End of Program (po_inject.c)
*************************************************************************/
