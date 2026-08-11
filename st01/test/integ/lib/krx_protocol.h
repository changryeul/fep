/*------------------------------------------------------------------------
#   Module  : KRX Protocol Message Builder/Parser
#   File    : krx_protocol.h
#   Purpose : Reusable library for building and parsing KRX messages.
#             Based on pa_struct.h KRX_HEADER (82B) + KRX_BODY_COMMON (24B).
------------------------------------------------------------------------*/
#ifndef KRX_PROTOCOL_H
#define KRX_PROTOCOL_H

/*------------------------------------------------------------------------
    Constants (matching pa_struct.h)
------------------------------------------------------------------------*/
#define KRX_HDR_LEN         82      /* sizeof(KRX_HEADER) */
#define KRX_BODY_COMMON_LEN 24      /* sizeof(KRX_BODY_COMMON) */
#define KRX_MSG_COMMON_LEN  106     /* KRX_HDR_LEN + KRX_BODY_COMMON_LEN */

/* KRX_HEADER field offsets */
#define KRX_OFF_BEGIN       0       /* BeginString[8]       */
#define KRX_OFF_BODYLEN     8       /* BodyLength[6]        */
#define KRX_OFF_MSGTYPE     14      /* MsgType[11]          */
#define KRX_OFF_MSGSEQNUM   25      /* MsgSeqNum[11]        */
#define KRX_OFF_SENDER      36      /* SenderCompID[5]      */
#define KRX_OFF_DELIVER     41      /* DeliverToCompID[10]  */
#define KRX_OFF_ONBEHALF    51      /* OnBehalfOfCompID[10] */
#define KRX_OFF_SENDTIME    61      /* SendingTime[17]      */
#define KRX_OFF_DATACNT     78      /* DataCnt[3]           */
#define KRX_OFF_ENCRYPT     81      /* Encrypt[1]           */

/* KRX_BODY_COMMON field offsets (relative to body start = 82) */
#define KRX_OFF_DATASEQ     82      /* DataSeq[11]          */
#define KRX_OFF_TRCODE      93      /* Transaction_Code[11] */
#define KRX_OFF_MEGRP       104     /* Megrp_no[2]          */

/* KRX Session message types (MsgType field, 11 bytes) */
#define KRX_LOGON_REQ       "SCHLIQ00000"
#define KRX_LOGON_RESP      "SCHLIR00000"
#define KRX_LINK_REQ        "SCHOPQ00000"
#define KRX_LINK_RESP       "SCHOPR00000"
#define KRX_POLL_REQ        "SCHHEQ00000"
#define KRX_POLL_RESP       "SCHHER00000"
#define KRX_LOGOFF_REQ      "SCHLOQ00000"
#define KRX_LOGOFF_RESP     "SCHLOR00000"

/* BeginString */
#define KRX_BEGIN_STRING    "FEPKRX01"

/* Default sender */
#define KRX_DEFAULT_SENDER  "00012"

/* Session logon data (41 bytes: proc_info[10] + proc_uid[30] + encrypt[1]) */
#define KRX_SESSION_DATA_LEN  41

/*------------------------------------------------------------------------
    Parsed header structure (host-side, null-terminated strings)
------------------------------------------------------------------------*/
typedef struct {
    char    BeginString[9];
    char    BodyLength[7];
    char    MsgType[12];
    char    MsgSeqNum[12];
    char    SenderCompID[6];
    char    DeliverToCompID[11];
    char    OnBehalfOfCompID[11];
    char    SendingTime[18];
    char    DataCnt[4];
    char    Encrypt[2];
    int     body_length;        /* parsed integer value of BodyLength */
} KRX_PARSED_HEADER;

/*------------------------------------------------------------------------
    API: Message Builders
    All return the total message length on success, -1 on error.
    buf must be at least bufsize bytes.
------------------------------------------------------------------------*/

/* Session messages (header only, no body) */
int  krx_build_logon_req(char *buf, int bufsize,
                         const char *sender_id, const char *proc_info);
int  krx_build_logon_resp(char *buf, int bufsize,
                          const char *sender_id, int success);
int  krx_build_link_req(char *buf, int bufsize, const char *sender_id);
int  krx_build_link_resp(char *buf, int bufsize, const char *sender_id);
int  krx_build_poll_req(char *buf, int bufsize, const char *sender_id);
int  krx_build_poll_resp(char *buf, int bufsize, const char *sender_id);
int  krx_build_logoff_req(char *buf, int bufsize, const char *sender_id);

/* Data message (header + body_common + payload) */
int  krx_build_order_msg(char *buf, int bufsize,
                         const char *sender_id,
                         const char *tr_code,
                         const char *data, int data_len);

/* KRX→회원 데이터 push (수신 프로세스 검증용): MsgType/MsgSeqNum/ME그룹seq 직접 지정.
   body = BODY_COMMON(24: DataSeq=meg_seq, TrCode, Megrp="01") + payload(data_len).
   예) 회원처리호가 응답 TTRODP11301(318B): msg_type="TCHTDP00000", tr_code="TTRODP11301",
       data_len=294(318-24). 체결 TTRTDP21301(233B): data_len=209. */
int  krx_build_data_push(char *buf, int bufsize,
                         const char *sender_id, const char *msg_type,
                         const char *tr_code, int seq_num, int meg_seq,
                         const char *data, int data_len);

/*------------------------------------------------------------------------
    API: Message Parser
------------------------------------------------------------------------*/
int  krx_parse_header(const char *buf, int len, KRX_PARSED_HEADER *hdr);
int  krx_get_body_length(const char *buf);

/*------------------------------------------------------------------------
    API: Header Builder (low-level)
------------------------------------------------------------------------*/
int  krx_build_header(char *buf, int bufsize,
                      const char *msg_type, int body_length,
                      const char *sender_id, int seq_num);

#endif /* KRX_PROTOCOL_H */
