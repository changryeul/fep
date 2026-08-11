#ifndef		__FEP_TCPIP_H
#define		__FEP_TCPIP_H
/*------------------------------------------------------------------------
#	Module	: structures and variables for TCP use
#	File	: fep_tcpip.h
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		STX						0x02

#define		TCP_MESSAGE_MAX_LEN		4300
#define		TCP_BUFF_MAX_LEN		5120

/* Meritz Tcp Header */
#define		TCP_HEAD_LEN			(sizeof (TCP_HEAD))			/* 80	*/
#define		TCP_DATA_HEAD_LEN		(sizeof (TCP_DATA_HEAD))	/* 70	*/
#define		TCP_REJE_HEAD_LEN		(sizeof (TCP_RJ_HEADER))	/* 80	*/
#define		AP_HEAD_LEN				(sizeof (AP_HEAD))			/* 60	*/

#define		HEAD_SIZE				(sizeof (FILE_DATA_HEAD))	/* 20 (File WR 용으로 사용)	*/
#define		TCP_DATA_LEN			(TCP_MESSAGE_MAX_LEN - TCP_HEAD_LEN)			/* 4220	*/

/* IMECO Tcp Header */
#define		IMECO_HEAD_LEN			(sizeof (IMECO_TCP_HEAD))	/* 20	*/
#define		IMECO_TCP_DATA_LEN		(TCP_MESSAGE_MAX_LEN - IMECO_HEAD_LEN)	/* 4280	*/


/* TCP 통신 header (80 bytes)	*/
typedef struct {
	char			Stx[1];						/* start of text (0x02)				*/
	char			Length[4];					/* Packet길이 5Byte 제외(80->75)	*/
	char			ApType[4];					/* 업무구분식별자 (process type)	*/
	char			SR_gbn[1];					/* 송수신 구분 S:송신, R:수신		*/
	char			Date[8];					/* 영업일자 (YYYYMMDD)				*/
	char			Time[6];					/* 시각 (HHMMSS)					*/
	char			ResponseCode[3];			/* 응답코드 '000'이외는 오류		*/
	char			Q_gbn[3];					/* '000' Queue순차번호				*/
	char			SeqNo[8];					/* 전문일련번호; TCP port (접속응답)*/
	char			DataCnt[2];					/* Block내Data건수					*/
												/*  01~08:DATA전문 (기본:01)
													91:운영전문,
													92:개시전문,
													93:일련번호요청전문,
													98:POLL전문						*/
	char			Last_SeqNo[8];				/* 최종일련번호, 전문일련번호와 동일*/
	char			Server_gbn[1];				/* 서버구분, '1'(정산주문)			*/
	char			Branch[3];					/* 계좌개설지점번호 (SPACE)			*/
	char			Sv_No[4];					/* 물리적지점번호 Server번호, FEP 부여(8706)	*/
	char			Hts_Id[8];					/* HTS단말 ID, 특정매체 사용		*/
	char			User_Id[8];					/* 사번 or 로그인 ID, 특정매체 사용	*/
	char			Filler[8];					/* '00000000' Default				*/
}	TCP_HEAD;

/* data header (70 bytes)	*/
typedef struct {
	char	Filler[70];							/* 예비							*/
}	TCP_DATA_HEAD;

/* data header (20 bytes)   */
typedef struct {
    char    Length[4];                  /* Data±æÀÌ (20+½ÇData±æÀÌ)     */
    char    DataSeq[8];                 /* DataÀÏ·Ã¹øÈ£                 */
    char    ResponseCode[4];            /* ÀÀ´äÄÚµå                     */
    char    LineFlag[3];                /* ´ë¿Ü¼Û½ÅLine±¸ºÐ (ÁÖ¹®Àü¹®)  */
    char    Filler[1];                  /* ¿¹ºñ                         */
}   FILE_DATA_HEAD;

