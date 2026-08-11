/******************************************************************************/
/*  Components  : intcom.h						      */
/*  Description	: Inter communication between FEP, Master, BP, AP	      */
/*  Rev. History: Ver	Date	Description				      */
/*		  ----	-------	----------------------------------------------*/
/*		  1.0	2006.08	Initial version				      */
/******************************************************************************/
#ifndef	_INTCOM_H
#define	_INTCOM_H


/* TCP/IP communication service port	*/
#if 0
#define	PN_UDP_FEP_BPS	6000		/* FEP -> BP 시세전달 PORT	*/
					/* 6000 ~ 6100 : reserved	*/
#define	PN_UDP_DAY_CLS	6005		/* FEP -> BP GL시세전달(CLOSE)	*/
#define	PN_UDP_CME_TRD	6010		/* FEP -> BP GL시세전달(CME)	*/
#define	PN_UDP_CME_DEP	6015		/* FEP -> BP GL시세전달(CME)	*/
#define	PN_UDP_CBT_TRD	6020		/* FEP -> BP GL시세전달(CBOT)	*/
#define	PN_UDP_CBT_DEP	6025		/* FEP -> BP GL시세전달(CBOT)	*/
#define	PN_UDP_CBM_TRD	6026		/* FEP -> BP GL시세전달(CBOT-ZG/ZI)*/
#define	PN_UDP_SGX_TRD	6030		/* FEP -> BP GL시세전달(SGX)	*/
#define	PN_UDP_SGX_DEP	6035		/* FEP -> BP GL시세전달(SGX)	*/
#define	PN_UDP_HKE_TRD	6040		/* FEP -> BP GL시세전달(HKFE)	*/
#define	PN_UDP_HKE_DEP	6045		/* FEP -> BP GL시세전달(HKFE)	*/
#define	PN_UDP_ERX_TRD	6050		/* FEP -> BP GL시세전달(EUREX)	*/
#define	PN_UDP_ERX_DEP	6055		/* FEP -> BP GL시세전달(EUREX)	*/

#define	PN_UDP_CME_PAT	6110		/* FEP -> BP PATS시세전달(CME)	*/
#define	PN_UDP_ETC_PAT	6120		/* FEP -> BP PATS시세전달(ETC)	*/
#define	PN_UDP_SGX_PAT	6130		/* FEP -> BP PATS시세전달(SGX)	*/

#define PN_UDP_KRX_APS  6200            /* 국내FEP -> APS 달러-원 시세전달	*/

#define	PN_UDP_NWS_BPS	6910		/* FEP -> BP NEWS전달(Edaily)	*/
#define PN_UDP_NWS_BPS_KYB  9999    /* FEP -> BP NEWS전달(REAL) : KYOBO로부터 수신         */
#define	PN_UDP_TFR_FRX	18010		/* FEP -> BP Tenfore 시세전달	*/
#define	PN_UDP_FX_BP	10491		/* FEP -> BP Tenfore 시세전달	*/

#define	PN_UDP_APS_BPS	9800		/* APS -> BP TRAN 전달 PORT	*/
#define	PN_UDP_RTS_OEM	9809		/* APS -> BP 주문체결 전달 PORT	*/
#define	PN_TCP_QUOT2AP	9811		/* HTS -> APS QUOTE 전달 PORT	*/
#define	PN_TCP_LIST2AP	9812		/* HTS -> APS LIST 전달 PORT	*/
#define	PN_TCP_MST_ALL	9901		/* MASTER <-> Server		*/
					/* 9901 ~ 9905 : reserved	*/
#define	PN_UDP_ALR2ALL	9910		/* MASTER --> Server		*/
#define	PN_UDP_NOTICE	9950		/* Server --> Server		*/

/* defined port no. */
#define	PN_TCP_SISE	9801		/* HTS -> 원장SVR (시세)	*/
#define	PN_TCP_LIST	9802		/* HTS -> 원장SVR (종목상장)	*/
#define PN_TCP_SGXM     9803            /* HTS -> 원장SVR (SGX옵션상장) */
#define PN_TCP_SGXQ     9804            /* HTS -> 원장SVR (SGX옵션시세) */
#define PN_UDP_JMUN	9910		/* 원장SVR -> HTS (주문/체결)	*/
#define PN_UDP_WARN	9915		/* 원장SVR -> HTS (장애통보)	*/

#define	PN_UDP_RT_EXCL	16001		/* REUTER EXCEL시세수신 PORT   	*/
#endif

#define	IP_UDP_TO_XX	"52.80.1.241"
#define	DEV_SVR_IP	"52.80.1.50"

struct	cchdr {				/* communication common header	*/
	char	hdr[4];			/* 0xfa, 0xfb, 0xfc, 0xfd	*/
	char	fcd[1];			/* function code		*/
	char	len[7];			/* frame data length		*/
					/* char dat[n] = data 		*/
};
#define	L_CCHDR		sizeof(struct cchdr)

/* cchdr.fcd : function code */
#define	FCD_ACK		'A'		/* ACK    : general		*/
#define	FCD_NAK		'N'		/* NAK    : general		*/
#define	FCD_TRAN	'T'		/* TRAN   : transaction		*/
#define	FCD_QUERY	'Q'		/* QUERY  : master->server	*/
#define	FCD_REPLY	'R'		/* REPLY  : server->master	*/
#define	FCD_APBP	'P'		/* AP2BP  : 원장SVR <-> 시세SVR	*/

struct	c_query {			/* master -> server header	*/
	char	dlen[6];		/* data length			*/
};

struct	c_reply {			/* server -> master header	*/
	char	whoi;			/* server role			*/
	char	ipad[15];		/* server ip-address		*/
	char	stat[4];		/* system status		*/
	char	alen[6];		/* status & accounting length	*/
	char	mstp[1];		/* message type	('M': message)	*/
	char	mlen[5];		/* message length		*/
};
#define	IAM_FEP		1
#define	IAM_BPS		2
#define	IAM_APS		3

#endif
