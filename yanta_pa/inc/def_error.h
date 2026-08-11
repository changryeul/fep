#ifndef		__DEF_ERROR_H
#define		__DEF_ERROR_H
/*------------------------------------------------------------------------
#	Module	: define error codes
#	File	: def_error.h
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		USR_OK			0									/* user	*/
#define		USR_FATAL		1
#define		USR_DEBUG		2
#define		USR_WARN		3
#define		USR_ERROR		5

#define		SYS_OK			100								/* system	*/
#define		SYS_FATAL		101
#define		SYS_DEBUG		102
#define		SYS_WARN		103
#define		SYS_ERROR		105

#define		PRO_OK			200								/* process	*/
#define		PRO_FATAL		201
#define		PRO_DEBUG		202
#define		PRO_WARN		203
#define		PRO_ERROR		205

#define		SAM_OK			300									/* file	*/
#define		SAM_FATAL		301
#define		SAM_DEBUG		302
#define		SAM_WARN		303
#define		SAM_ERROR		305

#define		FIF_OK			400									/* FIFO	*/
#define		FIF_FATAL		401
#define		FIF_DEBUG		402
#define		FIF_WARN		403
#define		FIF_ERROR		405

#define		ORA_OK			500									/* DB	*/
#define		ORA_FATAL		501
#define		ORA_DEBUG		502
#define		ORA_WARN		503
#define		ORA_ERROR		505

#define		TCP_OK			600								/* TCP/IP	*/
#define		TCP_FATAL		601
#define		TCP_DEBUG		602
#define		TCP_WARN		603
#define		TCP_ERROR		605

#define		UDP_OK			700								/* UDP/IP	*/
#define		UDP_FATAL		701
#define		UDP_DEBUG		702
#define		UDP_WARN		703
#define		UDP_ERROR		705

#define		DSH_OK			800								/* data SHM	*/
#define		DSH_FATAL		801
#define		DSH_DEBUG		802
#define		DSH_WARN		803
#define		DSH_ERROR		805

/* system error	*/
#define		SYS_NO		errno						/* error number		*/
#define		SYS_STR		strerror(errno)				/* error message	*/

#if defined ISAM_INCL
#define		ISYS_NO		iserrno
#define		ISYS_STR	((iserrno >= 100 && iserrno < is_nerr) ?\
	is_errlist[iserrno-100] : is_errlist[iserrno])
#endif

/* error codes - TCP header	*/
#define		RES_NORMAL			"0000"
#define		RES_ERROR			"9999"			/* HTS거부. 정상처리함	*/
#define		ERR_HEAD_T1			"T001"					/* Stx			*/
#define		ERR_HEAD_T2			"T002"					/* Length		*/
#define		ERR_HEAD_T3			"T003"					/* ApType		*/
#define		ERR_HEAD_T4			"T004"					/* Date			*/
#define		ERR_HEAD_T5			"T005"					/* Time			*/
#define		ERR_HEAD_T6			"T006"					/* ResponseCode	*/
#define		ERR_HEAD_T7			"T007"					/* MsgType		*/
#define		ERR_HEAD_T8			"T008"					/* SeqNo		*/
#define		ERR_HEAD_T9			"T009"					/* DataCnt		*/
#define		ERR_HEAD_B1			"T101"					/* BatchTr		*/
#define		ERR_HEAD_B2			"T102"					/* BatchSeq		*/

/* error codes - data header	*/
#define		ERR_HEAD_D1			"D001"					/* Length		*/
#define		ERR_HEAD_D2			"D002"					/* DataSeq		*/
#define		ERR_HEAD_D3			"D003"					/* LineFlag		*/

#define		ERR_TIME_BEFORE		"F100"		/* 업무 시작전 상태			*/
#define		ERR_PROC_RUN		"F101"		/* 기동중인 process 접속불가*/
#define		ERR_TIME_END		"F102"		/* 업무 종료 상태			*/
#define		ERR_TIME_STOP		"F103"		/* 업무 중지 상태			*/
#define		ERR_REJECT			"F104"		/* 허용되지 않은 client 접속*/

#define		ERR_JANG_BEFORE		"K100"		/* 장시작전 대외계 거부		*/
#define		ERR_JANG_ERROR		"K101"		/* 대외기관송신process장애	*/
#define		ERR_JANG_END		"K102"		/* 장종료후 대외계 거부		*/
#define		ERR_JANG_STOP		"K103"		/* 운영자에 의한 stop 상태	*/
#define		ERR_TIME_OUT		"K104"		/* timeout					*/
#define		ERR_SEND_ERR		"K105"		/* 대외기관 전송 오류		*/
#define		ERR_TCP_STATUS		"K106"		/* 업무요구송신 process 오류*/

#define		ERR_FILE_WRITE		"E901"		/* file write 오류			*/
#define		ERR_BATCH_DUP		"E902"		/* 대외송신완료batchTR재수신*/

/* error codes - data */
#define     ERR_DATA_H1         "D500"                 /* TRCODE Error  */
#define     ERR_DATA_S1         "D501"              /* 종목코드 미존재  */
#define     ERR_DATA_S2         "D502"              /* 계좌번호 불일치  */
#define     ERR_DATA_S3         "D503"              /* 비밀번호 불일치  */
#define     ERR_DATA_S4         "D504"              /* IP ADDR  불일치  */
#define     ERR_DATA_S5         "D505"     	/* 자동주문 기동 한도에러 	*/
#define     ERR_DATA_S6         "D506"     	/* 자동주문 기동 입력값에러 */
#define     ERR_DATA_S10        "D510"              /* 변경요청 미처리	*/
#define     ERR_DATA_S11        "D511"              /* Auto Process Full */
#define     ERR_DATA_S12        "D512"		/* Auto Process Run Time Err */
#define     ERR_DATA_S15        "D515"		/* Auto 사용 권한 Over Err	*/
#define     ERR_DATA_S16        "D516"		/* Auto 전략 사용 권한 Over Err	*/
#define     ERR_DATA_S20        "D520"              /* 송신회선이상발견 */


#if defined ISAM_INCL
#define		ERR_DD_EDUPL		"K121"		/* cisam - duplication		*/
#define		ERR_DD_ENOREC		"K122"		/* cisam - no record		*/
#endif
#define		ERR_DD_DIVIDE		"K201"		/* 분배 오류				*/

#define		KRX_DATA_LOSS		"K999"		/* DR전환시 KSE/KSQ 재전송	*/

/*************************************************************************
	End of Program (def_error.h)
*************************************************************************/
#endif
