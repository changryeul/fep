#include "mds2.h"
#include "wfaapi.h"
#include "API_ST.h"

AG_DQUE		qin, qout;

char	*DQ_RCVRQ="APIQ_ORD_REQ";
char	*DQ_SNDRS="APIQ_ORD_RES";
int		SEM_RCVRQ=0xff170025;
int		SEM_SNDRS=0xff170026;

static API_MST2		*gmast;
static API_CUST		*gcust;

int setorder(char *sbuf);

void exitproc(int signo)
{
	int		rc;
	
	printf("\n program terminated..!!!! [%d]\n", signo);
	
	exit(0);
}

/*===============================================================
 * MAIN PROCESS
===============================================================*/
int main(int argc, char *argv[])
{
	int		ii, pos, seq=0;
	int		shmid, rc;
	int		slen, rwno[2];
	char	sbuf[1024], rbuf[1024], stemp[32];

	signal(SIGINT, (void *)exitproc);

	memset(sbuf, 0x00, sizeof(sbuf));
	memset(rbuf, 0x00, sizeof(rbuf));

	int		dsiz = 1024;

//////////////////////////////////////////////////// DQ INIT
	memset(&qout, 0x00, sizeof(qout));
	AG_DQUECLS(&qout);

	qout.xha |= AQ_XHA_OPN;
	qout.xha |= AG_XAS_LIV;

	memset(&qin, 0x00, sizeof(qin));
	AG_DQUECLS(&qin);

	qin.xha |= AQ_XHA_OPN;
	qin.xha |= AG_XAS_LIV;
	qin.xha |= AG_XAS_CFM;  // 필요 ? 

	rc = agdque_at(&qin, FILE_HOME, NULL, DQ_SNDRS, SEM_SNDRS, sizeof(rbuf));
	if (rc < 0)
		exitproc(-1);

	int		idx=0;
	rwno[0] = qin.hdr.wno;
	rc = agdque_rwi_io (&qin, idx, &rwno); 
	printf(" queue in. init read no [%d]->[%d]\n", qin.hdr.rno[idx], qin.hdr.wno);

//////////////////////////////////////////////////// SHM INIT
	shmid = shmget(DF_APIMST_KEY, 0, 0);
	if (shmid < 0)	{
		printf(" get shared memory error (%d/%s)\n", errno, strerror(errno));
		exitproc(-2);
	}
	gmast = (API_MST2 *)shmat(shmid, NULL, 0);
	if (gmast == NULL)	{
		printf(" shared memory attach error (%d/%s)\n", errno, strerror(errno));
		exitproc(-3);
	}
	gcust = (API_CUST *)((char *)gmast + sizeof(API_MST2));
/*
	API_CUST *pcust = gcust;
	for (ii=0; ii<gmast->apicnt; ii++, pcust++)
		printf("[%d] [%s:%s]\n", ii, pcust->compid, pcust->acno);
printf("----------------------------------------\n\n");
*/
//////////////////////////////////////////////////// SEND ORDER
NEWORD : 
	memset(sbuf, 0x00, sizeof(sbuf));
	if ((rc=setorder(sbuf)) < 0)
		goto NEWORD;

	// ORDER SEND
	rc = agdque_wrn (&qout, FILE_HOME, NULL, DQ_RCVRQ, SEM_RCVRQ, dsiz, sbuf, sizeof(API_ST));
	if (rc < 0) {
		printf("agdque_wrn error : %d\n", rc);
		exitproc(-5);
	}

	printf("\nsend order ok..!!!! [%d]\n", rc);

	API_ST *p = (API_ST *)rbuf;

	while (1)
	{
		memset(rbuf, 0x00, sizeof(rbuf));
		
		rc = agdque_rdn (&qout, FILE_HOME, NULL, DQ_SNDRS, SEM_SNDRS, rbuf, sizeof(rbuf), 0, (-1), &rwno);		// AQ_DWAITM -> (-1)
		if (rc == 0)	continue;
		else if (rc < 0) {
			printf("agdque_rdn error : %d\n", rc);
			exitproc(-6);
		}

printf ("RECV ===================================\n");
printf(" [R] MsgType      =[%.1s]\n",  p->api_MsgType     ); // TAG-35   메세지유형       : 'D'-신규, 'F'-취소, '9'-거부, '8'-주문확인 및 체결"
printf(" [R] SenderCompID =[%s]\n",    p->api_SenderCompID); // TAG-49   송신회사ID       : SenderCompId
printf(" [R] TargetCompID =[%s]\n",    p->api_TargetCompID); // TAG-56   KB ID            : TargetCompId
printf(" [R] Account      =[%s]\n",    p->api_Account     ); // TAG-1    계좌             : Dealer ID
printf(" [R] ClOrdID      =[%s]\n",    p->api_ClOrdID     ); // TAG-11   기관주문번호     : 기관 주문번호
printf(" [R] OrdID        =[%s]\n",    p->api_OrdID       ); // TAG-37   주문번호         : KB 주문번호
printf(" [R] OrderQty     =[%s]\n",    p->api_OrderQty    ); // TAG-38   주문수량         :
printf(" [R] LastQty      =[%s]\n",    p->api_LastQty     ); // TAG-32   체결수량         : 
printf(" [R] ExecID       =[%s]\n",    p->api_ExecID      ); // TAG-17   채결ID           : KB 체결번호
printf(" [R] LastPx       =[%s]\n",    p->api_LastPx      ); // TAG-31   체결가격         : 
printf(" [R] Price        =[%s]\n",    p->api_Price       ); // TAG-44   주문가격         : 
printf(" [R] Commission   =[%s]\n",    p->api_Commission  ); // TAG-12   수수료           : 
printf(" [R] Side         =[%.1s]\n",  p->api_Side        ); // TAG-54   매매구분         : '1'-BUY, '2'-SELL
printf(" [R] OrdType      =[%.1s]\n",  p->api_OrdType     ); // TAG-40   주문유형         : '1'-Market '2'-Limit
printf(" [R] Symbol       =[%.6s]\n",  p->api_Symbol      ); // TAG-55   상품코드         : [Primary Currency]/[Counter Currency]
printf(" [R] Text         =[%.50s]\n", p->api_Text        ); // TAG-58   상세텍스트       :
printf(" [R] TimeInForce  =[%.1s]\n",  p->api_TimeInForce ); // TAG-59   체결조건         : '0'-For Day
printf(" [R] TransactTime =[%s]\n",    p->api_TransactTime); // TAG-60   주문일시         : '20180425-12:05:14.256'
printf(" [R] ValueDate    =[%.8s]\n",  p->api_ValueDate   ); // TAG-64   결제일자         : Specific date of trade settlement in YYYYMMDD format.
printf(" [R] OrdStatus    =[%.1s]\n",  p->api_OrdStatus   ); // TAG-39   주문상태         : '0'-NEW
printf(" [R] ExecType     =[%.1s]\n",  p->api_ExecType    ); // TAG-150  거래유형         : '0'-New
printf(" [R] RefuslCd     =[%s]\n",    p->api_RefuslCd    ); //          거부코드         : 거부코드 '90':한도초과 오류
printf("\n\n");
		if (p->api_MsgType[0] == '9' || p->api_OrdStatus[0] == '4' || p->api_OrdStatus[0] == '8' || p->api_OrdStatus[0] == '2')
		{
			printf("FINISHED >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");
			goto NEWORD;
		}
	}

	exitproc(0);
}

