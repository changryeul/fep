/*------------------------------------------------------------------------
#   Module  : KRX Protocol Message Builder/Parser
#   File    : krx_protocol.c
#   Purpose : Builds and parses KRX TCP messages matching the real
#             pa_struct.h KRX_HEADER (82B) layout.
------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "krx_protocol.h"

/*------------------------------------------------------------------------
    Internal: fill a fixed-width field (space-padded, no null terminator)
------------------------------------------------------------------------*/
static void fill_field(char *dst, int width, const char *src)
{
    int slen, i;

    memset(dst, ' ', width);
    if (src == NULL) return;

    slen = (int)strlen(src);
    if (slen > width) slen = width;
    for (i = 0; i < slen; i++)
        dst[i] = src[i];
}

/*------------------------------------------------------------------------
    Internal: fill a numeric field (zero-padded, right-aligned)
------------------------------------------------------------------------*/
static void fill_num(char *dst, int width, int val)
{
    char tmp[32];
    sprintf(tmp, "%0*d", width, val);
    memcpy(dst, tmp, width);
}

/*------------------------------------------------------------------------
    Internal: get current timestamp YYYYMMDDHHMMSSMMM (17 chars)
------------------------------------------------------------------------*/
static void fill_timestamp(char *dst)
{
    time_t      now;
    struct tm   *tm_info;

    time(&now);
    tm_info = localtime(&now);

    sprintf(dst, "%04d%02d%02d%02d%02d%02d000",
            tm_info->tm_year + 1900,
            tm_info->tm_mon + 1,
            tm_info->tm_mday,
            tm_info->tm_hour,
            tm_info->tm_min,
            tm_info->tm_sec);
}

/* Sequence counter for messages */
static int g_seq_num = 1;

/*------------------------------------------------------------------------
    krx_build_header — low-level header builder
    Fills 82 bytes at buf[0..81].
    body_length: length of everything after the header (to go in BodyLength field).
    Returns 82 on success, -1 on error.
------------------------------------------------------------------------*/
int krx_build_header(char *buf, int bufsize,
                     const char *msg_type, int body_length,
                     const char *sender_id, int seq_num)
{
    if (buf == NULL || bufsize < KRX_HDR_LEN)
        return -1;

    memset(buf, ' ', KRX_HDR_LEN);

    /* BeginString[8] */
    fill_field(buf + KRX_OFF_BEGIN, 8, KRX_BEGIN_STRING);

    /* BodyLength[6] — zero-padded */
    fill_num(buf + KRX_OFF_BODYLEN, 6, body_length);

    /* MsgType[11] */
    fill_field(buf + KRX_OFF_MSGTYPE, 11, msg_type);

    /* MsgSeqNum[11] — zero-padded */
    fill_num(buf + KRX_OFF_MSGSEQNUM, 11, seq_num > 0 ? seq_num : g_seq_num++);

    /* SenderCompID[5] */
    fill_field(buf + KRX_OFF_SENDER, 5, sender_id ? sender_id : KRX_DEFAULT_SENDER);

    /* DeliverToCompID[10] — blank */
    /* OnBehalfOfCompID[10] — blank */

    /* SendingTime[17] */
    fill_timestamp(buf + KRX_OFF_SENDTIME);

    /* DataCnt[3] — "001" default */
    fill_num(buf + KRX_OFF_DATACNT, 3, 1);

    /* Encrypt[1] — "N" */
    buf[KRX_OFF_ENCRYPT] = 'N';

    return KRX_HDR_LEN;
}

/*------------------------------------------------------------------------
    krx_build_logon_req — SCHLIQ00000
    Session message: header(82) + session_data(41) = 123 bytes
------------------------------------------------------------------------*/
int krx_build_logon_req(char *buf, int bufsize,
                        const char *sender_id, const char *proc_info)
{
    int body_len = KRX_SESSION_DATA_LEN;    /* 41 */
    int total_len = KRX_HDR_LEN + body_len;

    if (buf == NULL || bufsize < total_len)
        return -1;

    memset(buf, ' ', total_len);

    krx_build_header(buf, bufsize, KRX_LOGON_REQ, body_len, sender_id, 0);

    /* Session data: proc_info[10] + proc_uid[30] + encrypt_flag[1] */
    if (proc_info)
        fill_field(buf + KRX_HDR_LEN, 10, proc_info);
    else
        fill_field(buf + KRX_HDR_LEN, 10, "INTEG_TEST");

    /* proc_uid[30] — blank */
    /* encrypt_flag[1] */
    buf[KRX_HDR_LEN + 40] = 'N';

    return total_len;
}

