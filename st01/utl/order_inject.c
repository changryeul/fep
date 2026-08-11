/*------------------------------------------------------------------------
#   Module  : 주문 테스트 데이터 주입 도구
#   File    : order_inject.c
#   Author  : Test Utility
#
#   내부 클라이언트(Noa) 역할을 시뮬레이션하여
#   pa_8200_tr에 TCP로 접속, CLI 프로토콜로 주문 데이터를 전송한다.
#
#   동작 흐름:
#     1) pa_8200_tr 프로세스에 TCP 접속
#     2) LINK(로그인) 전송 → LIOK 응답 확인
#     3) DATA(주문) 전송 → DAOK 응답 확인
#     4) 종료
#
#   주문 경로:
#     order_inject → pa_8200_tr → DSHM_W() → pa_1100_ts → KRX
#
#   Usage:
#     order_inject <IP> <PORT> <종목코드> [매수/매도] [수량] [가격] [건수] [간격ms]
#     order_inject 127.0.0.1 18200 KR7005930003
#     order_inject 127.0.0.1 18200 KR7005930003 1 100 98500
#     order_inject 127.0.0.1 18200 KR7005930003 2 100 98500 1000 10
#
#   주문별 OrderNo/SeqNo 자동 증가. 주문별 송신(IN)/DAOK수신(OUT) 시각을
#   order_inject.lat에 기록 (lat_report 입력 포맷). 반복 모드([건수]>1)는
#   진행/요약(RTT min/avg/max) 출력 추가.
#
#   Note: 실 거래소 연결 없이 pa_8200_tr까지의 경로를 테스트
------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

/*------------------------------------------------------------------------
    CLI 프로토콜 헤더 (cli_interface.h 참조)
------------------------------------------------------------------------*/
#define CLI_HEAD_LEN    50
#define CLI_DATA_LEN    500

typedef struct {
    char    Length[4];           /* 데이터 길이              */
    char    MsgType[4];         /* LINK/DATA/POLL           */
    char    ResponsCode[4];     /* 응답코드 0000=정상       */
    char    TradeDate[8];       /* 거래일자 YYYYMMDD        */
    char    SeqNo[8];           /* 시퀀스 번호              */
    char    Filler[22];         /* 예비                     */
} CLI_HEAD;

/*------------------------------------------------------------------------
    KRX 채권일반호가 (pa_struct.h 참조, 254 bytes)
------------------------------------------------------------------------*/
typedef struct {
    char DataSeq                 [11];  /* 메세지일련번호          */
    char Transaction_Code        [11];  /* 트랜잭션코드            */
    char Megrp_no                [2];   /* ME그룹번호              */
    char Undly_Asset_Mkt_Id      [3];   /* 시장ID                  */
    char Board_id                [2];   /* 보드ID                  */
    char MembershipNo            [5];   /* 회원번호                */
    char BranchNo                [5];   /* 지점번호                */
    char OrderNo                 [10];  /* 주문ID                  */
    char OriginalOrderNo         [10];  /* 원주문ID                */
    char ItemCode                [12];  /* 종목코드                */
    char TradeFlag               [1];   /* 매도매수구분 1=매도 2=매수 */
    char New_Modify_Cancel_gbn   [1];   /* 정정취소구분 1=신규     */
    char AccountNo               [12];  /* 계좌번호                */
    char OrderQuantity           [10];  /* 호가수량                */
    char Price                   [11];  /* 호가가격                */
    char Order_Type              [1];   /* 호가유형코드            */
    char Order_Condition         [1];   /* 호가조건코드            */
    char Ask_Type                [2];   /* 매도유형코드            */
    char Trust_Principal_Type    [2];   /* 위탁자기구분코드        */
    char Trust_Company_No        [5];   /* 위탁사번호              */
    char Account_Type            [2];   /* 계좌구분코드            */
    char Country_Code            [3];   /* 국가코드                */
    char Investor_Type           [4];   /* 투자자구분코드          */
    char Filler                  [6];   /* 필러값                  */
    char Foreign_Investor_Type   [2];   /* 외국인투자자구분코드    */
    char Sm_Bond_Part_Yn         [1];   /* 소액채권장종료매매참여  */
    char Tax_Exempt_Yn           [1];   /* 비과세여부              */
    char Order_Mesia_Type        [1];   /* 주문매체구분코드        */
    char Order_Identi            [12];  /* 주문자식별정보          */
    char Mac_Addr                [12];  /* MAC주소                 */
    char Order_Date              [8];   /* 호가일자                */
    char Member_Send_Time        [9];   /* 회원사주문시각          */
    char MembershipItem          [60];  /* 회원사용영역            */
    char Trdr_Id                 [5];   /* 거래원번호              */
    char Mm_Order_Type_No        [11];  /* 시장조성자호가구분번호  */
} KRX_NOTE_JUMUN_DATA;          /* 254 bytes */