/* 송신포맷, 80+70+294=>444 bytes	*/
/* - 채권일반_254, 채권LP_255, 금융파생_294, IMECO_112 => 가장큰 사이즈인 294로 처리함 */ 
/* async로 처리하며 복수건없이 단건씩 처리한다 */
typedef struct {
	TCP_HEAD		Head;								/* 통신 Header	*/
	TCP_DATA_HEAD	Data_Head;							/* Data Header	*/
/*	KRX_JUMUN_DATA	Data;								   주문포맷(294) */
	char			Data[TCP_DATA_LEN];					/* Data부		*/
}	TCP_MESSAGE;

/* FEP 거부 Header */
typedef struct {
	char	DataSeq[11];			/* 일련번호 미사용	*/
	char	Rj_trcd[11];			/* 거부 트렌젝션코드, "MERITZREJEC"	*/
	char	Megrp_no[2];			/* 매칭그룹, "00" -> 체결전문 형식 맞춤	*/
	char	Rj_tm[9];				/* 거부처리시각		*/
	char	Rj_gbn[3];				/* 거부구분			*/
	char	Rj_cd[4];				/* "0000" 이외는 오류	*/
	char	Rj_txt[40];				/* 거부사유txt		*/
}	TCP_RJ_HEADER;


/* 업무계 header (60 bytes)	*/
typedef struct {
	char	Stx[1];
	char	Len[4];
	char	ApType[4];
	char	SndRcvType[1];
	char	Date[8];
	char	Time[6];
	char	ResponseCode[3];
	char	QueueNo[3];
	char	SeqNo[8];
	char	DataCnt[2];
	char	Filler[20];
}	AP_HEAD;

/* IMECO FEP Header (20 bytes) */
typedef struct {
	char			Length[4];					/* Header를 제외한 Packet길이 (예, 20+80->80)	*/
	char			MsgType[1];					/* 업무구분식별자 (L:Logon, H:Heartbeat, D:주문,응답,체결)	*/
	char			ResponseCode[4];			/* 응답코드 '0000'이외는 오류		*/
	char			SeqNo[10];					/* 전문일련번호; TCP port (접속응답)*/

	char			MsgCount[1];				/* '0' 상수로 사용 					*/
}	IMECO_TCP_HEAD;

/* async로 처리하며 복수건없이 단건씩 처리한다 */
typedef struct {
    IMECO_TCP_HEAD	Head;						/* 통신 Header (20) */
    char            Data[IMECO_TCP_DATA_LEN];	/* Data부       	*/
}   IMECO_TCP_MESSAGE;


/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
#ifdef	_GLOBAL

TCP_HEAD   					Tcp_Head;
TCP_HEAD   					*pTcp_Head;
TCP_MESSAGE					Tcp_Message;
TCP_MESSAGE					*pTcp_Message;
TCP_DATA_HEAD				Tcp_Data_Head;
TCP_DATA_HEAD				*pTcp_Data_Head;

/* 2025 Imeco Add */
IMECO_TCP_HEAD   			Imeco_Tcp_Head;
IMECO_TCP_HEAD   			*pImeco_Tcp_Head;
IMECO_TCP_MESSAGE			Imeco_Tcp_Message;
IMECO_TCP_MESSAGE			*pImeco_Tcp_Message;

#else

extern TCP_HEAD   			Tcp_Head;
extern TCP_HEAD   			*pTcp_Head;
extern TCP_MESSAGE			Tcp_Message;
extern TCP_MESSAGE			*pTcp_Message;
extern TCP_DATA_HEAD		Tcp_Data_Head;
extern TCP_DATA_HEAD		*pTcp_Data_Head;

/* 2025 Imeco Add */
extern IMECO_TCP_HEAD   	Imeco_Tcp_Head;
extern IMECO_TCP_HEAD   	*pImeco_Tcp_Head;
extern IMECO_TCP_MESSAGE	Imeco_Tcp_Message;
extern IMECO_TCP_MESSAGE	*pImeco_Tcp_Message;
#endif

/* q add */
typedef struct {
    long mtype;
    unsigned char mtext[4096];
} Msgbuf;

/*************************************************************************
	End of Program (fep_tcpip.h)
*************************************************************************/
#endif
