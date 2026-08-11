#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: Online 업무 수신
#	File	: pw_3000_tr.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
#define		TCP_TIME_OUT	15

#ifdef MEM_USE
#define		WR_CNT			ODW_CNT(0,0)
#define		OUT_NAME		ODN(D_K,P_K,0)
#else
#define		WR_CNT			OFW_CNT(0,0)
#define		OUT_NAME		OFN(D_K,P_K,0)
#endif

/*------------------------------------------------------------------------
	Global Variables
------------------------------------------------------------------------*/
int		Sockfd, PktType, DataSize, DataSize2, MaxCnt, S_K, W_flag;
char	ApType[10], RecvPkt[TCP_BUFF_MAX_LEN], SendPkt[TCP_BUFF_MAX_LEN];

TCP_MESSAGE		*R_Pkt = (TCP_MESSAGE *)RecvPkt;
TCP_HEAD		*S_Pkt = (TCP_HEAD *)SendPkt;

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	PW_3000_TR (int, char **);
void	Init_Parameters (void);
int		Receive_Packet (void);
int		Send_Packet (void);
int		Write_Data (void);
int		Write_Data_Elw (void);
int		Check_Header (void);
void	Register_Signal (void);
void	Catch_Signal (int);
void	Set_Socket_Linger (void);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	Init_Proc (argc, argv);
	PW_3000_TR (argc, argv);

	TCP1_NET_STA = OFF;
	close (Sockfd);
	Exit_Process ();
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	PW_3000_TR (int argc, char **argv)
/*----------------------------------------------------------------------*/
{
	int		rt;

	Register_Signal ();
	Init_Parameters ();

	if (argc != 3)
	{
		Log (USR_FATAL, "invalid arguments number[%d]", argc);
		Exit_Process ();
	}

	Sockfd = AtoIf (argv[1], strlen (argv[1]));
	PROC(D_K,P_K).l.t1.line_gubun = AtoIf (argv[2], 1);
	PROC(D_K,P_K).l.t1.port_type = AtoIf (&argv[2][2], 1);
	Log (USR_OK, "Sockfd[%d] line_gubun[%d] port_type[%d]",
		Sockfd, PROC(D_K,P_K).l.t1.line_gubun, PROC(D_K,P_K).l.t1.port_type);

	Set_Socket_Linger ();

	PktType = T_STOK;

	rt = Send_Packet ();

	if (rt == NOTOK)
		return;

	while (START_S < JOB_END)
	{
		Stat_Save ();

		rt = Receive_Packet ();

		if (rt == NOTOK)
			break;
		else if (rt == FAIL)
			continue;

		rt = Send_Packet ();

		if (rt == NOTOK)
			break;
	}

	return;
}	/* End of PW_3000_TR ()	*/

/*************************************************************************
	Function		: . Init_Parameters
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . initiate the global variables
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Init_Parameters (void)
/*----------------------------------------------------------------------*/
{
	SYS_NO = 0;

	if (TIME_OUT == 0)
		TIME_OUT = TCP_TIME_OUT;

	sprintf (ApType, "%-2.2s%-4.4s%-2.2s", _Exe_Name, _Exe_Name+3, _Exe_Name+8);
	LtoU (ApType, strlen (ApType));

#if defined A3161||A3162||A3163||A3164||A3165||A3166||A3167||A3168||A3169||A3170
	S_K = AtoIf (&ApType[4], 2) - ELW_ACC_NO_BASE;
    DataSize2 = OFS(D_K,P_K,0);
#endif

#ifdef MEM_USE
    DataSize = ODS(D_K,P_K,0);
#else
    DataSize = OFS(D_K,P_K,0);
#endif
	MaxCnt = FILE_BUF_LEN / (sizeof (FILE_RW_HEAD) + DataSize + 1);

	return;
}  /* End of Init_Parameters () */