/*------------------------------------------------------------------------
    krx_build_logon_resp — SCHLIR00000
------------------------------------------------------------------------*/
int krx_build_logon_resp(char *buf, int bufsize,
                         const char *sender_id, int success)
{
    int body_len = KRX_SESSION_DATA_LEN;
    int total_len = KRX_HDR_LEN + body_len;

    if (buf == NULL || bufsize < total_len)
        return -1;

    memset(buf, ' ', total_len);

    krx_build_header(buf, bufsize, KRX_LOGON_RESP, body_len, sender_id, 0);

    /* Response: proc_info[10] with result code */
    fill_field(buf + KRX_HDR_LEN, 10, success ? "0000000000" : "9999999999");
    buf[KRX_HDR_LEN + 40] = 'N';

    return total_len;
}

/*------------------------------------------------------------------------
    krx_build_link_req — SCHOPQ00000 (Service Start Request)
------------------------------------------------------------------------*/
int krx_build_link_req(char *buf, int bufsize, const char *sender_id)
{
    int total_len = KRX_HDR_LEN;    /* header only */

    if (buf == NULL || bufsize < total_len)
        return -1;

    memset(buf, ' ', total_len);
    krx_build_header(buf, bufsize, KRX_LINK_REQ, 0, sender_id, 0);

    return total_len;
}

/*------------------------------------------------------------------------
    krx_build_link_resp — SCHOPR00000 (Service Start Response)
------------------------------------------------------------------------*/
int krx_build_link_resp(char *buf, int bufsize, const char *sender_id)
{
    int total_len = KRX_HDR_LEN;

    if (buf == NULL || bufsize < total_len)
        return -1;

    memset(buf, ' ', total_len);
    krx_build_header(buf, bufsize, KRX_LINK_RESP, 0, sender_id, 0);

    return total_len;
}

/*------------------------------------------------------------------------
    krx_build_poll_req — SCHHEQ00000 (Heartbeat Request)
------------------------------------------------------------------------*/
int krx_build_poll_req(char *buf, int bufsize, const char *sender_id)
{
    int total_len = KRX_HDR_LEN;

    if (buf == NULL || bufsize < total_len)
        return -1;

    memset(buf, ' ', total_len);
    krx_build_header(buf, bufsize, KRX_POLL_REQ, 0, sender_id, 0);

    return total_len;
}

/*------------------------------------------------------------------------
    krx_build_poll_resp — SCHHER00000 (Heartbeat Response)
------------------------------------------------------------------------*/
int krx_build_poll_resp(char *buf, int bufsize, const char *sender_id)
{
    int total_len = KRX_HDR_LEN;

    if (buf == NULL || bufsize < total_len)
        return -1;

    memset(buf, ' ', total_len);
    krx_build_header(buf, bufsize, KRX_POLL_RESP, 0, sender_id, 0);

    return total_len;
}

/*------------------------------------------------------------------------
    krx_build_logoff_req — SCHLOQ00000
------------------------------------------------------------------------*/
int krx_build_logoff_req(char *buf, int bufsize, const char *sender_id)
{
    int total_len = KRX_HDR_LEN;

    if (buf == NULL || bufsize < total_len)
        return -1;

    memset(buf, ' ', total_len);
    krx_build_header(buf, bufsize, KRX_LOGOFF_REQ, 0, sender_id, 0);

    return total_len;
}

/*------------------------------------------------------------------------
    krx_build_order_msg — Data message with KRX_BODY_COMMON + payload
    header(82) + body_common(24) + data
------------------------------------------------------------------------*/
int krx_build_order_msg(char *buf, int bufsize,
                        const char *sender_id,
                        const char *tr_code,
                        const char *data, int data_len)
{
    int body_len = KRX_BODY_COMMON_LEN + data_len;
    int total_len = KRX_HDR_LEN + body_len;
    char msg_type[12];

    if (buf == NULL || bufsize < total_len)
        return -1;

    memset(buf, ' ', total_len);

    /* MsgType for data: use "TCHODR00000" pattern */
    memset(msg_type, 0, sizeof(msg_type));
    strncpy(msg_type, "TCHODR00000", 11);

    krx_build_header(buf, bufsize, msg_type, body_len, sender_id, 0);

    /* KRX_BODY_COMMON: DataSeq[11] + Transaction_Code[11] + Megrp_no[2] */
    fill_num(buf + KRX_OFF_DATASEQ, 11, g_seq_num - 1);   /* same seq as header */
    fill_field(buf + KRX_OFF_TRCODE, 11, tr_code);
    fill_field(buf + KRX_OFF_MEGRP, 2, "01");

    /* Payload after body_common */
    if (data && data_len > 0)
        memcpy(buf + KRX_MSG_COMMON_LEN, data, data_len);

    return total_len;
}

