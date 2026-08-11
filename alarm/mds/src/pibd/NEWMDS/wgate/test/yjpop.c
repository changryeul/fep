//#include "mds.h"
//#include "mdfold.h"

#include "comrdb.h"
#include "jnlfile.h"
#include "API_ST.h"


// Defines ===========================================================

struct message {
	long	mtype;
	unsigned char	mtext[1024];
};

#define		LOG_DIR			"/fslog/kei/wfg"

#define		DF_JNL_PATH		"/fsfile/kei/fxwin/jnl"
#define		DF_ORD_JNL		"API_JNL_ORD"
#define		DF_EXC_JNL		"API_JNL_EXC"

#define		ENV_DIR			"/fsfxwin/wfg/env"
#define		ENV_FNM			"APIORDKEY.cfg"

#define		SQL_CNT		sqlca.sqlerrd[2]

#define		DF_API_BUY		'1'
#define		DF_API_SEL		'2'

#define		DF_MX_FEE		50
#define		ORD_PER_SEC		2		// 초당 주문 가능 건수

typedef struct {
	char	compid			[10];	// FCMID (Tag49=SenderCompId)
	char	lgen_no			[10];	// 고객구분에 따라 시장참여자번호(10) OR 트레이더번호(4)
	int		trdstat			;		// 거래상태 구분 (한도초과, 관리자 중지 등)
	struct {
		char	fx_prdct_cd		[ 6];	// FX상품코드
		double	bid_spot_mrgn	;		// BID마진(수수료)
		double	ask_spot_mrgn	;		// ASK마진(수수료)
	} fee[DF_MX_FEE];
	struct timeval	tv		[ORD_PER_SEC];	// 직전,전직전 주문시간
} API_MST;

enum {
	GB_ORDNO = 1,
	GB_EXCNO
};

// TODO : 로그인 SKIP을 위한 임시 변경
enum {
	TRD_OFF	= 1, // = 0,
	TRD_ON = 0
};

enum {
	ERR_FEE			= 10,
	ERR_ORD_CNT		,
	ERR_SYMB_NODATA ,
	ERR_ORD_LIMIT	= 90		// 한도초과 오류.. 주문거부대상
};


// Global Variables ========================================================
API_MST		mast;

char	*whoami="ApiOrdGate";