/*************************************************************************
	Function		: . Receive_Packet
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure, 1:interrupted)
	Comment			: . receive packet
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Receive_Packet (void)
/*----------------------------------------------------------------------*/
{
	int		rt, recv_len, seq;

	memset (RecvPkt, 0, sizeof (RecvPkt));

	rt = Select_Receive (Sockfd, RecvPkt);

	if (rt == OK)
		return (NOTOK);
	else if (rt == NOTOK)
	{
		if (SYS_NO == EINTR)
			return (FAIL);

		return (NOTOK);
	}

	recv_len = strlen (RecvPkt);

	if (recv_len < TCP_HEAD_LEN || recv_len != rt ||
		recv_len > TCP_MESSAGE_MAX_LEN)
	{
		Log (USR_ERROR, "invalid length[%d,%d]", recv_len, rt);
		PktType = T_LNTH;
		Send_Packet ();
		return (NOTOK);
	}

	if (memcmp (R_Pkt->Head.MsgType, TCP_STOP_CD, strlen (TCP_STOP_CD)) == 0)
	{
		Log (USR_OK, "received STOP request");
		TCP1_NET_STA = END;
		close (Sockfd);
		Exit_Process ();
	}
	else if (memcmp (R_Pkt->Head.MsgType,
		TCP_POLL_CD, strlen (TCP_POLL_CD)) == 0)
	{
		seq = AtoIf (R_Pkt->Head.SeqNo, sizeof (R_Pkt->Head.SeqNo));

		if (seq != INT_SEQ)
			Log (USR_WARN, "POLL:SeqNo [%d:%d]", seq, INT_SEQ);

		PktType = T_POOK;
	}
	else if (memcmp (R_Pkt->Head.MsgType,
		TCP_DATA_CD, strlen (TCP_DATA_CD)) == 0)
		PktType = T_DAOK;
	else
	{
		Log (USR_ERROR, "TCP header (MsgType) [%.4s:%s]",
			R_Pkt->Head.MsgType, TCP_DATA_CD);
		memcpy (R_Pkt->Head.ResponseCode, ERR_HEAD_T7, strlen (ERR_HEAD_T7));
		PktType = T_EROR;
		Send_Packet ();
		return (NOTOK);
	}

	return (OK);
}	/* Receive_Packet ()	*/

