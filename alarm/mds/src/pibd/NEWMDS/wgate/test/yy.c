static char sqla_program_id[292] = 
{
 '\xac','\x0','\x41','\x45','\x41','\x56','\x41','\x49','\x44','\x42','\x79','\x45','\x53','\x4c','\x4c','\x6f','\x30','\x31','\x31','\x31',
 '\x31','\x20','\x32','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x20','\x8','\x0','\x4b','\x45','\x49','\x30','\x30','\x30',
 '\x20','\x20','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x8','\x0','\x59','\x59','\x20','\x20','\x20','\x20','\x20','\x20','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0',
 '\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0','\x0'
};

#include "sqladef.h"

static struct sqla_runtime_info sqla_rtinfo = 
{{'S','Q','L','A','R','T','I','N'}, sizeof(wchar_t), 0, {'C',' ',' ',' '}};


static const short sqlIsLiteral   = SQL_IS_LITERAL;
static const short sqlIsInputHvar = SQL_IS_INPUT_HVAR;


#line 1 "yy.sqc"
#include "mds2.h"

#include "comdef.h"
#include "wfaapi.h"    // 실시간전송(Client)
#include "axsnd.h"
#include "mymq.h"
#include "agfix.h"
#include "agxpi.h"
#include "agtrc.h"
#include "comrdb.h"
#include "comhdr.h"
#include "comfun.h"

AG_FIX		s_afix;
Ag_Fix		s_xfix;
TFILE		s_trcf;
TFile	_trcf_	=  (TFile) &s_trcf;

char	*pname;

void exitproc(int );

int sise2mds(char *excode, char *pbuff, int dlen, MDSSISE *psise);
static int conv_SMBS(char *pbuff, int dlen, MDSSISE *psise);

/*===============================================================
 * EXITPROC
===============================================================*/
void exitproc(int exitval)
{
	printf("\nProgram Terminated.. exitval[%d]\n\n", exitval);

	l_db2disconnect();
	
	exit(exitval);
}

void Usage(int exitval)
{
	printf("\n\t Usage : %s [-pPORT] [-m224.0.0.1]\n", whoami);
	printf("\t   -p : 수신포트 \n");
	printf("\t   -m : 멀티캐스트IP \n");
	exitproc(exitval);
}

