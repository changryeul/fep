#ifndef		__FEP_INTERFACE_H
#define		__FEP_INTERFACE_H
/*------------------------------------------------------------------------
#	Module	: common constants and functions used in FEP interface
#	File	: fep_interface.h
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
/* macros for interface	*/
#define		MAIN			0
#define		BACKUP			1
#define		EMERGENCY		2
#define		CHANGE			3
#define		ALTERNATE_P		4
#define		ALTERNATE_B		5

#define		TR_LINK			0
#define		RP_LINK			1
#define		TR_RESD			2
#define		RP_RESD			3
#define		TR_POLL			4
#define		RP_POLL			5
#define		TR_DATA			6
#define		RP_DATA			7
#define		TR_STOP			8
#define		RP_STOP			9
#define		TR_DEND			10
#define		RP_DEND			11
#define		TR_RESE			12
#define		RP_RESE			13
#define		TR_ERCD			14
#define		RP_ERCD			15
#define     TR_LOON         16
#define     RP_LOON         17
#define     TR_LOOU         18
#define     RP_LOOU         19
#define		TR_RELI			21
#define		RP_RELI			22
#define		TR_CONT			23
#define		RP_CONT			24
#define		TR_HEAD			25
#define		RP_HEAD			26
#define		TR_TAIL			27
#define		RP_TAIL			28
#define     TR_SEQU         29
#define     RP_SEQU         30

#define		LIOK_LEN		20
#define		STOP_LEN		12
#define		RESE_LEN		18
#define		HEAD_LEN		80
#define		ERCD_LEN		92
#define		PROC_LEN		99

#define		LINK			1
#define		LIOK			2
#define		RELI			3
#define		REOK			4
#define		RESN			5
#define		LIVE			6
#define 	MMJC			7
#define 	LAST			8
#define 	LAOK			9

#define     FO01            11
#define     FO02            12
#define     FO03            13

#define 	AW54			21
#define 	AW55			22
#define 	CW01			23
#define 	CW02			24

#define 	FOCL			31
#define 	DATA			32
#define 	ERCD			33
#define 	REST			34
#define 	STOP			35
#define 	RESE			36
#define		REJE			99

#define		LK				1
#define		LC				2
#define		RE				3

#define		TR_AW54			"AW54"			/* 선물옵션 접수내역 재확인	*/
#define		TR_AW55			"AW55"			/* 선물옵션 체결내역 재확인	*/

#define		TCP_COMPANY		"00021"

/* TCP 업무접속 처리코드	*/
#define		TCP_LINK_CD		"LINK"						/* 접속요구		*/
#define		TCP_LIOK_CD		"LIOK"						/* 접속응답		*/
#define		TCP_STRT_CD		"STRT"						/* 개시요구		*/
#define		TCP_STOK_CD		"STOK"						/* 개시응답		*/
#define		TCP_DATA_CD		"DATA"						/* data처리요구	*/
#define		TCP_DAOK_CD		"DAOK"						/* data처리응답	*/
#define		TCP_RSND_CD		"RSND"						/* 재전송요구	*/
#define		TCP_RSOK_CD		"RSOK"						/* 재전송응답	*/
#define		TCP_POLL_CD		"POLL"						/* 회선시험요구	*/
#define		TCP_POOK_CD		"POOK"						/* 회선시험응답	*/
#define		TCP_STOP_CD		"STOP"						/* 종료요구		*/

/* TCP TR	*/
#define		T_LINK			1							/* 접속요구		*/
#define		T_LIOK			2							/* 접속응답		*/
#define		T_STRT			3							/* 개시요구		*/
#define		T_STOK			4							/* 개시응답		*/
#define		T_DATA			5							/* data처리요구	*/
#define		T_DAOK			6							/* data처리응답	*/
#define		T_RSND			7							/* 재전송요구	*/
#define		T_RSOK			8							/* 재전송응답	*/
#define		T_POLL			9							/* 회선시험요구	*/
#define		T_POOK			10							/* 회선시험응답	*/
#define		T_STOP			11							/* 종료요구		*/
#define		T_LNTH			12							/* 전문길이오류	*/
#define		T_EROR			13							/* 기타오류		*/

/* macros for sequence and status	*/
#define	START_S		START_STAT(D_K,P_K)		/* start status				*/
#define SESSION_S   SESSION_STAT(D_K,P_K)   /* SAFE ROUTINE status      */
#define	INT_SEQ		IF_SEQ(D_K,P_K)			/* interface sequence		*/
#define	TIME_OUT	TIME_VALUE(D_K,P_K)		/* timeout (sec)			*/
#define	DELAY_TIME	PROC(D_K,P_K).delay		/* PS delay time (millisec)	*/
#define	DATE_CURR  	PROC(D_K,P_K).date		/* business date			*/
#define	LOAD_CNT	DATA_CNT(D_K,P_K)		/* No of data loaded on SHM	*/
#define	DR_FLAG		PROC(D_K,P_K).dr_flag	/* KSE DR flag				*/