/*************************************************************************
	Function		: . Send_Packet
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure)
	Comment			: . send packet
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Send_Packet (void)
/*----------------------------------------------------------------------*/
{
	int		rt, seq, m_jan, j_jan, p_jan;
	char	error_flag, t_time[12];

	error_flag = OFF;
	memset (SendPkt, 0, sizeof (SendPkt));

	if (PktType == T_STOK)
		memset (SendPkt, ' ', TCP_HEAD_LEN);
	else
		memcpy (SendPkt, RecvPkt, TCP_HEAD_LEN);

	if (PktType != T_EROR)
	{
		S_Pkt->Stx[0] = STX;
		ItoAf (TCP_HEAD_LEN, S_Pkt->Length, sizeof (S_Pkt->Length));
		memcpy (S_Pkt->ApType, ApType, sizeof (S_Pkt->ApType));
		memcpy (S_Pkt->Date, DATE_CURR, sizeof (S_Pkt->Date));
		memset (t_time, 0, sizeof (t_time));
		Get_Time (t_time);
		memcpy (S_Pkt->Time, t_time, sizeof (S_Pkt->Time));
		ItoAf (0, S_Pkt->DataCnt, sizeof (S_Pkt->DataCnt));
	}

	switch (PktType)
	{
		case	T_STOK:
			ItoAf (0, S_Pkt->ResponseCode, sizeof (S_Pkt->ResponseCode));
			memcpy (S_Pkt->MsgType, TCP_STOK_CD, sizeof (S_Pkt->MsgType));
			break;
		case	T_POOK:
			memcpy (S_Pkt->MsgType, TCP_POOK_CD, sizeof (S_Pkt->MsgType));
			break;
		case    T_LNTH:
			memcpy (S_Pkt->ResponseCode, ERR_HEAD_T2, strlen (ERR_HEAD_T2));
			memcpy (S_Pkt->MsgType+2, "OK", 2);
			break;
		case	T_DAOK:
			rt = Check_Header ();

		/* **************************************************** */
		/* 주문번호 : &R_Pkt->Data[HEAD_SIZE+115]				*/
		/* 주문계약수량 : &R_Pkt->Data[HEAD_SIZE+115+46]		*/
		/* 회원사처리항목 : &R_Pkt->Data[HEAD_SIZE+115+46+108]	*/
		/* 매도매수구분 : &R_Pkt->Data[HEAD_SIZE+115+32]		*/
		/* 원주문번호 : &R_Pkt->Data[HEAD_SIZE+115+10]			*/
		/* **************************************************** */
#if defined A1150
/* MC 처리 횟수 (870001+1000)번, 계약수 100(선옵구분없이) */
			if (rt == OK)
			{
				/* 하드코딩 주문번호가 1000 넘었냐, */
				/* 50번 계좌의 Client주문버호를 비교 */

				/* 주문 1000개 이상 내면 안된다(누적) */
				if (ACCNO(D_K, AtoIf(&R_Pkt->Data[HEAD_SIZE+115+46+108+3], 2) - ACC_NO_BASE).acc_order_no >
					ACCNO(D_K, AtoIf(&R_Pkt->Data[HEAD_SIZE+115+46+108+3], 2) - ACC_NO_BASE).js_order_no_band_b + 300)
				{
					Log (USR_ERROR, "MC 주문 횟수 제한 오류 (Jumun_Cnt) [%d:%d]",
						ACCNO(D_K, AtoIf(&R_Pkt->Data[HEAD_SIZE+115+46+108+3], 2) - ACC_NO_BASE).acc_order_no,
						ACCNO(D_K, AtoIf(&R_Pkt->Data[HEAD_SIZE+115+46+108+3], 2) - ACC_NO_BASE).js_order_no_band_b);
					memcpy (S_Pkt->ResponseCode, ERR_DATA_S5, strlen (ERR_DATA_S5));
					rt = NOTOK;
				}

				/* 정정,취소의 원주문번호 Check */
				if ((memcmp (&R_Pkt->Data[HEAD_SIZE+93], "TCHODR10001", 11) != 0)	&&
					((AtoIf (&R_Pkt->Data[HEAD_SIZE+115+10], 6) >
					 ACCNO(D_K, AtoIf(&R_Pkt->Data[HEAD_SIZE+115+46+108+3], 2) - ACC_NO_BASE).acc_order_no)
					||
					(AtoIf (&R_Pkt->Data[HEAD_SIZE+115+10], 6) <
					 ACCNO(D_K, AtoIf(&R_Pkt->Data[HEAD_SIZE+115+46+108+3], 2) - ACC_NO_BASE).js_order_no_band_b))	)
				{
					Log (USR_ERROR, "정정,취소 주문번호 오류 [%d] ~ [%d] ~ [%d] s_k[%d]",
						ACCNO(D_K, AtoIf(&R_Pkt->Data[HEAD_SIZE+115+46+108+3], 2) - ACC_NO_BASE).acc_order_no,
						AtoIf (&R_Pkt->Data[HEAD_SIZE+115+10], 6),
						ACCNO(D_K, AtoIf(&R_Pkt->Data[HEAD_SIZE+115+46+108+3], 2) - ACC_NO_BASE).js_order_no_band_b,
						AtoIf(&R_Pkt->Data[HEAD_SIZE+115+46+108+3], 2) - ACC_NO_BASE);
					memcpy (S_Pkt->ResponseCode, ERR_DATA_S5, strlen (ERR_DATA_S5));
					rt = NOTOK;
				}

				/* 한번에 100계약 이상 주문시 에러 */
				if (AtoIf (&R_Pkt->Data[HEAD_SIZE+115+46], 10) > 100)
				{
					Log (USR_ERROR, "1회 주문수량 100회이상 발생 [%10.10s]",
						&R_Pkt->Data[HEAD_SIZE+115+46]);
					memcpy (S_Pkt->ResponseCode, ERR_DATA_S5, strlen (ERR_DATA_S5));
					rt = NOTOK;
				}
			}

			if (rt == OK)
			{
				rt = Write_Data ();

				if (rt != OK)
					error_flag = ON;
			}
			else
				error_flag = ON;

#elif defined A3161||A3162||A3163||A3164||A3165||A3166||A3167||A3168||A3169||A3170
/* **************************************************************************** */
/* 주문수신시 사전 Check 항목.													*/
/* 1. 종목 seq == 종목코드 (Client, 통신 Error)									*/
/* 2. 주문번호 채번 Check, ACCNO(D_K,s_k+ACC_NO_CNT).acc_order_no증가.			*/
/*    (Client, 통신 Error)														*/
/* 3. 다음 채널의 주문번호대 넘는것 Check.(Client/Server, 통신 Error)			*/
/* 4. 매도일때, 잔고 >= "주문수량" + Active_Order (Client, Error Write)			*/
/* **************************************************************************** */
/* ELW는 선매도를 취할수 없다. 현재 주문수량과 보유수량을 따져서 0과 같거나 커야한다 */
			W_flag = 1;

			if (rt == OK)
			{
				/* 1번 Check */
				/* 종목 일련번호의 종목코드 정합성 체크(회원사처리항목 17~20, 4바이트) */
				seq =  AtoIf (&R_Pkt->Data[HEAD_SIZE+115+46+124], 4);
				if ((memcmp (Shm_Elw[seq].Elw_Master.stock_code,
							&R_Pkt->Data[HEAD_SIZE+115+20], 12) != 0)	||
				/* 2번 Check */
					(ACCNO(D_K, S_K + ACC_NO_CNT).acc_order_no >=
					 AtoIf (&R_Pkt->Data[HEAD_SIZE+115], 6))	||	
				/* 3번 Check */
					(ACCNO(D_K, S_K + ACC_NO_CNT + 1).js_order_no_band_b <= 
					 AtoIf (&R_Pkt->Data[HEAD_SIZE+115], 6))	)
				{
					rt = NOTOK;
				}
			}

			if (rt == OK)
			{
				/* 2번 Check에 의한 주문번호 증가 */
				ACCNO(D_K, S_K + ACC_NO_CNT).acc_order_no =
					AtoIf (&R_Pkt->Data[HEAD_SIZE+115], 6);

				/* 4번 Check */
				/* 잔고확인(신규 && 매도일때만) */
				/* 실질잔고 >= 주문나가있는 매도잔고 + 들어온 매도수량 */
				if ((memcmp (&R_Pkt->Data[HEAD_SIZE+115+32], "1", 1) == 0)	&&
					(memcmp (&R_Pkt->Data[HEAD_SIZE+93], "TCHODR10001", 11) == 0))
				{
					p_jan = AtoIf (&R_Pkt->Data[HEAD_SIZE+115+46], 10);
					if (Shm_Elw[seq].Elw_Curr.getcnt[S_K] >=
						Shm_Elw[seq].Elw_Curr.jucnt[S_K] + p_jan)
					{
						W_flag = 1;
						/* ELW는 주문 나갈때 주문 나간 수량 증가 시킨다 */
						Shm_Elw[seq].Elw_Curr.jucnt[S_K] += p_jan;
					}
					else
					{
						W_flag = 2;
						Log (USR_ERROR, "미약정 매도주문 시도 주문번호 [%10.10s] [%12.12s] 매도수량[%d] Active_Order[%d] 보유수량[%d]",
							&R_Pkt->Data[HEAD_SIZE+82+33],
							&R_Pkt->Data[HEAD_SIZE+82+33+20],
							p_jan, Shm_Elw[seq].Elw_Curr.jucnt[S_K],
							Shm_Elw[seq].Elw_Curr.getcnt[S_K]);
					}
				}
			}

			if (rt == OK)
			{
				rt = Write_Data_Elw ();

				if (rt != OK)
				{
					error_flag = ON;
				/* ELW는 주문 나갈때 주문 나간 수량 다시 감소시킨다 */
					if (W_flag == 1)
						Shm_Elw[seq].Elw_Curr.jucnt[S_K] -= p_jan;
				}
			}
			else
				error_flag = ON;

#else
			if (rt == OK)
			{
				rt = Write_Data ();

				if (rt != OK)
					error_flag = ON;
			}
			else
				error_flag = ON;
#endif

			memcpy (S_Pkt->MsgType, TCP_DAOK_CD, sizeof (S_Pkt->MsgType));
			break;
		default:
			break;
	}

	if (PktType != T_EROR)
		ItoAf (INT_SEQ, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));

	rt = Select_Send (Sockfd, SendPkt, TCP_HEAD_LEN);

	if (rt != OK)
		return (NOTOK);

	if (PktType == T_STOK)
		TCP1_NET_STA = ON;
	else
		Log (TCP_OK, "TCP RD [%s](%d)<%d>", RecvPkt, strlen (RecvPkt), INT_SEQ);

	Log (TCP_OK, "TCP SD [%s](%d)<%d>", SendPkt, strlen (SendPkt), INT_SEQ);

	if (PktType == T_LNTH || error_flag == ON)
		return (NOTOK);

	return (OK);
}	/* Send_Packet ()	*/