/*===============================================================
 * MAIN PROCESS
===============================================================*/
int main(int argc, char *argv[])
{
	int		rc;	
	int		mport;
	char	stemp[128], mipad[128];

	memset(mipad, 0x00, sizeof(mipad));
	memset(stemp, 0x00, sizeof(stemp));
	/*===============================================================
	 * PROCESS 초기화
	===============================================================*/
	if (argc < 3)	Usage(-1);
	
	for (ii=1; ii < argc; ii++)
	{
		memset(options, 0x00, sizeof(options));

		if (strncmp(argv[ii], "-p", 3) == 0)
		{
			strcpy(stemp, &argv[ii][3]);
			mport = atoi(stemp);
		}
		if (strncmp(argv[ii], "-m", 3) == 0)
		{
			strcpy(mipad, &argv[ii][3]);
		}
	}

	pname = basename(argv[0]);

	if (mport <= 0 || strlen(mipadd) <= 0)		Usage(-2);

	signal(SIGTERM, exitproc);

	APLog(pname, APLOG_DEBUG, "Start....!");

	/*===============================================================
	 * FIX 전문 변환용 로컬메모리 초기화
	===============================================================*/
	// init log file
	rc = agxpi_tfile_vinit (_trcf_, LOG_DIR, AG_NULL, AG_HOME, TRC_DIR, AGINI_GID, AGINI_SGI, AGINI_MYI,TDAT, DALL);
	agtrc_open (AG_NULL, _trcf_);
	
	// start message
	agtrc_msg (_trcf_, AGINI_GID, AGINI_SGI, AGINI_MYI, 0, 0, TDBG, "fixgetval START\n");
	mxzinit(s_afix);
	s_xfix = (Ag_Fix) &s_afix;

	rc = agfix_at (s_xfix, AFIX_MAT_INIT);
	if (rc)
	{
		agtrc_msg (_trcf_, AGINI_GID, AGINI_SGI, AGINI_MYI, 0, 0, TERR, "agfix_at ERROR-R:%d E:%d ...\n", rc, errno);
		exitproc(-5);
	}

	/*===============================================================
	 * UDP 서버기동 : 수신준비
	===============================================================*/
	int		rsock, bsize, options = 1;
	struct	sockaddr_in svrsock, clisock;
	struct	ip_mreq ip_mreq;
	socklen_t socklen = sizeof(svrsock);

	memset(&svrsock, 0, sizeof(svrsock));
	memset(&clisock, 0, sizeof(clisock));	

	if ((rsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		printf("Cannot open a socket for %s.%d\n", mipad, mport);
		return (-1);
	}

	options = 1;
	setsockopt(rsock, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options));
	
	bsize = 80 * 1024;
	setsockopt(rsock, SOL_SOCKET, SO_RCVBUF, &bsize, sizeof(int));

	svrsock.sin_family = AF_INET;
	svrsock.sin_addr.s_addr = INADDR_ANY;
	svrsock.sin_port = htons(mport);

	ip_mreq.imr_multiaddr.s_addr = inet_addr(mipad);
	ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(rsock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&ip_mreq, sizeof(ip_mreq)) != 0)
	{
		printf("Cannnot add a multicast member for %s.%d\n", mipad, mport);
		close(rsock);
		return (-1);
	}

	if (bind(rsock, (struct sockaddr *)&svrsock, sizeof(svrsock)) != 0)
	{
		printf("Cannot bind a socket for %s.%d\n", mipad, mport);
		close(rsock);
		return (-1);
	}
	
	APLog(pname, APLOG_DEBUG, "Start to receive from %s:%d", mipad, mport);

	/*===============================================================
	 * DB Connection
	===============================================================*/
	rc = l_db2connect();
	if (rc != 0)
	{
		APLog(pname, APLOG_DEBUG, "[%s] DB connect failed : %d\n",__func__, rc);
		exitproc(-13);
	}

	APLog(pname, APLOG_DEBUG, "[%s] DB connect success !", __func__);

	/*===============================================================
	 * MAIN PROCESS (시세수신)
	===============================================================*/
	int		rtn, dlen;
	char	rbuff[MAX_PACKET_SIZE+512];	// FIX DATA
	char	sbuff[512];				// MDSSISE
	char	ctime [14], rejmsg[128];
	struct	timeval rtime;
	struct tm *lt;
	
	MDSSISE	*psise;

	while (1)
	{
		/*===============================================================
		 * UDP 시세 수신
		===============================================================*/
		memset(rbuff, 0x00, sizeof(rbuff));
		
		dlen = recv(rsock, rbuff, sizeof(rbuff), 0);
		if (dlen < 0)
		{
			if (errno == EINTR || errno == EAGAIN)		continue;

			break;
		}

		gettimeofday(&rtime, NULL);
		lt = localtime(&rtime.tv_sec);

APLog(pname, APLOG_DEBUG, "[%s:%d] RECEIVE DATA (%d:%.*s)", __func__, arch->rsum, dlen, dlen, rbuff);

		/*===============================================================
		 * FIX FORMAT CONVERT
		===============================================================*/
		memset(sbuff, 0x00, sizeof(sbuff));

		rtn = sise2mds("S", rbuff, dlen, psise);
		if (rtn != 0)
		{
			APLog(pname, APLOG_DEBUG, "Receive Data Convert Error (%d)", rtn);
			continue;
		}

//-------------------------------------------------------------------------
// BIZ LOGIC
//-------------------------------------------------------------------------

	}

	APLog(pname, APLOG_DEBUG, "UDP 시세수신 BREAK. Call Termination function..");
	
	close(rsock);

	exitproc(-99);
}

