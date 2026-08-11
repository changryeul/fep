/*------------------------------------------------------------------------
#   System  : HANWHA FEP 
#	Module	: display KRX error message 
#	File	: display_errmsg.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*************************************************************************
	Function		: . display KRX error message
	Parameters IN	: . p_code		: integer
					: . p_firstseq	: integer
	Return Code		: .	
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Display_ErrMsg (int p_code, int p_firstseq)
/*----------------------------------------------------------------------*/
{
	switch (p_code)
	{
		/* 세션 */
		case    0:  /* 정상 */
			break;
		case    1:  
			Log (USR_ERROR, "사용자검증(ID,PASSWORD)오류[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;
		case    2: 
			Log (USR_ERROR, "세션메시지전문수순오류[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;
		case    3:  
			Log (USR_ERROR, "헤더회원번호오류[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;
		case    4: 
			Log (USR_ERROR, "데이터일련번호오류[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;
		case    5: 
			Log (USR_ERROR, "데이타건수오류[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;
		case    6: /* Batch송수신시 TR별 종료(999~) 전송후 동일 TR 재송신 */ 
			Log (USR_WARN, "업무기마감오류[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;
		case    7:
			Log (USR_ERROR, "온라인개시이전[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;
		case    8:
			Log (USR_ERROR, "세션전문메시지타입오류[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;
		case    9: 
			Log (USR_ERROR, "업무처리중[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;
		case	10: 
			Log (USR_ERROR, "헤더메세지길이오류[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;
		case	11: 
			Log (USR_ERROR, "암복호화관련오류[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;
		case	12: 
			Log (USR_ERROR, "Message Body내에 Null데이타존재[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;

		/* 공통 */
		case	90: 
			Log (USR_ERROR, "시스템오류[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;

		/* 업무 */
		case  101:  
			Log (USR_OK, "호가접수개시전[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;
		case  102:  
			Log (USR_WARN, "매매거래시간종료후[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;
		case  103:  
			Log (USR_ERROR, "호가접수일시중지[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;
		default:
			Log (USR_ERROR,
				"RP_DATA recv:unknown 거부사유코드[%d] <%d:%d:%d>",
				p_code, p_firstseq, INT_SEQ, LOAD_CNT);
			break;
	}

}	/* End of Display_ErrMsg ()	*/

/*************************************************************************
	End of Program (display_errmsg.c)
*************************************************************************/