/*************************************************************************
	Function		: . Write_Data
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure)
	Comment			: . write received data to file
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Write_Data (void)
/*----------------------------------------------------------------------*/
{
	int				i, next, seq, d_cnt, rt, len, f_size, rec_size;
	char			m_time[24], w_buf[FILE_BUF_LEN];
	char			*data = (char *)R_Pkt->Data;
	BUFF_RW_HEAD	f_head;

	d_cnt = AtoIf (R_Pkt->Head.DataCnt, sizeof (R_Pkt->Head.DataCnt));
	f_size = sizeof (BUFF_RW_HEAD);
	rec_size = f_size + DataSize + 1;

	memset (m_time, 0, sizeof (m_time));
	Get_MicroTime (m_time);

	memset (w_buf, ' ', sizeof (w_buf));

	for (i = 0, next = 0; i < d_cnt; i ++)
	{
		pTcp_Data_Head = (TCP_DATA_HEAD *)(data+next);

		seq = AtoIf (pTcp_Data_Head->DataSeq, sizeof (pTcp_Data_Head->DataSeq));

		if (seq != INT_SEQ + i + 1)
		{
			Log (USR_ERROR, "data header (DataSeq) [%d:%d]",
				seq, INT_SEQ + i + 1);
			memcpy (S_Pkt->ResponseCode, ERR_HEAD_D2, strlen (ERR_HEAD_D2));
			return (NOTOK);
		}

		len = AtoIf (pTcp_Data_Head->Length, sizeof (pTcp_Data_Head->Length));

		if (len <= HEAD_SIZE || len > HEAD_SIZE + DataSize)
		{
			Log (USR_ERROR, "data header (Length) [%d:%d]",
				len, HEAD_SIZE + DataSize);
			memcpy (S_Pkt->ResponseCode, ERR_HEAD_D1, strlen (ERR_HEAD_D1));
			return (NOTOK);
		}

		memcpy (f_head.DataHeader, data+next, HEAD_SIZE);
		ItoAf (INT_SEQ + i + 1, f_head.If_Seq, sizeof (f_head.If_Seq));
		memcpy (f_head.ApType, ApType, strlen (ApType));
		ItoAf (0, f_head.ResponseCode, sizeof (f_head.ResponseCode));
		memcpy (f_head.RecvTime1, m_time, sizeof (f_head.RecvTime1));
		memcpy (f_head.RecvTime2, &m_time[sizeof(f_head.RecvTime1)],
			sizeof (f_head.RecvTime2));

		memcpy (&w_buf[rec_size*i], &f_head, f_size);
		memcpy (&w_buf[rec_size*i+f_size], data+next+HEAD_SIZE,
			len - HEAD_SIZE);
		w_buf[rec_size*i+f_size+DataSize] = '\n';

		next += len;
	}

	SYS_NO = 0;

#ifdef MEM_USE
	rt = DSHM_W (TS_W1_1, w_buf, d_cnt);
#else
	rt = F_W (TS_W1_1, w_buf, d_cnt);
#endif

	if (rt != d_cnt)
	{
#ifdef MEM_USE
		Log (DSH_FATAL, "DSHM write[%s]", OUT_NAME);
#else
		Log (SAM_FATAL, "file write[%s]", OUT_NAME);
#endif
		memcpy (S_Pkt->ResponseCode, ERR_FILE_WRITE, strlen (ERR_FILE_WRITE));
		return (NOTOK);
	}

	INT_SEQ += d_cnt;
	Set_TR_Time ();
	Log (USR_OK, "data write[%s:%d:%d]", OUT_NAME, WR_CNT, d_cnt);
	ItoAf (d_cnt, S_Pkt->DataCnt, sizeof (S_Pkt->DataCnt));

	return (OK);
}	/* End of Write_Data ()	*/