/*===============================================================
 * MAIN PROCESS
===============================================================*/
int main(int argc, char *argv[])
{
	int		ii, rc;	
	char	options[32], *aphome;

	memset(&mast, 0x00, sizeof(mast));

	/*===============================================================
	 * JOURNAL INIT	  
	===============================================================*/
	JNLF	ojnl;
	
	aphome = getenv("APPL_HOME");
	
	// ORDER SEND JOURNAL
	memset(&ojnl, 0x00, sizeof(ojnl));
//	sprintf(ojnl.jfname, "%s/%s", DF_JNL_PATH, "YARI"); //DF_ORD_JNL);
	sprintf(ojnl.jfname, "%s/%s", DF_JNL_PATH, DF_ORD_JNL);

int idx=1;

if (argc > 1)	idx=atoi(argv[1]);
printf("index=[%d]\n", idx);

	rc = JFopen(&ojnl, idx, 0);
	if (rc != 0)
	{
		printf("jfopen error.. (%d:%s)\n", rc, strerror(errno));
		exit(1);
	}

	rc = JFseek(&ojnl, 0);
	if (rc != 0)
	{
		printf("jfseek error.. (%d:%s)\n", rc, strerror(errno));
		exit(1);
	}

exit(0);

	int		dlen, size, msqid;
	char	rbuff[1024+1];
	API_ST	*p;
	
	memset(rbuff, 0x00, sizeof(rbuff));
	p = (API_ST *)rbuff;
	
	ojnl.rcvb = rbuff;
//	ojnl._indx = idx;
	
	while (1)
	{
		memset(rbuff, 0x00, sizeof(rbuff));

		rc = JFpop(&ojnl, 0);	//600);
		if (rc <= 0)	break;
//		printf("jfpop [%d:%d] [%s]\n", rc, ojnl.jferrn, rbuff);
/*
printf("MsgType     =[%.*s]\n",sizeof(p->api_MsgType     ), p->api_MsgType        ); // TAG-35   메세지유형       : 'D'-신규, 'F'-취소, '3'-거부, '8'-주문확인 및 체결"  
printf("SenderCompID=[%.*s]\n",sizeof(p->api_SenderCompID), p->api_SenderCompID   ); // TAG-49   송신회사ID       : SenderCompId                                         
printf("TargetCompID=[%.*s]\n",sizeof(p->api_TargetCompID), p->api_TargetCompID   ); // TAG-56   KB ID            : TargetCompId                                         
printf("ClOrdID     =[%.*s]\n",sizeof(p->api_ClOrdID     ), p->api_ClOrdID        ); // TAG-11   기관주문번호     : 기관 주문번호                                        
printf("OrigClOrdID =[%.*s]\n",sizeof(p->api_OrigClOrdID ), p->api_OrigClOrdID    ); // TAG-41   기관원주문번호   : 기관 원주문번호                                      
printf("OrdID       =[%.*s]\n",sizeof(p->api_OrdID       ), p->api_OrdID          ); // TAG-37   주문번호         : KB 주문번호                                          
printf("OrigOrdID   =[%.*s]\n",sizeof(p->api_OrigOrdID   ), p->api_OrigOrdID      ); // TAG-37   원주문번호       : KB 원주문번호                                          
printf("OrderQty    =[%.*s]\n",sizeof(p->api_OrderQty    ), p->api_OrderQty       ); // TAG-38   주문수량         :                                                      
printf("LastQty     =[%.*s]\n",sizeof(p->api_LastQty     ), p->api_LastQty        ); // TAG-32   체결수량         :                                                      
printf("ExecID      =[%.*s]\n",sizeof(p->api_ExecID      ), p->api_ExecID         ); // TAG-17   채결ID           : KB 체결번호                                          
printf("LastPx      =[%.*s]\n",sizeof(p->api_LastPx      ), p->api_LastPx         ); // TAG-31   체결가격         :                                                      
printf("Price       =[%.*s]\n",sizeof(p->api_Price       ), p->api_Price          ); // TAG-44   주문가격         :                                                      
printf("Side        =[%.*s]\n",sizeof(p->api_Side        ), p->api_Side           ); // TAG-54   매매구분         : '1'-BUY, '2'-SELL                                    
printf("OrdType     =[%.*s]\n",sizeof(p->api_OrdType     ), p->api_OrdType        ); // TAG-40   주문유형         : Always 2                                             
printf("Symbol      =[%.*s]\n",sizeof(p->api_Symbol      ), p->api_Symbol         ); // TAG-55   상품코드         : [Primary Currency]/[Counter Currency]                
printf("Text        =[%.*s]\n",sizeof(p->api_Text        ), p->api_Text           ); // TAG-58   상세텍스트        :                                                     
printf("TransactTime=[%.*s]\n",sizeof(p->api_TransactTime), p->api_TransactTime   ); // TAG-60   주문일시         : '20180425-12:05:14.256'
printf("OrdStatus   =[%.*s]\n",sizeof(p->api_OrdStatus   ), p->api_OrdStatus      ); // TAG-39   주문상태         : '0'-NEW, '1'-Partially filled, '2'-Filled, '4'-Canceled, '8'-Rejected"
printf("ExecType    =[%.*s]\n",sizeof(p->api_ExecType    ), p->api_ExecType       ); // TAG-150  거래유형         : '0'-New, '4'-Canceled, '8'-Rejected(거절), 'I'-Order Status, 'F'-Filled"
printf("RefuslCd    =[%.*s]\n",sizeof(p->api_RefuslCd    ), p->api_RefuslCd       ); //          거부코드         : 거부코드 '90':한도초과 오류
printf("===========================================================");
*/
	}
	
	exit(0);
}