int setorder(char *pbuf)
{
	int		ii, pos;
	char	stemp[128];
	time_t	t;
	struct tm *tm;

	API_ST *p = (API_ST *)pbuf;

	t = time(NULL);
	tm = localtime(&t);

	API_CUST *pcust = gcust;
	for (ii=0; ii<gmast->apicnt; ii++, pcust++)
		printf("[%d] [%s:%s]\n", ii, pcust->compid, pcust->acno);
	printf("\n no : ");
	scanf("%d", &pos);
	if (pos < 0 || pos >= gmast->apicnt) 
	{ printf("\n\tinput error..\n\n\n"); return (-1); }
	
	pcust = (gcust + pos);

	printf("\n========= SET ORD ==========\n");
	printf("symbol : ");
	scanf("%s", stemp);
	MEMCPY(p->api_Symbol, stemp);
	memcpy(p->api_Currency, p->api_Symbol, 3);

	printf("side ('1'=BUY '2'=SELL) : ");
	scanf("%s", stemp);
	if (stemp[0] != '1' && stemp[0] != '2')
	{ printf("\n\tinput error..\n\n\n"); return (-1); }
	p->api_Side[0] = stemp[0];

	printf("qty : ");
	scanf("%s", stemp);
	MEMCPY(p->api_OrderQty, stemp);

	printf("ordtype ('1'=Market '2'=Limit) : ");
	scanf("%s", stemp);
	if (stemp[0] != '1' && stemp[0] != '2')
	{ printf("\n\tinput error..\n\n\n"); return (-1); }
	p->api_OrdType[0] = stemp[0];

if (p->api_OrdType[0] == '2') {
	printf("prc : ");
	scanf("%s", stemp);
	MEMCPY(p->api_Price, stemp);
}

	printf("timeinforce ('0'=Day '3'=IOC '4'=FOK) : ");
	scanf("%s", stemp);
	if (stemp[0] != '0' && stemp[0] != '3' && stemp[0] != '4')
	{ printf("\n\tinput error..\n\n\n"); return (-1); }
	p->api_TimeInForce[0] = stemp[0];

	//======================================
	p->api_MsgType[0] = 'D';
	MEMCPY(p->api_SenderCompID, pcust->compid);
	MEMCPY(p->api_Account, pcust->acno);
	memcpy(p->api_SettType, "SP", 2);
	sprintf(stemp, "%02d%02d%02d%02d", tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	memcpy(p->api_ClOrdID, stemp, strlen(stemp));

printf ("SEND ===================================\n");
printf(" [S] MsgType      = [%.1s]\n", p->api_MsgType     ); // TAG-35   메세지유형       : 'D'-신규, 'F'-취소, '9'-거부, '8'-주문확인 및 체결"
printf(" [S] SenderCompID = [%s]\n",   p->api_SenderCompID); // TAG-49   송신회사ID       : SenderCompId
printf(" [S] Account      = [%s]\n",   p->api_Account     ); // TAG-1    계좌             : Dealer ID
printf(" [S] ClOrdID      = [%s]\n",   p->api_ClOrdID     ); // TAG-11   기관주문번호     : 기관 주문번호
printf(" [S] OrdID        = [%s]\n",   p->api_OrdID       ); // TAG-37   주문번호         : KB 주문번호
printf(" [S] OrderQty     = [%s]\n",   p->api_OrderQty    ); // TAG-38   주문수량         :
printf(" [S] Price        = [%s]\n",   p->api_Price       ); // TAG-44   주문가격         : 
printf(" [S] Side         = [%.1s]\n", p->api_Side        ); // TAG-54   매매구분         : '1'-BUY, '2'-SELL
printf(" [S] OrdType      = [%.1s]\n", p->api_OrdType     ); // TAG-40   주문유형         : '1'-Market '2'-Limit
printf(" [S] Symbol       = [%.6s]\n", p->api_Symbol      ); // TAG-55   상품코드         : [Primary Currency]/[Counter Currency]
printf(" [S] TimeInForce  = [%.1s]\n", p->api_TimeInForce ); // TAG-59   체결조건         : '0'-For Day

	return 0;
}