/*************************************************************************
	Function		: . Write_Data_Elw
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure)
	Comment			: . write received data to file
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Write_Data_Elw (void)
/*----------------------------------------------------------------------*/
{
	int				i, next, seq, d_cnt, rt, len, f_size, rec_size, rec_size2;
	char			m_time[24], w_buf[FILE_BUF_LEN];
	char			*data = (char *)R_Pkt->Data;
	BUFF_RW_HEAD	f_head;

	d_cnt = AtoIf (R_Pkt->Head.DataCnt, sizeof (R_Pkt->Head.DataCnt));
	f_size = sizeof (BUFF_RW_HEAD);
	rec_size = f_size + DataSize + 1;
	rec_size2 = f_size + DataSize2 + 1;

	memset (m_time, 0, sizeof (m_time));
	Get_MicroTime (m_time);

	memset (w_buf, ' ', sizeof (w_buf));

	for (i = 0, next = 0; i < d_cnt; i ++)
	{
		pTcp_Data_Head = (TCP_DATA_HEAD *)(data+next);

		seq = AtoIf (pTcp_Data_Head->DataSeq, sizeof (pTcp_Data_Head->DataSeq));

		if (seq != INT_SEQ + i + 1)
		{
			Log (USR_ERROR, "data header (DataSeq) [%d:%d]",
				seq, INT_SEQ + i + 1);
			memcpy (S_Pkt->ResponseCode, ERR_HEAD_D2, strlen (ERR_HEAD_D2));
			return (NOTOK);
		}

		len = AtoIf (pTcp_Data_Head->Length, sizeof (pTcp_Data_Head->Length));

		if (len <= HEAD_SIZE || len > HEAD_SIZE + DataSize)
		{
			Log (USR_ERROR, "data header (Length) [%d:%d]",
				len, HEAD_SIZE + DataSize);
			memcpy (S_Pkt->ResponseCode, ERR_HEAD_D1, strlen (ERR_HEAD_D1));
			return (NOTOK);
		}

		memcpy (f_head.DataHeader, data+next, HEAD_SIZE);
		ItoAf (INT_SEQ + i + 1, f_head.If_Seq, sizeof (f_head.If_Seq));
		memcpy (f_head.ApType, ApType, strlen (ApType));
		ItoAf (0, f_head.ResponseCode, sizeof (f_head.ResponseCode));
		memcpy (f_head.RecvTime1, m_time, sizeof (f_head.RecvTime1));
		memcpy (f_head.RecvTime2, &m_time[sizeof(f_head.RecvTime1)],
			sizeof (f_head.RecvTime2));

		if (W_flag == 1)
		{
			memcpy (&w_buf[rec_size*i], &f_head, f_size);
			memcpy (&w_buf[rec_size*i+f_size], data+next+HEAD_SIZE,
				len - HEAD_SIZE);
			w_buf[rec_size*i+f_size+DataSize] = '\n';
		}
		/* 주문응답 24바이트를 추가로 만들어서 넣어준다.(9999로 넣는다) */
		else
		{
			/* 201011 추가. */
			/* 응답은 매체에 따라서 사이즈가 다양하다 이에 OFN의 Size+20으로 변경 */
			ItoAf (HEAD_SIZE + DataSize2, f_head.DataHeader, 4);

			memcpy (&w_buf[rec_size2*i], &f_head, f_size);

			/* KRX_HEAD_LEN (82) */
			memcpy (&w_buf[rec_size2*i+f_size], data+next+HEAD_SIZE, 82);
			/* 응답 24바이트 */
			memcpy (&w_buf[rec_size2*i+f_size+82],
				"999900000000000000000000", 24);
			/* 주문데이터 300바이트 */
			memcpy (&w_buf[rec_size2*i+f_size+82+24],
				data+next+HEAD_SIZE+82, 300);

			w_buf[rec_size2*i+f_size+DataSize2] = '\n';
		}

		next += len;
	}

	SYS_NO = 0;

	if (W_flag == 1)
		rt = DSHM_W (TS_W1_1, w_buf, d_cnt);
	else
		rt = F_W (TS_W1_1, w_buf, d_cnt);

	if (rt != d_cnt)
	{
		if (W_flag == 1)
			Log (DSH_FATAL, "DSHM write[%s]", OUT_NAME);
		else
			Log (SAM_FATAL, "file write[%s]", OUT_NAME);

		memcpy (S_Pkt->ResponseCode, ERR_FILE_WRITE, strlen (ERR_FILE_WRITE));
		return (NOTOK);
	}

	INT_SEQ += d_cnt;
	Set_TR_Time ();

	if (W_flag == 1)
		Log (USR_OK, "data write[%s:%d:%d]", OUT_NAME, WR_CNT, d_cnt);
	else
		Log (USR_OK, "data write[%s:%d:%d]", OFN(D_K,P_K,0), OFW_CNT(0,0), d_cnt);

	ItoAf (d_cnt, S_Pkt->DataCnt, sizeof (S_Pkt->DataCnt));

	return (OK);
}	/* End of Write_Data_Elw ()	*/