/*------------------------------------------------------------------------
    SEARCH_HEADER (pa_struct.h 참조, 50 bytes)
------------------------------------------------------------------------*/
typedef struct {
    char    TrCode[6];          /* TrCode (100100 등)       */
    char    Scr_key[4];         /* 화면키                   */
    char    ErrCode[4];         /* Error Code               */
    char    ApType_Cd[5];       /* ApType Code              */
    char    Media_gbn[1];       /* 매체구분(A/C/T)          */
    char    Filler[30];         /* 예비                     */
} SEARCH_HEADER;

/*------------------------------------------------------------------------
    Utility functions
------------------------------------------------------------------------*/
static void right_align(char *dst, int len, const char *src)
{
    int slen = strlen(src);
    memset(dst, ' ', len);
    if (slen >= len)
        memcpy(dst, src, len);
    else
        memcpy(dst + len - slen, src, slen);
}

static void zero_pad(char *dst, int len, int val)
{
    char tmp[32];
    sprintf(tmp, "%0*d", len, val);
    memcpy(dst, tmp, len);
}

static int tcp_send(int fd, const char *buf, int len)
{
    int sent = 0, rt;
    while (sent < len) {
        rt = send(fd, buf + sent, len - sent, 0);
        if (rt <= 0) return -1;
        sent += rt;
    }
    return sent;
}

static int tcp_recv(int fd, char *buf, int maxlen)
{
    int rt = recv(fd, buf, maxlen, 0);
    return rt;
}