/* macros for daemon FIFO fds	*/
#define	START_FD	SFIFD(D_K)
#define	EXIT_FD		EFIFD(D_K)
#define	DTART_FD	DFIFD(D_K)

#define	TCP1_NET_STA		TCP1_NSTAT(D_K,P_K)		/* network status	*/

#define	TCP2_LINE_GU		TCP2_LINE_GUBUN(D_K,P_K)/* type of line		*/
#define	TCP2_CON_STA		TCP2_CSTAT(D_K,P_K)		/* connect status	*/
#define	TCP2_PORT_NO		TCP2_PORT(D_K,P_K,S_K)	/* port no			*/
#define	TCP2_LINE_ST		TCP2_LSTAT(D_K,P_K,S_K)	/* line status		*/
#define	TCP2_LINE_ST1(k)	TCP2_LSTAT(D_K,P_K,k)	/* line status		*/
#define	TCP2_PROC_ST		TCP2_PSTAT(D_K,P_K,S_K)	/* process status	*/
#define	TCP2_PROC_ST1(k)	TCP2_PSTAT(D_K,P_K,k)	/* process status	*/
#define	TCP2_NET_STA(k)		TCP2_NSTAT(D_K,P_K,k)	/* network status	*/

/* macros for input FIFO fd	*/
#define	INPUT_FD		IFIFD(D_K,P_K,0)	/* fd of in-file 1 FIFO		*/
#define	INPUT_FD2		IFIFD(D_K,P_K,1)	/* fd of in-file 2 FIFO		*/
#define	INPUT_FD3		IFIFD(D_K,P_K,2)	/* fd of in-file 3 FIFO		*/

#define W_CNT(i,j)      IFW(D_K,P_K,i,j)    /* write count of in-file   */
#define R_CNT(i,j)      IFR(D_K,P_K,i,j)    /* read count of in-file    */

#define WRITE_CNT       W_CNT(0,0)
#define READ_CNT        R_CNT(0,0)
#define WRITE_CNT2      W_CNT(1,0)
#define READ_CNT2       R_CNT(1,0)

#define OFW_CNT(i,j)    OFW(D_K,P_K,i,j)    /* write count of out-file(신) */
#define OFR_CNT(i,j)    OFR(D_K,P_K,i,j)    /* read count of out-file(신)  */
#define OW_CNT(i,j)     OFW_CNT(i,j)        /* write count of out-file(구) */
#define OR_CNT(i,j)     OFR_CNT(i,j)        /* read count of out-file(구)  */

#define	O_W_CNT1		OFW_CNT(0,0)
#define	O_W_CNT2		OFW_CNT(1,0)
#define	O_W_CNT3		OFW_CNT(2,0)
#define	O_W_CNT4		OFW_CNT(3,0)
#define	O_W_CNT5		OFW_CNT(4,0)
#define	O_W_CNT6		OFW_CNT(5,0)
#define	O_W_CNT7		OFW_CNT(6,0)
#define	O_W_CNT8		OFW_CNT(7,0)
#define	O_W_CNT9		OFW_CNT(8,0)

#define	O_R_CNT1		OFR_CNT(0,0)
#define	O_R_CNT2		OFR_CNT(1,0)
#define	O_R_CNT3		OFR_CNT(2,0)
#define	O_R_CNT4		OFR_CNT(3,0)
#define	O_R_CNT5		OFR_CNT(4,0)
#define	O_R_CNT6		OFR_CNT(5,0)
#define	O_R_CNT7		OFR_CNT(6,0)
#define	O_R_CNT8		OFR_CNT(7,0)
#define	O_R_CNT9		OFR_CNT(8,0)

/* macros for data SHM	*/
#define	IDW_CNT(i,j)	IDW(D_K,P_K,i,j)	/* write count of in-SHM	*/
#define	IDR_CNT(i,j)	IDR(D_K,P_K,i,j)	/* read count of in-SHM		*/

#define	ODW_CNT(i,j)	ODW(D_K,P_K,i,j)	/* write count of out-SHM	*/
#define	ODR_CNT(i,j)	ODR(D_K,P_K,i,j)	/* read count of out-SHM	*/

/*************************************************************************
	End of Program (fep_interface.h)
*************************************************************************/
#endif