/*************************************************************************
	Function		: . Check_Header
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int (0:success, -1:failure)
	Comment			: . check validity of the received header
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Check_Header (void)
/*----------------------------------------------------------------------*/
{
	int		val;

	/* Stx (0x02)	*/
	if (R_Pkt->Head.Stx[0] != STX)
	{
		Log (USR_ERROR, "TCP header (Stx) [%#04x:%#04x]",
			R_Pkt->Head.Stx[0], STX);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T1, strlen (ERR_HEAD_T1));
		return (NOTOK);
	}

	/* Length (packet 총 길이)	*/
	val = AtoIf (R_Pkt->Head.Length, sizeof (R_Pkt->Head.Length));
	if (val != strlen (RecvPkt) || strlen (RecvPkt) <= TCP_HEAD_LEN)
	{
		Log (USR_ERROR, "TCP header (Length) [%d:%d]", val, strlen (RecvPkt));
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T2, strlen (ERR_HEAD_T2));
		return (NOTOK);
	}

	/* ApType (업무구분식별자)	*/
	if (memcmp (R_Pkt->Head.ApType, ApType, sizeof (R_Pkt->Head.ApType)) != 0)
	{
		Log (USR_ERROR, "TCP header (ApType) [%.8s:%s]",
			R_Pkt->Head.ApType, ApType);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T3, strlen (ERR_HEAD_T3));
		return (NOTOK);
	}

	/* ResponseCode (응답코드)	*/
	if (AtoIf (R_Pkt->Head.ResponseCode,
		sizeof (R_Pkt->Head.ResponseCode)) != 0)
	{
		Log (USR_ERROR, "TCP header (ResponseCode) [%.4s:%04d]",
			R_Pkt->Head.ResponseCode, 0);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T6, strlen (ERR_HEAD_T6));
		return (NOTOK);
	}

	/* MsgType (운영코드)	*/
	if (memcmp (R_Pkt->Head.MsgType, TCP_DATA_CD, strlen (TCP_DATA_CD)) != 0)
	{
		Log (USR_ERROR, "TCP header (MsgType) [%.4s:%s]",
			R_Pkt->Head.MsgType, TCP_DATA_CD);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T7, strlen (ERR_HEAD_T7));
		return (NOTOK);
	}

	/* SeqNo (일련번호)	*/
	val = AtoIf (R_Pkt->Head.SeqNo, sizeof (R_Pkt->Head.SeqNo));

	if (val != INT_SEQ + 1)
	{
		Log (USR_ERROR, "TCP header (SeqNo) [%d:%d]", val, INT_SEQ + 1);
		ItoAf (INT_SEQ, S_Pkt->SeqNo, sizeof (S_Pkt->SeqNo));
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T8, strlen (ERR_HEAD_T8));
		return (NOTOK);
	}

	/* DataCnt (한 packet 내 blocking된 data 건수)	*/
	val = AtoIf (R_Pkt->Head.DataCnt, sizeof (R_Pkt->Head.DataCnt));

	if (val < 1 || val > MaxCnt)
	{
		Log (USR_ERROR, "TCP header (DataCnt) [%d:%d]", val, MaxCnt);
		memcpy (S_Pkt->ResponseCode, ERR_HEAD_T9, strlen (ERR_HEAD_T9));
		return (NOTOK);
	}

	return (OK);
}	/* End of Check_Header ()	*/