/*===============================================================
 * 수신한 FIX 데이터를 내부 포맷으로 전환
===============================================================*/
static int conv_SMBS(char *pbuff, int dlen, MDSSISE *psise)
{
	int		ii, rc, nrec;
	uint32_t	ymd, hms;
	char    type[DF_2], strval[64], strval2[64];
	char	symbol[ 7+1];
	struct tm *lt;

	memset(symbol, 0x00, sizeof(symbol));
	psise->excode[0] = 'S';				// 원천 : 'S'=SMB, 'K'=KMB, 'E'=EBS, 'C'=CMB

// ★TODO-08 : 데이터 원천에 따라 FIX패킷 Convert 시 분기로직 추가 : sise_fix2ecm()

	AFIX_TOK_SETVATINT(s_xfix, MsgSeqNum,				AG_ZERO, (double) EOF);
	AFIX_TOK_SETVATINT(s_xfix, LastMsgSeqNumProcessed,	AG_ZERO, (double) EOF);
	AFIX_TOK_SETVATINT(s_xfix, BeginSeqNo,				AG_ZERO, (double) EOF);
	AFIX_TOK_SETVATINT(s_xfix, EndSeqNo,				AG_ZERO, (double) EOF);
	AFIX_TOK_SETVATINT(s_xfix, NewSeqNo,				AG_ZERO, (double) EOF);

	agfix_token_cls(s_xfix);
	agfix_set_define_leg (s_xfix, NoMDEntries, AG_ZERO, (int *) AG_NULL, (int *) AG_NULL, (double) EOF);
	
	pbuff[dlen] = 0x00;
	rc = agfix_dec (s_xfix, pbuff, dlen+1, SOH, EQU, AG_ZERO);		
	if (rc) 
 	{
		APLog(pname, APLOG_DEBUG, "[%s] agfix_dec error (%d) [%d:%s]",__func__, rc, dlen, pbuff);
		return (-1);
	}

	rc = AFIX_TOK_GETSTR (s_xfix, Symbol, AG_ZERO, symbol);
	if (rc != AG_OK)
	{
		APLog(pname, APLOG_DEBUG, "[%s] AFIX_TOK_GETSTR error (%d) [%d:%s]",__func__, rc, dlen, pbuff);
		return (-1);
	}
	//mds_time(market, 0, &market->xymd, &market->xhms, &market->kymd, &market->khms);
	sprintf(psise->symb, "%.3s%.3s", symbol, &symbol[4]);

	mds_time(market, 0, &ymd, &hms, NULL, NULL);	// get local date & time
	sprintf (psise->date, "%08d", ymd);

	struct timeb itb;
	ftime(&itb);

	lt = localtime(&itb.time);
	sprintf (psise->time, "%02d%02d%02d%03d",lt->tm_hour,lt->tm_min,lt->tm_sec,itb.millitm);
	//AFIX_TOK_GETSTR (s_xfix, SendingTime, AG_ZERO, g_time);	

	nrec = AG_ZERO;
	rc = AFIX_TOK_GETINT (s_xfix, NoMDEntries, AG_ZERO, nrec);
	if (rc != AG_OK)	nrec = AG_ONE;

	for (ii = 0; ii < nrec; ii++)
	{
		MXZINIT (type); 
		AFIX_TOK_GETSTR (s_xfix, MDEntryType, ii, type);

		switch (type[0])
		{
		case '0' :
// AFIX_TOK_GETDBL에서 잘못된 값이 리턴되어 STR로 처리후 변환
//			AFIX_TOK_GETDBL (s_xfix, 270, 			ii, psise->bidprc);
//			memset(strval, 0x00, sizeof(strval));
			AFIX_TOK_GETSTR (s_xfix, MDEntryPx, 	ii, strval);
			psise->bidprc = atof(strval);
//			AFIX_TOK_GETDBL (s_xfix, MDEntrySize, 	ii, psise->bidqty);
			AFIX_TOK_GETSTR (s_xfix, MDEntrySize, 	ii, strval);
			psise->bidqty = atof(strval);
			AFIX_TOK_GETSTR (s_xfix, MDEntryDate, 	ii, psise->biddate);
			AFIX_TOK_GETSTR (s_xfix, QuoteEntryID, 	ii, strval);		// psise->bidquoteid);
			sprintf(strval2, "%.1s.%.*s", psise->excode, sizeof(psise->quotid)-2, strval);
			memcpy(psise->quotid, strval2, strlen(strval2));			// excode + QuoteEntryID 로 채번
			AFIX_TOK_GETSTR (s_xfix, 9063,			ii, psise->bidsettype);
//			AFIX_TOK_GETDBL (s_xfix, 9064,			ii, psise->bidbestprc);
			AFIX_TOK_GETSTR (s_xfix, 9064,			ii, strval);
			psise->bidbestprc = atof(strval);
//			AFIX_TOK_GETDBL (s_xfix, 9065,			ii, psise->bidbestqty);
			AFIX_TOK_GETSTR (s_xfix, 9065,			ii, strval);
			psise->bidbestqty = atof(strval);
			//bid시세가 없어서 임시추가 

			if (memcmp(psise->symb, "USDKRW", 6) == 0)
			{
//				AFIX_TOK_GETDBL (s_xfix, 9065, ii, psise->bidqty);
				AFIX_TOK_GETSTR (s_xfix, 9065, ii, strval);
				psise->bidqty = atof(strval);
			}
			break;

		case '1' :
//			AFIX_TOK_GETDBL (s_xfix, 270, 			ii, psise->offerprc);
//			memset(strval, 0x00, sizeof(strval));
			AFIX_TOK_GETSTR (s_xfix, MDEntryPx, 	ii, strval);
			psise->offerprc = atof(strval);
//			AFIX_TOK_GETDBL (s_xfix, MDEntrySize, 	ii, psise->offerqty);
			AFIX_TOK_GETSTR (s_xfix, MDEntrySize, 	ii, strval);
			psise->offerqty = atof(strval);
			AFIX_TOK_GETSTR (s_xfix, MDEntryDate, 	ii, psise->offerdate);
//			AFIX_TOK_GETSTR (s_xfix, QuoteEntryID, 	ii, psise->offerquoteid);
			AFIX_TOK_GETSTR (s_xfix, 9063,			ii, psise->offersettype);
//			AFIX_TOK_GETDBL (s_xfix, 9064,			ii, psise->offerbestprc);
			AFIX_TOK_GETSTR (s_xfix, 9064,			ii, strval);
			psise->offerbestprc = atof(strval);
//			AFIX_TOK_GETDBL (s_xfix, 9065,			ii, psise->offerbestqty);
			AFIX_TOK_GETSTR (s_xfix, 9065,			ii, strval);
			psise->offerbestqty = atof(strval);
		 	//offer시세가 없어서 임시추가 

			if (memcmp(psise->symb, "USDKRW", 6) == 0)
			{
//				AFIX_TOK_GETDBL (s_xfix, 9065, ii, psise->offerqty);
				AFIX_TOK_GETSTR (s_xfix, 9065, ii, strval);
				psise->offerqty = atof(strval);
			}
			break;

		case '2' :	// 35=X (MarketData Incremantal Refresh for Trade Summary)
//APLog(pname, APLOG_DEBUG, "[%s] receive X=[%.*s]", __func__, dlen, pbuff);
			// MDEntryDate, MDEntryTime 수신 데이터로 UPDATE
			// TODO : MDUpdateAction(279) : New,Change,Delete 에 대한 처리 필요한지 확인
			// TODO : AggressorSide(5797) : 0=No aggressor 에 대한 처리 필요한지 확인
			AFIX_TOK_GETSTR (s_xfix, MDEntryDate, 	ii, psise->date);
			// GMT시간이므로 GMT+9로 변형해야 국내시간 나오므로 서버시간으로 사용
			//AFIX_TOK_GETSTR (s_xfix, MDEntryTime, 	ii, strval);		// HHMMSS -> HHMMSSsss
			//sprintf(strval2, "%.6s000", strval);
			sprintf (strval2, "%02d%02d%02d%03d",lt->tm_hour,lt->tm_min,lt->tm_sec,itb.millitm);
			memcpy(psise->time, strval2, sizeof(psise->time));

			char	AggSide[8];

			AFIX_TOK_GETSTR (s_xfix, 5797, 	ii, AggSide);	// 0=No aggressor, 1=Buy, 2=Sell
			if (AggSide[0] == '1')
			{
				AFIX_TOK_GETSTR (s_xfix, MDEntryPx, 	ii, strval);
				psise->bidprc   = atof(strval);
				AFIX_TOK_GETSTR (s_xfix, MDEntrySize, 	ii, strval);		// always '0'
				psise->bidqty   = atof(strval);
				//MDUpdateAction=279
				AFIX_TOK_GETSTR (s_xfix, MDEntryDate, 	ii, psise->biddate);
				//AggressorSide=5797
				AFIX_TOK_GETSTR (s_xfix, 9063,			ii, psise->bidsettype);
			}
			else
			{
				AFIX_TOK_GETSTR (s_xfix, MDEntryPx, 	ii, strval);
				psise->offerprc = atof(strval);
				AFIX_TOK_GETSTR (s_xfix, MDEntrySize, 	ii, strval);		// always '0'
				psise->offerqty = atof(strval);
				//MDUpdateAction=279
				AFIX_TOK_GETSTR (s_xfix, MDEntryDate, 	ii, psise->offerdate);
				//AggressorSide=5797
				AFIX_TOK_GETSTR (s_xfix, 9063,			ii, psise->offersettype);
			}

			AFIX_TOK_GETSTR (s_xfix, MDEntryID, 	ii, strval);
			// quotID[0] 을 'X'로 세팅하여 기존 시세 데이터와 구분 함.
			sprintf(strval2, "X%.1s.%.*s", psise->excode, sizeof(psise->quotid)-3, strval);
			memcpy(psise->quotid, strval2, strlen(strval2));			// excode + QuoteEntryID 로 채번

			//TradeDate=75
			//FutSettDate=64

			break;
		}
	}

	// 24.08.26) BEST 값이 없으면 AVAIL 값으로 대체함.
	if (psise->bidbestprc   <= 0.)	psise->bidbestprc   = psise->bidprc;
	if (psise->offerbestprc <= 0.)	psise->offerbestprc = psise->offerprc;

	return 0;
}

int sise2mds(char *excode, char *pbuff, int dlen, MDSSISE *psise)
{
	int		rc;

	switch (excode[0])
	{
	case 'S' :
		rc = conv_SMBS(pbuff, dlen, psise);
		if (rc < 0)
		{
			APLog(pname, APLOG_DEBUG, "[%s] conv_SMBS error ! [%d] [%.*s]", __func__, rc, dlen, pbuff);
			return (-1);
		}
		break;

	default :
		APLog(pname, APLOG_DEBUG, "[%s] excode error ! [%.1s]", __func__, excode);
		return (-1);
	}

	return 0;
}