int krx_build_data_push(char *buf, int bufsize,
                        const char *sender_id, const char *msg_type,
                        const char *tr_code, int seq_num, int meg_seq,
                        const char *data, int data_len)
{
    int body_len  = KRX_BODY_COMMON_LEN + data_len;
    int total_len = KRX_HDR_LEN + body_len;

    if (buf == NULL || bufsize < total_len)
        return -1;

    memset(buf, ' ', total_len);

    /* Header: MsgType/BodyLength/MsgSeqNum 직접 지정 */
    krx_build_header(buf, bufsize, msg_type, body_len, sender_id, seq_num);

    /* BODY_COMMON: DataSeq=ME그룹seq / TrCode / Megrp="01" */
    fill_num(buf + KRX_OFF_DATASEQ, 11, meg_seq);
    fill_field(buf + KRX_OFF_TRCODE, 11, tr_code);
    fill_field(buf + KRX_OFF_MEGRP, 2, "01");

    if (data && data_len > 0)
        memcpy(buf + KRX_MSG_COMMON_LEN, data, data_len);

    return total_len;
}

/*------------------------------------------------------------------------
    krx_parse_header — parse raw 82-byte header into KRX_PARSED_HEADER
    Returns 0 on success, -1 on error.
------------------------------------------------------------------------*/
int krx_parse_header(const char *buf, int len, KRX_PARSED_HEADER *hdr)
{
    if (buf == NULL || hdr == NULL || len < KRX_HDR_LEN)
        return -1;

    memset(hdr, 0, sizeof(KRX_PARSED_HEADER));

    memcpy(hdr->BeginString,       buf + KRX_OFF_BEGIN,    8);  hdr->BeginString[8] = '\0';
    memcpy(hdr->BodyLength,        buf + KRX_OFF_BODYLEN,  6);  hdr->BodyLength[6] = '\0';
    memcpy(hdr->MsgType,           buf + KRX_OFF_MSGTYPE, 11);  hdr->MsgType[11] = '\0';
    memcpy(hdr->MsgSeqNum,         buf + KRX_OFF_MSGSEQNUM, 11); hdr->MsgSeqNum[11] = '\0';
    memcpy(hdr->SenderCompID,      buf + KRX_OFF_SENDER,   5);  hdr->SenderCompID[5] = '\0';
    memcpy(hdr->DeliverToCompID,   buf + KRX_OFF_DELIVER, 10);  hdr->DeliverToCompID[10] = '\0';
    memcpy(hdr->OnBehalfOfCompID,  buf + KRX_OFF_ONBEHALF, 10); hdr->OnBehalfOfCompID[10] = '\0';
    memcpy(hdr->SendingTime,       buf + KRX_OFF_SENDTIME, 17); hdr->SendingTime[17] = '\0';
    memcpy(hdr->DataCnt,           buf + KRX_OFF_DATACNT,  3);  hdr->DataCnt[3] = '\0';
    memcpy(hdr->Encrypt,           buf + KRX_OFF_ENCRYPT,  1);  hdr->Encrypt[1] = '\0';

    /* Parse BodyLength to integer */
    hdr->body_length = krx_get_body_length(buf);

    return 0;
}

/*------------------------------------------------------------------------
    krx_get_body_length — extract BodyLength field as integer
------------------------------------------------------------------------*/
int krx_get_body_length(const char *buf)
{
    int i, val = 0;

    if (buf == NULL) return -1;

    for (i = 0; i < 6; i++) {
        char c = buf[KRX_OFF_BODYLEN + i];
        if (c >= '0' && c <= '9')
            val = val * 10 + (c - '0');
    }
    return val;
}

/*------------------------------------------------------------------------
    End of krx_protocol.c
------------------------------------------------------------------------*/