/*************************************************************************
	Function		: . Register_Signal
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . register signals
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Register_Signal (void)
/*----------------------------------------------------------------------*/
{
	struct sigaction	act;

	sigemptyset (&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = Catch_Signal;

	if (sigaction (SIGPIPE, &act, NULL) < 0)
	{
		Log (SYS_ERROR, "sigaction (SIGPIPE) {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	if (sigaction (SIGTERM, &act, NULL) < 0)
	{
		Log (SYS_ERROR, "sigaction (SIGTERM) {%d:%s}", SYS_NO, SYS_STR);
		return;
	}

	return;
}	/* End of Register_Signal ()	*/

/*************************************************************************
	Function		: . Catch_Signal
	Parameters IN	: . signo : signal number
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . catch signal
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Catch_Signal (int signo)
/*----------------------------------------------------------------------*/
{
	Log (PRO_WARN, "signal (%d) occurred", signo);
	TCP1_NET_STA = OFF;
	close (Sockfd);
	Exit_Process ();
}	/* End of Catch_Signal ()	*/

/*************************************************************************
	Function		: . Set_Socket_Linger
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
	Comment			: . set linger option on socket
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Set_Socket_Linger (void)
/*----------------------------------------------------------------------*/
{
	int				rt;
	struct linger	ling;

    /* close () returns after discarding any unsent data	*/
	ling.l_onoff = 1;
	ling.l_linger = 0;

	rt = setsockopt (Sockfd, SOL_SOCKET, SO_LINGER, (char *)&ling,
		sizeof (ling));

	if (rt < 0)
		Log (TCP_ERROR, "setsockopt SO_LINGER {%d:%s}", SYS_NO, SYS_STR);

	return;
}

/*************************************************************************
	End of Program (pw_3000_tr.c)
*************************************************************************/