static void get_today(char *buf8)
{
    time_t now;
    struct tm *t;
    time(&now);
    t = localtime(&now);
    snprintf(buf8, 64, "%04d%02d%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
}

static void get_time9(char *buf9)
{
    time_t now;
    struct tm *t;
    struct timespec ts;
    time(&now);
    t = localtime(&now);
    clock_gettime(CLOCK_REALTIME, &ts);
    snprintf(buf9, 64, "%02d%02d%02d%03ld",
            t->tm_hour, t->tm_min, t->tm_sec, ts.tv_nsec / 1000000);
}

/*------------------------------------------------------------------------
    Main
------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int     sockfd, rt;
    struct  sockaddr_in serv_addr;
    char    send_buf[4096], recv_buf[4096];
    char    today[64], time9[64];

    /* 파라미터 */
    const char *ip;
    int         port;
    const char *item_code;
    int         trade_flag  = 2;            /* default: 매수 */
    int         quantity    = 100;           /* default: 100  */
    int         price       = 100000;        /* default: 100000 */
    int         count       = 1;             /* default: 1건    */
    int         interval_ms = 0;             /* default: 0ms    */

    if (argc < 4) {
        printf("=========================================================\n");
        printf(" 주문 테스트 데이터 주입 도구 (order_inject)\n");
        printf("=========================================================\n");
        printf(" Usage:\n");
        printf("   order_inject <IP> <PORT> <종목코드> [매수매도] [수량] [가격] [건수] [간격ms]\n");
        printf("\n");
        printf(" Parameters:\n");
        printf("   IP         : pa_8200_tr 프로세스 IP (예: 127.0.0.1)\n");
        printf("   PORT       : pa_8200_tr 프로세스 PORT (예: 18200)\n");
        printf("   종목코드   : KRX 12자리 (예: KR7005930003)\n");
        printf("   매수매도   : 1=매도, 2=매수 (default: 2)\n");
        printf("   수량       : 주문수량 (default: 100)\n");
        printf("   가격       : 주문가격 (default: 100000)\n");
        printf("   건수       : 반복 주입 건수 (default: 1)\n");
        printf("   간격ms     : 주문 간 간격 밀리초 (default: 0)\n");
        printf("\n");
        printf(" Example:\n");
        printf("   order_inject 127.0.0.1 18200 KR7005930003\n");
        printf("   order_inject 127.0.0.1 18200 KR7005930003 1 500 95000\n");
        printf("   order_inject 127.0.0.1 18200 KR7005930003 2 100 98500 1000 10\n");
        printf("\n");
        printf(" Flow:\n");
        printf("   order_inject → pa_8200_tr → DSHM → pa_1100_ts → KRX\n");
        printf("=========================================================\n");
        return 1;
    }

    ip        = argv[1];
    port      = atoi(argv[2]);
    item_code = argv[3];
    if (argc > 4) trade_flag = atoi(argv[4]);
    if (argc > 5) quantity   = atoi(argv[5]);
    if (argc > 6) price      = atoi(argv[6]);
    if (argc > 7) count      = atoi(argv[7]);
    if (argc > 8) interval_ms = atoi(argv[8]);

    if (count < 1) count = 1;
    if (interval_ms < 0) interval_ms = 0;

    get_today(today);
    today[8] = '\0';

    printf("[INFO] 접속: %s:%d\n", ip, port);
    printf("[INFO] 종목: %.12s  매수매도: %d  수량: %d  가격: %d\n",
            item_code, trade_flag, quantity, price);

    /*------------------------------------------------------------------
        1) TCP 접속
    ------------------------------------------------------------------*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("[ERROR] socket 생성 실패: %s\n", strerror(errno));
        return 1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);

    rt = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (rt < 0) {
        printf("[ERROR] 접속 실패 %s:%d - %s\n", ip, port, strerror(errno));
        close(sockfd);
        return 1;
    }
    printf("[OK] TCP 접속 성공\n");

    /*------------------------------------------------------------------
        2) LINK (로그인) 전송
    ------------------------------------------------------------------*/
    {
        CLI_HEAD *h;
        int send_len;

        memset(send_buf, ' ', sizeof(send_buf));
        h = (CLI_HEAD *)send_buf;

        /* CLI_HEAD 설정 */
        memcpy(h->MsgType, "LINK", 4);
        memcpy(h->ResponsCode, "0000", 4);
        memcpy(h->TradeDate, today, 8);
        zero_pad(h->SeqNo, 8, 1);

        /* Length = 전체크기 - 4 (Length필드 자체 제외)
         * Select_Receive_Cli: 먼저 4바이트(Length) 읽고, 그 값만큼 추가 수신
         * LINK는 데이터 없으므로: CLI_HEAD_LEN - 4 = 46 */
        send_len = CLI_HEAD_LEN;
        zero_pad(h->Length, 4, send_len - 4);

        rt = tcp_send(sockfd, send_buf, send_len);
        if (rt < 0) {
            printf("[ERROR] LINK 전송 실패\n");
            close(sockfd);
            return 1;
        }
        printf("[SEND] LINK 전송 (%d bytes)\n", rt);

        /* 응답 대기 */
        memset(recv_buf, 0, sizeof(recv_buf));
        rt = tcp_recv(sockfd, recv_buf, sizeof(recv_buf));
        if (rt <= 0) {
            printf("[ERROR] LINK 응답 수신 실패\n");
            close(sockfd);
            return 1;
        }

        h = (CLI_HEAD *)recv_buf;
        printf("[RECV] 응답: MsgType=%.4s ResponsCode=%.4s (%d bytes)\n",
                h->MsgType, h->ResponsCode, rt);

        if (memcmp(h->MsgType, "LIOK", 4) != 0) {
            printf("[ERROR] LINK 응답이 LIOK가 아님\n");
            close(sockfd);
            return 1;
        }
        printf("[OK] 로그인 성공\n");
    }

    /*------------------------------------------------------------------
        3) DATA (주문) 전송
           pa_8200_tr의 Write_Data()는 Data 앞 3자리로 분배:
           "100xxx" → DSHM_W → pa_1100_ts (주문)
    ------------------------------------------------------------------*/
    {
        CLI_HEAD            *h;
        SEARCH_HEADER       *sh;
        KRX_NOTE_JUMUN_DATA *order;
        int     send_len, data_len;
        char    *data_ptr;
        FILE    *lat_fp;
        struct timeval tv;
        long long t_send, t_ack, rtt, rtt_min, rtt_max, rtt_sum;
        int     k, ok_cnt, fail_cnt;
        char    ordno_buf[16];

        memset(send_buf, ' ', sizeof(send_buf));
        h = (CLI_HEAD *)send_buf;
        data_ptr = send_buf + CLI_HEAD_LEN;

        /* SEARCH_HEADER (50 bytes) */
        sh = (SEARCH_HEADER *)data_ptr;
        memcpy(sh->TrCode, "100100", 6);       /* 주문 TR */
        memset(sh->Scr_key, ' ', 4);
        memcpy(sh->ErrCode, "0000", 4);
        memcpy(sh->ApType_Cd, "A1101", 5);     /* PA1101 주문송신 */
        memcpy(sh->Media_gbn, "T", 1);         /* T=테스트 */
        memset(sh->Filler, ' ', 30);

        /* KRX_NOTE_JUMUN_DATA (254 bytes) */
        order = (KRX_NOTE_JUMUN_DATA *)(data_ptr + sizeof(SEARCH_HEADER));
        memset(order, ' ', sizeof(KRX_NOTE_JUMUN_DATA));

        /* DataSeq: 11자리, 주문송신에서 재설정됨 */
        right_align(order->DataSeq, 11, "00000000001");

        /* Transaction_Code: TCHODR40001 (채권일반호가 신규주문) */
        memcpy(order->Transaction_Code, "TCHODR40001", 11);

        /* ME그룹번호 */
        memcpy(order->Megrp_no, "01", 2);

        /* 시장ID: K=KTS */
        memcpy(order->Undly_Asset_Mkt_Id, "K  ", 3);

        /* 보드ID */
        memcpy(order->Board_id, "01", 2);

        /* 회원번호 */
        memcpy(order->MembershipNo, "99999", 5);

        /* 지점번호 */
        memcpy(order->BranchNo, "00001", 5);

        /* 주문ID (10자리, 우측정렬) */
        right_align(order->OrderNo, 10, "0000000001");

        /* 원주문ID */
        memset(order->OriginalOrderNo, ' ', 10);

        /* 종목코드 (12자리) */
        {
            char item_buf[13];
            memset(item_buf, ' ', 12);
            item_buf[12] = '\0';
            memcpy(item_buf, item_code, strlen(item_code) < 12 ? strlen(item_code) : 12);
            memcpy(order->ItemCode, item_buf, 12);
        }

        /* 매도매수구분: 1=매도, 2=매수 */
        order->TradeFlag[0] = '0' + trade_flag;

        /* 정정취소구분: 1=신규 */
        order->New_Modify_Cancel_gbn[0] = '1';

        /* 계좌번호 */
        memcpy(order->AccountNo, "100000000001", 12);

        /* 호가수량 (10자리, 우측정렬) */
        {
            char qty_buf[16];
            sprintf(qty_buf, "%d", quantity);
            right_align(order->OrderQuantity, 10, qty_buf);
        }

        /* 호가가격 (11자리, 우측정렬) */
        {
            char prc_buf[16];
            sprintf(prc_buf, "%d", price);
            right_align(order->Price, 11, prc_buf);
        }

        /* 호가유형코드: 1=지정가 */
        order->Order_Type[0] = '1';

        /* 호가조건코드: 0=없음 */
        order->Order_Condition[0] = '0';

        /* 위탁자기구분: 01=위탁 */
        memcpy(order->Trust_Principal_Type, "01", 2);

        /* 계좌구분: 01 */
        memcpy(order->Account_Type, "01", 2);

        /* 국가코드: KR */
        memcpy(order->Country_Code, "KR ", 3);

        /* 투자자구분: 1000=개인 */
        memcpy(order->Investor_Type, "1000", 4);

        /* 주문매체: T=테스트 */
        order->Order_Mesia_Type[0] = 'T';

        /* 호가일자 */
        memcpy(order->Order_Date, today, 8);

        /* 회원사주문시각 */
        get_time9(time9);
        memcpy(order->Member_Send_Time, time9, 9);

        /* 회원사용영역 (60 bytes) - 시장구분 offset+35 = "07" (KTS) */
        memset(order->MembershipItem, ' ', 60);
        memcpy(order->MembershipItem + 35, "07", 2);

        /* CLI HEAD 설정 */
        data_len = sizeof(SEARCH_HEADER) + sizeof(KRX_NOTE_JUMUN_DATA);
        send_len = CLI_HEAD_LEN + data_len;

        memcpy(h->MsgType, "DATA", 4);
        memcpy(h->ResponsCode, "0000", 4);
        memcpy(h->TradeDate, today, 8);
        zero_pad(h->SeqNo, 8, 2);
        zero_pad(h->Length, 4, send_len - 4);  /* Length필드 자체(4바이트) 제외 */

        printf("[INFO] 주문 전문 구성:\n");
        printf("       SEARCH_HEADER: %lu bytes (TrCode=%.6s)\n",
                (unsigned long)sizeof(SEARCH_HEADER), sh->TrCode);
        printf("       KRX_NOTE_JUMUN_DATA: %lu bytes\n",
                (unsigned long)sizeof(KRX_NOTE_JUMUN_DATA));
        printf("       Transaction_Code: %.11s\n", order->Transaction_Code);
        printf("       ItemCode: %.12s\n", order->ItemCode);
        printf("       TradeFlag: %.1s (1=매도,2=매수)\n", order->TradeFlag);
        printf("       OrderQuantity: %.10s\n", order->OrderQuantity);
        printf("       Price: %.11s\n", order->Price);
        printf("       전체 전송: %d bytes (CLI_HEAD=%d + Data=%d)\n",
                send_len, CLI_HEAD_LEN, data_len);
        if (count > 1)
            printf("[INFO] 반복 주입: %d건, 간격 %dms\n", count, interval_ms);

        /* RTT 기록 파일 (lat_report 입력 포맷) */
        lat_fp = fopen("order_inject.lat", "a");
        if (lat_fp == NULL)
            printf("[WARN] order_inject.lat 열기 실패 - RTT 기록 생략\n");

        ok_cnt = fail_cnt = 0;
        rtt_min = rtt_max = rtt_sum = 0;

        for (k = 0; k < count; k++) {
            /* 주문별 필드 갱신: OrderNo, SeqNo, 회원사주문시각 */
            sprintf(ordno_buf, "%010d", 1 + k);
            memcpy(order->OrderNo, ordno_buf, 10);
            zero_pad(h->SeqNo, 8, 2 + k);
            get_time9(time9);
            memcpy(order->Member_Send_Time, time9, 9);

            /* IN: 송신 직전 시각 기록 */
            gettimeofday(&tv, NULL);
            t_send = (long long)tv.tv_sec * 1000000LL + tv.tv_usec;
            if (lat_fp != NULL)
                fprintf(lat_fp, "%lld|order_inject|IN|%.10s\n", t_send, ordno_buf);

            rt = tcp_send(sockfd, send_buf, send_len);
            if (rt < 0) {
                printf("[ERROR] DATA 전송 실패 (%d번째)\n", k + 1);
                if (lat_fp != NULL) fclose(lat_fp);
                close(sockfd);
                return 1;
            }
            if (count == 1)
                printf("[SEND] DATA(주문) 전송 (%d bytes)\n", rt);

            /* 응답 대기 */
            memset(recv_buf, 0, sizeof(recv_buf));
            rt = tcp_recv(sockfd, recv_buf, sizeof(recv_buf));
            if (rt <= 0) {
                printf("[ERROR] DATA 응답 수신 실패 (%d번째)\n", k + 1);
                if (lat_fp != NULL) fclose(lat_fp);
                close(sockfd);
                return 1;
            }

            if (count == 1) {
                CLI_HEAD *rh = (CLI_HEAD *)recv_buf;
                printf("[RECV] 응답: MsgType=%.4s ResponsCode=%.4s (%d bytes)\n",
                        rh->MsgType, rh->ResponsCode, rt);
            }

            if (memcmp(((CLI_HEAD *)recv_buf)->MsgType, "DAOK", 4) == 0) {
                /* OUT: DAOK 수신 시각 기록 + RTT 집계 */
                gettimeofday(&tv, NULL);
                t_ack = (long long)tv.tv_sec * 1000000LL + tv.tv_usec;
                if (lat_fp != NULL)
                    fprintf(lat_fp, "%lld|order_inject|OUT|%.10s\n", t_ack, ordno_buf);

                rtt = t_ack - t_send;
                if (ok_cnt == 0 || rtt < rtt_min) rtt_min = rtt;
                if (ok_cnt == 0 || rtt > rtt_max) rtt_max = rtt;
                rtt_sum += rtt;
                ok_cnt++;

                if (count == 1)
                    printf("[OK] 주문 데이터 전달 성공 (pa_8200_tr → DSHM 큐)\n");
            }
            else {
                fail_cnt++;
                if (count == 1) {
                    printf("[WARN] 예상치 못한 응답: %.4s\n",
                            ((CLI_HEAD *)recv_buf)->MsgType);
                }
            }

            if (count > 1 && (k + 1) % 100 == 0)
                printf("[INFO] 진행: %d/%d (성공 %d, 실패 %d)\n",
                        k + 1, count, ok_cnt, fail_cnt);

            if (interval_ms > 0 && k + 1 < count)
                usleep((useconds_t)interval_ms * 1000);
        }

        if (lat_fp != NULL)
            fclose(lat_fp);

        if (count > 1) {
            printf("[DONE] 총 %d건: 성공 %d, 실패 %d\n", count, ok_cnt, fail_cnt);
            if (ok_cnt > 0)
                printf("[RTT ] min %lld us / avg %lld us / max %lld us"
                        " (ACK 기준, order_inject.lat 기록)\n",
                        rtt_min, rtt_sum / ok_cnt, rtt_max);
        }
    }

    /*------------------------------------------------------------------
        4) 종료
    ------------------------------------------------------------------*/
    close(sockfd);
    printf("[OK] 접속 종료\n");
    printf("\n");
    printf("=== 주문 추적 ===\n");
    printf("  1. pa_8200_tr 로그: Write_Data() → DSHM_W 확인\n");
    printf("  2. pa_1100_ts 로그: Make_Data_Block() → Device_Write() 확인\n");
    printf("  3. pa_1200_tr 로그: KRX 응답 수신 확인\n");

    return 0;
}

/*************************************************************************
    End of Program (order_inject.c)
*************************************************************************/
