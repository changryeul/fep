//
// fetch.c
//
#include <dirent.h>

#include "context.h"
#include "stream.h"
#include "mdfold.h"
#include "cmbstrn.h"
#include "axsnd.h"

#include  "mdcommon.h"
#include <time.h>
#include <sys/timeb.h>
//#include "etcsvc.h"
//static  struct  cmbsquote   g_rinfo;
static struct cmbsquote*  g_quote;
// transaction dispatch tables



#define  X_PUSH  0x01
#define  X_SYNC  0x02
#define  P_PUSH  0x04
#define  X_NOTI  0x08

#define FlotePoint  8

//static void keycopy(char *to, char *from, struct keydesc *keydesc)
//{
//  int  off, len;
//  int  ii;
//
//  for (ii = 0; ii < keydesc->k_nparts; ii++)
//  {
//    off = keydesc->k_part[ii].kp_start;
//    len = keydesc->k_part[ii].kp_leng;
//
//    memcpy(&to[off], &from[off], len);
//  }
//}

static int _fetch0_(MARKET *market, ISAM *isam, char *record, int mode, int limit)
{
  return(0);
}

//
// _fetch1_()
// Fetch TICK_TYPE CISAM records
//
static int _fetch1_(MARKET *market, ISAM *isam, char *record, int mode, int limit, int xymd)
{
  return(0);
}

//
// _fetch2_()
// Fetch HEOD_TYPE CISAM records
//
static int _fetch2_(MARKET *market, ISAM *isam, char *record, int mode, int limit, int xymd)
{
  return(0);
}
//
// mds_isfetch()
// Initialize CISAM data file
//
int mds_isfetch(MARKET *market, int dbid, void *record, int mode, int limit, int cflag)
{
  return(0);
}

int mds_fetch(MARKET *market, int dbid, void *record, int mode, int limit, int cflag)
{
  return(0);
}





/******************************************************************************
* Function Name : sendfolder(MARKET *market, MDFOLD *folder, char *check, int iSource)
* Description   : 테이블에 시세를 저장하고 클라이언트로 실시간 시세를 전송 한다. 
******************************************************************************/
void sendfolder(MARKET *market, MDFOLD *folder, char *check, int iSource)
{
   
    MDCTX   *ctx = market->ctx;
    int ii = 0, ibidzeroCount = 0, iaskzeroCount = 0;
    int nRet =0;
    int count = 0;
    char    form[16];
    //if (ctx->posta.sock >= 0)
        
    if (market->xchg->notify.cast)
    {
  //  char    send_b[8092];
    //send to transfer
    if (check[QUOT] & X_PUSH)
    {  
      S_SENDQUOT sendquot;
    //  send_b[0] = 'Q';
      memset(&sendquot, 0, sizeof(S_SENDQUOT));
      sprintf(sendquot.symb, "%.*s%.3s" , SYMB_LEN,folder->quot.symb,subCodeSpot[0]);
      sendquot.xymd = folder->quot.xymd;
      sendquot.xhms = folder->quot.xhms;
      sendquot.kymd = folder->quot.kymd;
      sendquot.khms = folder->quot.khms;
      memcpy(&sendquot.pricedata,   &folder->quot.spotdata, sizeof(folder->quot.spotdata));  
    //  memcpy(&send_b[1], &sendquot, sizeof(S_SENDQUOT));
      //Q는 안보냄 .
    //  sendto(ctx->posta.sock, send_b, sizeof(S_SENDQUOT)+1, 0, (struct sockaddr *)&ctx->posta.sin, sizeof(struct sockaddr_in));
      //DB에 데이터 저장  TICK만 저장
      INSERTTICKDATA(market, &sendquot, sizeof(S_SENDQUOT),folder->mstr.pind, iSource);

 //     for (count = 0; count < MAX_TENNER -1; count++)
 //     {            
 //       sprintf(sendquot.symb, "%.*s%.*s" , SYMB_LEN,folder->quot.symb,SYMB_SUBLEN,subFowardode[count]);
 //       memcpy(&sendquot.pricedata,   &folder->quot.foworddata[count], sizeof(folder->quot.foworddata[count]));  
      //  memcpy(&send_b[1], &sendquot, sizeof(S_SENDQUOT));
   //     nRet = INSERTTICKDATA(market,&sendquot, sizeof(S_SENDQUOT),folder->mstr.pind, iSource);

 //     }      
    }
    

  
    S_SENDBOOK sendbook;
    if (check[BOOK] & X_PUSH)
    {
      memset(&sendbook, 0, sizeof(S_SENDBOOK));
      sprintf(sendbook.symb,  "%.*s" , SYMB_LEN,folder->book.symb);
      
      sprintf(sendbook.xymd, "%08d", folder->book.xymd);  // date
      sprintf(sendbook.xhms, "%06d", folder->book.xhms/1000);  // time
      ibidzeroCount = 0, iaskzeroCount = 0;  
      double dCask = folder->book.spotdata.cask;
      double dCbid =  folder->book.spotdata.cbid;
          
      sprintf(form, "%%*.%df", folder->mstr.zdiv);
      for (ii = 0; ii <BOOK_LEVEL;ii++)
      {
        //가능수량이 0보다 작은경우 수량은 최소수량 
        if (folder->book.spotdata.bid[ii].vbid - dCbid< 0.00000001)
        {
        //  ibidzeroCount++;
          sprintf(sendbook.price[ii].pbid ,  form,    sizeof(sendbook.price[ii-ibidzeroCount].pbid),    folder->book.spotdata.bid[ii].pbid);
          sprintf(sendbook.price[ii].vbid ,  "%f" , folder->mstr.dBidBaseAmount);
        }else
        {
          
          sprintf(sendbook.price[ii].pbid ,  form,    sizeof(sendbook.price[ii-ibidzeroCount].pbid),    folder->book.spotdata.bid[ii].pbid);
          sprintf(sendbook.price[ii].vbid ,  "%f" , folder->book.spotdata.bid[ii].vbid - dCbid);    

        }
               
            
        if (folder->book.spotdata.ask[ii].vask -  dCask < 0.00000001)
        {
        //  iaskzeroCount++;
          sprintf(sendbook.price[ii].pask ,  form , sizeof(sendbook.price[ii-iaskzeroCount].pask),    folder->book.spotdata.ask[ii].pask);
          sprintf(sendbook.price[ii].vask ,  "%f" , folder->mstr.dAskBaseAmount);
        }else
        {
            
          sprintf(sendbook.price[ii].pask ,  form , sizeof(sendbook.price[ii-iaskzeroCount].pask),    folder->book.spotdata.ask[ii].pask);
          sprintf(sendbook.price[ii].vask ,  "%f" , folder->book.spotdata.ask[ii].vask -  dCask);  

        }    
        
        dCbid = dCbid -  folder->book.spotdata.bid[ii].vbid  ;    
        if (dCbid < 0.00000001)
          dCbid = 0;  
        dCask = dCask -  folder->book.spotdata.ask[ii].vask ;    
        if (dCask < 0.00000001)
          dCask = 0;    
        if (ii-iaskzeroCount > DISPLAYBOOK_LEVEL)
        {
          break;
        }
      }
    //  send_b[0] = 'B';
    //  memcpy(&send_b[1], &sendbook, sizeof(S_SENDBOOK));
    //  sendto(ctx->posta.sock, send_b, sizeof(S_SENDBOOK)+1, 0, (struct sockaddr *)&ctx->posta.sin, sizeof(struct sockaddr_in));
      sprintf(sendbook.symb, "%.*s%.3s" , SYMB_LEN,folder->book.symb,subCodeSpot[0]);
      upsert(market, folder, check);
    }
  }
}





/******************************************************************************
* Function Name : upsert(MARKET *market, MDFOLD *folder, char *check)
* Description   : 클라이언트로 실시간 메시지를 전송하는 함수를 호출한다. 
******************************************************************************/
void upsert(MARKET *market, MDFOLD *folder, char *check)
{
  
  if (folder == NULL)
    return;

  if (check[MSTR] & X_SYNC)
    mds_syncfolder(market, folder, MSTR); 

  if (check[QUOT] & X_PUSH)
  {
    if (!market->xchg->qfil)
    {
      mds_pushfolder(market, folder, PUSH_QUOT);

    }
  }
  if (check[QUOT] & X_SYNC)
  {
    //mds_log(market, LOG_DEBUG, "[%s] QUOT check:%x", folder->symb, check[QUOT]);
    mds_syncfolder(market, folder, QUOT);
  }

  if (check[BOOK] & X_PUSH)
  {

    if (!market->xchg->bfil)
    {
      mds_pushfolder(market, folder, PUSH_BOOK);  
      
    }
    
  }
}




//
// p_leadmonth()
// Copy quote to lead month symbol
//
static void p_leadmonth(MARKET *market, MDFOLD *folder, char *check)
{
  MDFOLD  *mdfold;
  char  symb[SYMB_LEN];
  char  enam[128+1], snam[128+1], knam[128+1];
  char  *f, *t;
  int  l;

#if 0
  if (market->xchg->type != XT_FUTURE)
    return;

  if (!(folder->mstr.jchk & JCHK_LM))
    return;

  sprintf(symb, "%s%s", folder->mstr.root, LM_SUFFIX);
  if ((mdfold = mds_getfolder(market, symb)) == NULL)
    return;

  if (check[MSTR])
  {
    f = (char *)&folder->mstr;
    t = (char *)&mdfold->mstr;
    l = sizeof(MDMSTR) - SYMB_LEN;
    f += SYMB_LEN;
    t += SYMB_LEN;
    strcpy(enam, mdfold->mstr.enam);    // save name
    strcpy(snam, mdfold->mstr.snam);
    strcpy(knam, mdfold->mstr.knam);
    memcpy(t, f, l);

    mdfold->mstr.seqn = 0;        // reset symbol's sequence
    strcpy(mdfold->mstr.enam, enam);    // restore name
    strcpy(mdfold->mstr.snam, snam);
    strcpy(mdfold->mstr.knam, knam);

    mds_syncfolder(market, mdfold, MSTR);

    // for settle
    f = (char *)&folder->quot;
    t = (char *)&mdfold->quot;
    l = sizeof(MDQUOT) - SYMB_LEN;
    f += SYMB_LEN;
    t += SYMB_LEN;
    memcpy(t, f, l);
  }
  if (check[QUOT])
  {
    f = (char *)&folder->quot;
    t = (char *)&mdfold->quot;
    l = sizeof(MDQUOT) - SYMB_LEN;
    f += SYMB_LEN;
    t += SYMB_LEN;
    memcpy(t, f, l);

/*
    if (check[QUOT] & X_PUSH)
      mds_pushfolder(market, folder, PUSH_QUOT);
*/
    if (check[QUOT] & X_SYNC)
      mds_syncfolder(market, mdfold, QUOT);
  }
 
  if (check[BOOK])
  {
    f = (char *)&folder->book;
    t = (char *)&mdfold->book;
    l = sizeof(MDBOOK) - SYMB_LEN;
    f += SYMB_LEN;
    t += SYMB_LEN;
    memcpy(t, f, l);
    mds_syncfolder(market, mdfold, BOOK);
  }
#endif
  /*
  if (check[HEOD])
    cmexeod(market, mdfold, check);
  */
}

//
// mds_dbglog()
// Print formating string for debugging
//
int mds_dbglog(MARKET *market, char *pname, const char *format, ...)
{
  FILE  *logF;
  time_t  clock;
  struct  tm tm;
#ifdef  LOG_WITH_WDAY
  struct  tm tx;
  struct  stat lstat;
#endif
  char  logmsg[5*1024], logpath[128], lstr[40], mode[8];
  va_list  vl;

  if (strlen(market->procname) <= 0)
    return(-1);

  clock = time(0);
  clock += market->e2lt;
  localtime_r(&clock, &tm);

  sprintf(logpath, "%s/%s.%02d%02d", TMP_DIR, pname, tm.tm_mon+1, tm.tm_mday);

  sprintf(mode, "a");
  if ((logF = fopen(logpath, mode)) == NULL)
    return(-1);

  
  fprintf(logF, "[%02d/%02d %02d:%02d:%02d] ", 
    tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec); 

  va_start(vl, format);
  vfprintf(logF, format, vl);
  fprintf(logF, "\n");
  va_end(vl);
  fclose(logF);

  return(0);
}

/******************************************************************************
* Function Name : MDFOLD *pibo_getfolder2(char *exnm, char *symb, MARKET **market)
* Description   : 해당종목의 폴더정보를 읽기 전용으로 가지고 온다. 
******************************************************************************/
MDFOLD *pibo_getfolder2(char *exnm, char *symb, MARKET **market)
{
  int    ii, jj;
  char  ksymb[16];
  MDFOLD  *folder;
  MARKET  *m;

  if (exnm != NULL && strlen(exnm) > 0)
  {
    m = mds_open(exnm, O_RDONLY);
    if (m == NULL)
      return NULL;

    folder = mds_getfolder(m, symb);
    if (market != NULL)
      *market = m;
    return folder;
  }
  else
  {
    XCHG *xchg;
    xchg = mds_exchanges();

    for (ii = 0; strlen(xchg[ii].exnm) != 0; ii++)
    {
      MARKET *m = mds_open(xchg[ii].exnm, O_RDONLY);
      if (m == NULL) continue;

      folder = mds_getfolder(m, symb);
      if (folder != NULL)
      {
        if (market != NULL)
          *market = m;
        return folder;
      }
    }
    if (market != NULL)
      *market = NULL;
    return NULL;
  }
}



//
//extern int cmetymd(MARKET *market, MDFOLD *folder, uint32_t tymd, char *check);
//
// optbook()
// Receive market book data of Option format
//
//MDFOLD *getSpotbook(MARKET *market , char *msgb, int msgl, char *check)
//{
//  //struct  books *books = (struct books *)msgb;
//
//  struct  cmbsquote *quote = (struct cmbsquote *)msgb;
//  MDFOLD  *folder;
//  MDMSTR  *m;
//  MDBOOK  *d, x;
//  MDQUOT  *q;
//  double  a,b;
//  uint32_t vbid, vask;
//  uint32_t nbid, nask;
//  char  symb[SYMB_LEN+1];
//
//  int  ii, jj, nn;
//  double  dval;
//  int    ival;
//  memset(&symb, 0x00, sizeof(symb));
//  str2s(symb, SYMB_LEN, quote->symb, sizeof(quote->symb));
//
//  if ((folder = mds_getfolder(market, symb)) == NULL)
//    return(NULL);
//
//
//  m = &folder->mstr;
//  d = &folder->book;
//  q = &folder->quot;
//  if (d->symb[0] == 0x00 || d->symb[0] == 0x20)
//    strcpy(d->symb, m->symb);
//
//
//  BOOKSTRUCT    spotdata;
//  BOOKSTRUCT    foworddata[MAX_TENNER];
////  BOOKSTRUCT    swapdata[MAX_TENNER];
//  
//  
//  memcpy(&x, d, sizeof(MDBOOK));
//
//  x.xymd = str2i(quote->date, sizeof(quote->date));
//  x.xhms = str2i(quote->time, sizeof(quote->time));
//  x.kymd = str2i(quote->date, sizeof(quote->date));
//  x.khms = str2i(quote->time, sizeof(quote->time));
//  memset(&spotdata, 0x00, sizeof(BOOKSTRUCT));
//  memset(&foworddata, 0x00, sizeof(BOOKSTRUCT)*MAX_TENNER);
////  memset(&swapdata, 0x00, sizeof(BOOKSTRUCT)*MAX_TENNER);
//  b = q->spotdata.bidlast;
//  a = q->spotdata.offerlast;
//
//  double dpinc = m->pinc*10;
//  double nOfferAmount = atof(quote->offerQty);
//  double nBidAmount = atof(quote->bidQty);
//
//    
//  for (ii = 0; ii < BOOK_LEVEL; ii++)
//  {
//    x.spotdata.ask[ii].pask = a + (dpinc* ii );
//    x.spotdata.ask[ii].vask = nOfferAmount /(ii +1);
//    x.spotdata.bid[ii].pbid = b - (dpinc* ii);
//    x.spotdata.bid[ii].vbid = nBidAmount/(ii +1);
//  }
//  for (jj = 0; jj < MAX_TENNER-1; jj++)
//  {
//    b = q->foworddata[jj].bidlast;
//    a = q->foworddata[jj].offerlast;
//    for (ii = 0; ii < BOOK_LEVEL; ii++)
//    {
//      x.foworddata[jj].ask[ii].pask = a + (dpinc* ii);
//      x.foworddata[jj].ask[ii].vask = nOfferAmount /(ii +1);
//      x.foworddata[jj].bid[ii].pbid = b - (dpinc * ii);
//      x.foworddata[jj].bid[ii].vbid = nBidAmount /(ii +1);
//    }
//  }
//  
//  nbid = 0; nask = 0;
//  vbid = 0; vask = 0;
//
// 
//  memcpy(d, &x, sizeof(MDBOOK));
//  check[BOOK] = X_PUSH;
//  return(folder);
//}
//
////
//// futbook()
//// Receive market book data of Future format
////
//
//MDFOLD *getKRWbook(MARKET *market, char *msgb, int msgl, char *check)
//{
//
////  struct  books *books = (struct books *)msgb;
//  struct  cmbsquote *quote = (struct cmbsquote *)msgb;
//  MDFOLD  *folder;
//  MDMSTR  *m;
//  MDBOOK  *d, x;
//  MDQUOT  *q;
//  double  a,b;
//  uint32_t vbid, vask;
//  uint32_t nbid, nask;
//  char  symb[SYMB_LEN+1];
//
//  int  ii, jj, nn;
//  double  dval;
//  int    ival;
//  char  symbol[SYMB_LEN];
//  memset(&symb, 0x00, sizeof(symb));
//
//  //ccy2가 KRW가 아닐경우 KRW환율 생성 
//  if (memcmp(quote->ccy2, "KRW", 3) != 0)
//  {  
//    if (memcmp(quote->ccy1, "USD", 3) == 0 && memcmp(quote->ccy2, "KRW", 3) != 0 )
//    {
//      sprintf(symbol, "%.3sKRW",quote->ccy2);
//
//    }
//    else if (memcmp(quote->ccy2, "USD", 3) == 0)
//    { 
//      sprintf(symbol, "%.3sKRW",quote->ccy1);
//    }
//  }else
//    return(NULL);
//
//  sprintf(symb, "%.*s", SYMB_LEN, symbol);
//  if ((folder = mds_getfolder(market, symb)) == NULL)
//    return(NULL);
//
//  m = &folder->mstr;
//  d = &folder->book;
//  q = &folder->quot;
//  if (d->symb[0] == 0x00 || d->symb[0] == 0x20)
//    strcpy(d->symb, m->symb);
//
//
//
//  BOOKSTRUCT    spotdata;
//  BOOKSTRUCT    foworddata[MAX_TENNER];
//  BOOKSTRUCT    swapdata[MAX_TENNER];
//  
//  
//  memcpy(&x, d, sizeof(MDBOOK));
//
//  x.xymd = str2i(quote->date, sizeof(quote->date));
//  x.xhms = str2i(quote->time, sizeof(quote->time));
//  x.kymd = str2i(quote->date, sizeof(quote->date));
//  x.khms = str2i(quote->time, sizeof(quote->time));
//  memset(&spotdata, 0x00, sizeof(BOOKSTRUCT));
//  memset(&foworddata, 0x00, sizeof(BOOKSTRUCT)*MAX_TENNER);
//  memset(&swapdata, 0x00, sizeof(BOOKSTRUCT)*MAX_TENNER);
//  b = q->spotdata.bidlast;
//  a = q->spotdata.offerlast;
//  double dpinc = m->pinc*10;
//  double nOfferAmount = atof(quote->offerQty);
//  double nBidAmount = atof(quote->bidQty);
//  for (ii = 0; ii < BOOK_LEVEL; ii++)
//  {
//    x.spotdata.ask[ii].pask = a + (dpinc* ii);
//    x.spotdata.ask[ii].vask = nOfferAmount /(ii +1);
//    x.spotdata.bid[ii].pbid = b - (dpinc* ii);
//    x.spotdata.bid[ii].vbid = nBidAmount/(ii +1);
//  }
//  for (jj = 0; jj < MAX_TENNER-1; jj++)
//  {
//    b = q->foworddata[jj].bidlast;
//    a = q->foworddata[jj].offerlast;
//    for (ii = 0; ii < BOOK_LEVEL; ii++)
//    {
//      x.foworddata[jj].ask[ii].pask = a + (dpinc* ii);
//      x.foworddata[jj].ask[ii].vask = nOfferAmount /(ii +1);
//      x.foworddata[jj].bid[ii].pbid = b - (dpinc * ii);
//      x.foworddata[jj].bid[ii].vbid = nBidAmount /(ii +1);
//    }
//  }
//
//  nbid = 0; nask = 0;
//  vbid = 0; vask = 0;
//
//
//  memcpy(d, &x, sizeof(MDBOOK));
//  check[BOOK] = X_PUSH;
//  return(folder);
//}

//MDFOLD *getcalcKRWbook(MARKET *market, char *isymb, char *check) 
//{
//  MDFOLD  *folder;
//  MDMSTR  *m;
//  MDBOOK  *d, x;
//  MDQUOT  *q;
//  double  a,b;
//  uint32_t vbid, vask;
//  uint32_t nbid, nask;
//  char  symb[SYMB_LEN+1];
//
//  int  ii, jj, nn;
//  double  dval;
//  int    ival;
//  char  symbol[SYMB_LEN];
//
//
//
//  char ccy1[3];
//  char ccy2[3];
//  sprintf(ccy1, "%.3s", isymb);
//  sprintf(ccy2,"%.3s", &isymb[3]);
//  
//  
//  //ccy2가 KRW가 아닐경우 KRW환율 생성 
//  if (memcmp(ccy2, "KRW", 3) != 0)
//  {  
//    if (memcmp(ccy1, "USD", 3) == 0 && memcmp(ccy2, "KRW", 3) != 0 )
//    {
//      sprintf(symbol, "%.3sKRW",ccy2);
//
//    }
//    else if (memcmp(ccy2, "USD", 3) == 0)
//    { 
//      sprintf(symbol, "%.3sKRW",ccy1);
//    }
//  }else
//    return(NULL);
//
//  sprintf(symb, "%.*s", SYMB_LEN, symbol);
//  if ((folder = mds_getfolder(market, symb)) == NULL)
//    return(NULL);
//
//  m = &folder->mstr;
//  d = &folder->book;
//  q = &folder->quot;
//  if (d->symb[0] == 0x00 || d->symb[0] == 0x20)
//    strcpy(d->symb, m->symb);
//
//
//
//  BOOKSTRUCT    spotdata;
//  BOOKSTRUCT    foworddata[MAX_TENNER];
//  BOOKSTRUCT    swapdata[MAX_TENNER];
//  
//  
//  memcpy(&x, d, sizeof(MDBOOK));
//
//  x.xymd = q->xymd;
//  x.xhms = q->xhms;
//  x.kymd = q->kymd;
//  x.khms = q->khms;
//  memset(&spotdata, 0x00, sizeof(BOOKSTRUCT));
//  memset(&foworddata, 0x00, sizeof(BOOKSTRUCT)*MAX_TENNER);
//  memset(&swapdata, 0x00, sizeof(BOOKSTRUCT)*MAX_TENNER);
//  b = q->spotdata.bidlast;
//  a = q->spotdata.offerlast;
//  double dpinc = m->pinc*10;
//  double nOfferAmount = q->spotdata.offervol;
//  double nBidAmount = q->spotdata.bidvol;
//  for (ii = 0; ii < BOOK_LEVEL; ii++)
//  {
//    x.spotdata.ask[ii].pask = a + (dpinc* ii);
//    x.spotdata.ask[ii].vask = nOfferAmount /(ii +1);
//    x.spotdata.bid[ii].pbid = b - (dpinc* ii);
//    x.spotdata.bid[ii].vbid = nBidAmount/(ii +1);
//  }
//  for (jj = 0; jj < MAX_TENNER-1; jj++)
//  {
//    b = q->foworddata[jj].bidlast;
//    a = q->foworddata[jj].offerlast;
//    for (ii = 0; ii < BOOK_LEVEL; ii++)
//    {
//      x.foworddata[jj].ask[ii].pask = a + (dpinc* ii);
//      x.foworddata[jj].ask[ii].vask = nOfferAmount /(ii +1);
//      x.foworddata[jj].bid[ii].pbid = b - (dpinc * ii);
//      x.foworddata[jj].bid[ii].vbid = nBidAmount /(ii +1);
//    }
//  }
//
//  nbid = 0; nask = 0;
//  vbid = 0; vask = 0;
//
//
//  memcpy(d, &x, sizeof(MDBOOK));
//  check[BOOK] = X_PUSH;
//  return(folder);
//}




extern  int cmetymd(MARKET *market, MDFOLD *folder, uint32_t tymd, char *check);




struct  q_data {
  char        symb[SYMB_LEN];
  uint32_t    tymd;   
  uint32_t    xymd;   
  uint32_t    xhms;   
  uint32_t    kymd;   
  uint32_t    khms;  
  uint32_t    SMBSspotdate;  //SMBS수신 SPOT일자 
  int         bidmarkup;    //가상잔량 마크업 틱
  int          askmarkup;    //가상잔량 마크업 틱  
  double    usdkrwbidpirce;  //반영당시의 USDKRW가격
  double    usdkreaskprice;  //반영당시의 USDKRW가격
  struct    q_price    spotdata;
  struct    q_price    foworddata[MAX_TENNER];
  struct    q_price    swapdata[MAX_TENNER];
  struct    q_Markup  markupdata[MAX_TENNER];
};



/******************************************************************************
* Function Name : void insertPricedataTofolder(struct q_price* q,  MDQUOT *mdq, struct q_price *qd, int mux, int nType )
* Description   : 입력된 가격정보를 메모리에 반영한다. 
******************************************************************************/
void insertPricedataTofolder(struct q_price* q, MDQUOT *mdq, struct q_price *qd, int mux, int nType)
{
	int jj = 0;
	double diff, rate;
	if (q->offeropen == 0)
	{
		q->offerotim = mdq->xhms;
		q->offeropen = qd->offerlast * mux;
	}
	if (q->offerhigh < qd->offerlast)
	{
		q->offerhtim = mdq->xhms;
		q->offerhigh = qd->offerlast * mux;
	}

	if ((q->offerlow > qd->offerlast) || (q->offerlow == 0))
	{
		q->offerltim = mdq->xhms;
		q->offerlow = qd->offerlast  * mux;
	}

	q->offerlast = qd->offerlast * mux;
	q->offerbest = qd->offerbest * mux;
	get_diff_rate(q->offerlast, q->offerbase, &diff, &rate);
	q->offerdiff = diff;
	q->offerrate = rate;
	/* 전일종가 대비 */
	/* asis udtp -> dirf */
	if (q->offerbase < qd->offerlast)
	{
		q->offerdirf = '+';
		q->offersign = _UP_;
	}
	else
		if (q->offerbase > qd->offerlast)
		{
			q->offerdirf = '-';
			q->offersign = _DN_;
		}
		else
		{
			q->offerdirf = '=';
			q->offersign = _NC_;
		}


	if (q->bidopen == 0)
	{
		q->bidotim = mdq->xhms;
		q->bidopen = qd->bidlast * mux;
	}
	if (q->bidhigh < qd->bidlast)
	{
		q->bidhtim = mdq->xhms;
		q->bidhigh = qd->bidlast * mux;
	}
	if ((q->bidlow > qd->bidlast) || (q->bidlow == 0))
	{
		q->bidltim = mdq->xhms;
		q->bidlow = qd->bidlast  * mux;
	}

	q->bidlast = qd->bidlast * mux;
	q->bidbest = qd->bidbest * mux;
	get_diff_rate(q->bidlast, q->bidbase, &diff, &rate);
	q->biddiff = diff;
	q->bidrate = rate;


	/* 직전 대비 */
	if (q->bidbase < qd->bidlast)
	{
		q->biddirf = '+';
		q->bidsign = _UP_;
	}
	else
		if (q->bidbase > qd->bidlast)
		{
			q->biddirf = '-';
			q->bidsign = _DN_;
		}
		else
		{
			q->biddirf = '=';
			q->bidsign = _NC_;
		}

	if (q->midopen == 0)
	{
		q->midotim = mdq->xhms;
		q->midopen = qd->midlast;
	}
	if (q->midhigh < qd->midlast)
	{
		q->midhtim = mdq->xhms;
		q->midhigh = qd->midlast;
	}
	if ((q->midlow > qd->midlast) || (q->midlow == 0))
	{
		q->midltim = mdq->xhms;
		q->midlow = qd->midlast;
	}


	q->midlast = qd->midlast;
	q->midbest = qd->midbest;
	get_diff_rate(q->midlast, q->midbase, &diff, &rate);
	q->middiff = diff;
	q->midrate = rate;


	if (q->midbase - qd->midlast < -0.0000001)
	{
		q->middirf = '+';
		q->midsign = _UP_;
	}
	else
		if (q->midbase - qd->midlast > 0.0000001)
		{
			q->middirf = '-';
			q->midsign = _DN_;
		}
		else
		{
			q->middirf = '=';
			q->midsign = _NC_;
		}


	q->bidbase = q->bidlast;
	q->offerbase = q->offerlast;
	//재정은 스왑 마크업이 실시간 변동된다. 
	if (nType == 1)
	{
		q->donedaybidswap = qd->donedaybidswap;
		q->donedayofferswap = qd->donedayofferswap;
		for (jj = 0; jj < MARKUPGROUPCNT; jj++)
		{
			q->onedayMarkup[jj].donedaybidMarkup = qd->onedayMarkup[jj].donedaybidMarkup;
			q->onedayMarkup[jj].donedayofferMarkup = qd->onedayMarkup[jj].donedayofferMarkup;
			//  q->onedayMarkup[1].donedaybidMarkup    = qd->onedayMarkup[1].donedaybidMarkup    ;
			//  q->onedayMarkup[1].donedayofferMarkup  = qd->onedayMarkup[1].donedayofferMarkup ;
			//  q->onedayMarkup[2].donedaybidMarkup    = qd->onedayMarkup[2].donedaybidMarkup    ;
			//  q->onedayMarkup[2].donedayofferMarkup  = qd->onedayMarkup[2].donedayofferMarkup ;
			//  q->onedayMarkup[3].donedaybidMarkup    = qd->onedayMarkup[3].donedaybidMarkup    ;
			//  q->onedayMarkup[3].donedayofferMarkup  = qd->onedayMarkup[3].donedayofferMarkup ;
		}
		//    q->onedayMarkup[MARKUPGROUPCNT].donedaybidMarkup    = 0;
		//    q->onedayMarkup[MARKUPGROUPCNT].donedayofferMarkup  = 0; 
	}
}




/******************************************************************************
* Function Name : void insertMarkupTofolder(struct q_Markup* q, struct q_Markup *qd)
* Description   : 입력된 마크업정보를 메모리에 반영한다. 
******************************************************************************/
void insertMarkupTofolder(struct q_Markup* q, struct q_Markup *qd)
{
  int ii ;
  for (ii = 0 ;ii < MARKUPGROUPCNT ; ii++)
  {
    q->markupSet[ii].bidMarkup       =  qd->markupSet[ii].bidMarkup;
    q->markupSet[ii].bidCMBSMarkup   =  qd->markupSet[ii].bidCMBSMarkup;
    q->markupSet[ii].offerMarkup     =  qd->markupSet[ii].offerMarkup;
    q->markupSet[ii].offerCMBSMarkup =  qd->markupSet[ii].offerCMBSMarkup;
    q->markupSet[ii].bidMarkup       =  qd->markupSet[ii].bidMarkup;
    q->markupSet[ii].bidCMBSMarkup   =  qd->markupSet[ii].bidCMBSMarkup;
    q->markupSet[ii].offerMarkup     =  qd->markupSet[ii].offerMarkup;
    q->markupSet[ii].offerCMBSMarkup =  qd->markupSet[ii].offerCMBSMarkup;
    q->markupSet[ii].bidMarkup       =  qd->markupSet[ii].bidMarkup;
    q->markupSet[ii].bidCMBSMarkup   =  qd->markupSet[ii].bidCMBSMarkup;
    q->markupSet[ii].offerMarkup     =  qd->markupSet[ii].offerMarkup;
    q->markupSet[ii].offerCMBSMarkup =  qd->markupSet[ii].offerCMBSMarkup;
    q->markupSet[ii].bidMarkup       =  qd->markupSet[ii].bidMarkup;
    q->markupSet[ii].bidCMBSMarkup   =  qd->markupSet[ii].bidCMBSMarkup;
    q->markupSet[ii].offerMarkup     =  qd->markupSet[ii].offerMarkup;
    q->markupSet[ii].offerCMBSMarkup =  qd->markupSet[ii].offerCMBSMarkup ;
    q->markupSet[ii].bidMarkup       =  qd->markupSet[ii].bidMarkup;
    q->markupSet[ii].bidCMBSMarkup   =  qd->markupSet[ii].bidCMBSMarkup;
    q->markupSet[ii].offerMarkup     =  qd->markupSet[ii].offerMarkup;
    q->markupSet[ii].offerCMBSMarkup =  qd->markupSet[ii].offerCMBSMarkup ;
    q->markupSet[ii].bidMarkup       =  qd->markupSet[ii].bidMarkup;
    q->markupSet[ii].bidCMBSMarkup   =  qd->markupSet[ii].bidCMBSMarkup;
    q->markupSet[ii].offerMarkup     =  qd->markupSet[ii].offerMarkup;
    q->markupSet[ii].offerCMBSMarkup =  qd->markupSet[ii].offerCMBSMarkup ;
    q->markupSet[ii].bidMarkup       =  qd->markupSet[ii].bidMarkup;
    q->markupSet[ii].bidCMBSMarkup   =  qd->markupSet[ii].bidCMBSMarkup;
    q->markupSet[ii].offerMarkup     =  qd->markupSet[ii].offerMarkup;
    q->markupSet[ii].offerCMBSMarkup =  qd->markupSet[ii].offerCMBSMarkup ;
    q->markupSet[ii].bidMarkup       =  qd->markupSet[ii].bidMarkup;
    q->markupSet[ii].bidCMBSMarkup   =  qd->markupSet[ii].bidCMBSMarkup;
    q->markupSet[ii].offerMarkup     =  qd->markupSet[ii].offerMarkup;
    q->markupSet[ii].offerCMBSMarkup =  qd->markupSet[ii].offerCMBSMarkup ;
  }
}

/******************************************************************************
* Function Name : int  quote_proc(MARKET *market, MDFOLD *folder, struct q_data *qd, char *check, int nType)
* Description   : 시세가공처리 원가에서 각 테너별 스왑을 반영하여  테너별 가격생성 가상잔량 생성 처리를 한다. 
******************************************************************************/
int  quote_proc(MARKET *market, MDFOLD *folder, struct q_data *qd, char *check, int nType)
{
  uint32_t  tymd;
  int      toff;
  uint32_t  curr_cvol, curr_tvol;
  double    prev;
  MDQUOT    *mdq;
  MDMSTR    *m;
  MDBOOK    *mdbook, x;
  double diff, rate;
  struct q_price    *q;
  struct q_price    *qFoward;
  struct q_Markup    *qMarkup;
  struct q_price    *qSwap;
  int   ii =0;
  int   jj =0;
  if (qd->tymd == 0)
    mds_time(market, 0, &tymd, NULL, NULL, NULL);
  else
    tymd = qd->xymd;
  toff = mds_date2julian(market->xymd) - mds_date2julian(tymd);
  if (toff >= 3)
  {
    mds_log(market, LOG_MUST, "%4d.%s> symb=%s mds_date2julian : toff=%d tymd=%d xymd=%d", 
      __LINE__, __func__, qd->symb, toff, tymd, market->xymd);
    return(0);
  }

  mdq = &folder->quot;
  q = &mdq->spotdata; 
  m = &folder->mstr;
  mdbook = &folder->book;
  m->pmul = 1;

  prev = q->bidlast;
  if (mdq->tymd == 0)
    mdq->tymd = tymd;
//  if (SECOND(mdq->xhms) != SECOND(mdq->xhms))
//    mdq->sseq = 1;
//  else 
//    mdq->sseq++;
  
  mdq->seqn++;
  mdq->xymd = qd->xymd;
  mdq->xhms = qd->xhms;
  mdq->kymd = qd->kymd;
  mdq->khms = qd->khms;
  mdq->SMBSspotdate = qd->SMBSspotdate;


  BOOKSTRUCT    spotdata;
  BOOKSTRUCT    foworddata[MAX_TENNER];
  double        dTempBidlast,dTempOfferlast;
  memcpy(&x, mdbook, sizeof(MDBOOK));
  
  x.xymd = qd->xymd;
  x.xhms = qd->xhms;
  x.kymd = qd->kymd;
  x.khms = qd->khms;
  memset(&spotdata, 0x00, sizeof(BOOKSTRUCT));
  memset(&foworddata, 0x00, sizeof(BOOKSTRUCT)*MAX_TENNER);  
  dTempBidlast = qd->spotdata.bidlast;
  dTempOfferlast = qd->spotdata.offerlast;
  
  double dpinc = m->pinc*10;  //호가 가격단위 (마크업 단위)
  
  // 거래가능량에 지정한 비율을 곱한다. 
  double nOfferAmount;
  if (m->iVirtualAskAmoutType== 2) //비율적용
  {
    nOfferAmount = qd->spotdata.offervol * m->dAskAMountRate;
    //mds_log(market, LOG_MUST, "[%-15s][%4d] qd->spotdata.offervol [%f] [%s] [%f] ", __FUNCTION__, __LINE__, qd->spotdata.offervol, qd->symb, nOfferAmount);    
    // 가상잔량 Offer 설정 수치 적용   
    if (nOfferAmount == 0)
      nOfferAmount =    m->dAskMinAmount;
    else if (nOfferAmount > m->dAskMaxAmount )
      nOfferAmount = m->dAskMaxAmount;  
    else if  (nOfferAmount < m->dAskMinAmount )
      nOfferAmount = m->dAskMinAmount;
  }
  else
    nOfferAmount = m->dAskMinAmount;
    
  double nBidAmount;
  if (m->iVirtualBidAmoutType== 2) //비율적용
  {
    nBidAmount   = qd->spotdata.bidvol   * m->dBidAMountRate;
    // 가상잔량 bid 설정 수치 적용   
    if (nBidAmount == 0)
      nBidAmount =    m->dBidMinAmount;
    else if (nBidAmount > m->dBidMaxAmount )
      nBidAmount = m->dBidMaxAmount;  
    else if  (nBidAmount < m->dBidMinAmount )
      nBidAmount = m->dBidMinAmount;  
  }
  else
    nBidAmount = m->dBidMinAmount;

  m->dVirtualBidAmout[0]   = nBidAmount;
  m->dVirtualOfferAmout[0] = nOfferAmount;    
    
    
  // 1~10 호가의 가격 잔량을 설정 해준다. 가상잔량 설정값을 반영해서 생성한다. 
  for (ii = 0; ii < BOOK_LEVEL; ii++)
  {
    x.spotdata.ask[ii].pask = dTempOfferlast + (dpinc* ii);
    x.spotdata.bid[ii].pbid = dTempBidlast - (dpinc* ii);
    if  (m->iVirtualAmoutType == 2)//가상잔량 산출기준 2:비율 , 1: 고정 
    {  
      if (ii == 0)
      {
        x.spotdata.ask[ii].vask = nOfferAmount ;
        x.spotdata.bid[ii].vbid = nBidAmount;
      }else
      {
        x.spotdata.ask[ii].vask = nOfferAmount * m->dVirtualOfferAmout[ii] / 100;
        x.spotdata.bid[ii].vbid = nBidAmount   * m->dVirtualBidAmout[ii] / 100;
      }
    }
    else 
    {
      x.spotdata.ask[ii].vask = m->dVirtualOfferAmout[ii];
      x.spotdata.bid[ii].vbid = m->dVirtualBidAmout[ii];
    }    
  }
  

  q->bidvol       = qd->spotdata.bidvol;
  q->offervol      = qd->spotdata.offervol;
  q->bidbestvol   = qd->spotdata.bidbestvol;
  q->offerbestvol  = qd->spotdata.offerbestvol;

  q->bidvirvol    =  0;
  q->offervirvol  = 0;
  if (q->offeropen == 0)
  { 
    q->offerotim = mdq->xhms;
    q->offeropen = qd->spotdata.offerlast * m->pmul;
  }
  if (q->offerhigh < qd->spotdata.offerlast)
  {
    q->offerhtim = mdq->xhms;
    q->offerhigh = qd->spotdata.offerlast * m->pmul;
  }

  if ((q->offerlow  > qd->spotdata.offerlast ) || (q->offerlow == 0))
  {
    q->offerltim = mdq->xhms;
    q->offerlow  = qd->spotdata.offerlast  * m->pmul;
  }

  q->offerlast = qd->spotdata.offerlast * m->pmul;
  q->offerbest = qd->spotdata.offerbest * m->pmul;
  get_diff_rate(q->offerlast, q->offerbase, &diff, &rate);
  q->offerdiff = diff;
  q->offerrate = rate;
  
  /* 전일종가 대비 */

  if (q->offerbase < qd->spotdata.offerlast)
  {
    q->offerdirf = '+';
    q->offersign = _UP_;
  }
  else
  if (q->offerbase > qd->spotdata.offerlast)
  {
    q->offerdirf = '-';
    q->offersign = _DN_;  
  }
  else
  {
    q->offerdirf = '=';
    q->offersign = _NC_;  
  }

  if (q->bidopen == 0)
  { 
    q->bidotim = mdq->xhms;
    q->bidopen = qd->spotdata.bidlast * m->pmul;
  }
  if (q->bidhigh < qd->spotdata.bidlast)
  {
    q->bidhtim = mdq->xhms;
    q->bidhigh = qd->spotdata.bidlast * m->pmul;
  }
  if ((q->bidlow  > qd->spotdata.bidlast ) || (q->bidlow == 0))
  {
    q->bidltim = mdq->xhms;
    q->bidlow  = qd->spotdata.bidlast  * m->pmul;
  }

  q->bidlast = qd->spotdata.bidlast * m->pmul;
  q->bidbest = qd->spotdata.bidbest * m->pmul;
  get_diff_rate(q->bidlast, q->bidbase, &diff, &rate);
  q->biddiff = diff;
  q->bidrate = rate;

  /* 전일종가 대비 */
  if (q->bidbase - qd->spotdata.bidlast < -0.0000001)
  {
    q->biddirf = '+';
    q->bidsign = _UP_;
  }
  else
  if (q->bidbase - qd->spotdata.bidlast > 0.0000001)
  {
    q->biddirf = '-';
    q->bidsign = _DN_;  
  }
  else
  {
    q->biddirf = '=';
    q->bidsign = _NC_;  
  }
  
  if (q->midopen == 0)
  { 
    q->midotim = mdq->xhms;
    q->midopen = qd->spotdata.midlast * m->pmul;
  }
  if (q->midhigh < qd->spotdata.midlast)
  {
    q->midhtim = mdq->xhms;
    q->midhigh = qd->spotdata.midlast * m->pmul;
  }
  if ((q->midlow  > qd->spotdata.midlast ) || (q->midlow == 0))
  {
    q->midltim = mdq->xhms;
    q->midlow  = qd->spotdata.midlast  * m->pmul;
  }

  q->midlast = qd->spotdata.midlast * m->pmul;
  q->midbest = qd->spotdata.midbest * m->pmul;
  get_diff_rate(q->midlast, q->midbase, &diff, &rate);
  q->middiff = diff;
  q->midrate = rate;
  
  /* 전일종가 대비 */
  if (q->midbase - qd->spotdata.midlast < -0.0000001)
  {
    q->middirf = '+';
    q->midsign = _UP_;
  }
  else
  if (q->midbase - qd->spotdata.midlast > 0.0000001)
  {
    q->middirf = '-';
    q->midsign = _DN_;  
  }
  else
  {
    q->middirf = '=';
    q->midsign = _NC_;  
  }
  
  q->bidbase    = q->bidlast;
  q->offerbase  = q->offerlast;

  if (q->bidlast != prev)
    check[QUOT] |= X_NOTI;

  check[QUOT] |= (X_PUSH | X_SYNC);
  if (mdq->tymd != folder->mstr.tymd)
  {
    folder->mstr.pymd = folder->mstr.tymd;
    folder->mstr.tymd = mdq->tymd;
    check[MSTR] = X_PUSH;
  }
  //folder->mstr.stat = q->stat;
  //MAR를위한 입력
  mdq->usdkrwbidpirce = qd->usdkrwbidpirce;
  mdq->usdkreaskprice = qd->usdkreaskprice;
  x.usdkrwbidpirce  =qd->usdkrwbidpirce;
  x.usdkrwbidpirce  =qd->usdkreaskprice;
  for (ii =0; ii < MAX_TENNER  ; ii++)
  {  
    if (nType == 0)
    {
      if ( ii == MAX_TENNER -3)  //TOD시세 = 가격 
      {
        
        if (dTempBidlast != 0)  
        {
          
          qd->foworddata[ii].bidlast = dTempBidlast - (mdq->swapdata[ii].bidlast * m->swappmul);    
          qd->foworddata[ii].bidbest = q->bidbest - (mdq->swapdata[ii].bidbest * m->swappmul);
          qd->foworddata[ii].bidvol  = q->bidvol;//  + (mdq->swapdata[ii].bidvol * m->swappmul);
          qd->foworddata[ii].bidbestvol  = q->bidbestvol;
          
        }
        
        if ( dTempOfferlast != 0)
        {
          
          qd->foworddata[ii].offerlast = dTempOfferlast - (mdq->swapdata[ii].offerlast * m->swappmul);
          qd->foworddata[ii].offerbest = q->offerbest - (mdq->swapdata[ii].offerbest * m->swappmul);
          qd->foworddata[ii].offervol =  q->offervol;//  + (mdq->swapdata[ii].offervol * m->swappmul);
          qd->foworddata[ii].offerbestvol =  q->offerbestvol;//  + (mdq->swapdata[ii].offervol * m->swappmul);
          
        }    
      } 
      else if ( ii == MAX_TENNER -2)  //TOM시세 
      {
        if (dTempBidlast != 0)  
        {
          qd->foworddata[ii].bidlast = dTempBidlast - (mdq->swapdata[ii].bidlast * m->swappmul);    
          qd->foworddata[ii].bidbest = q->bidbest - (mdq->swapdata[ii].bidbest * m->swappmul);
          qd->foworddata[ii].bidvol  = q->bidvol;//  + (mdq->swapdata[ii].bidvol * m->swappmul);
          qd->foworddata[ii].bidbestvol  = q->bidbestvol;
          
        }
        
        if ( dTempOfferlast != 0)
        {
          qd->foworddata[ii].offerlast = dTempOfferlast - (mdq->swapdata[ii].offerlast * m->swappmul);
          qd->foworddata[ii].offerbest = q->offerbest - (mdq->swapdata[ii].offerbest * m->swappmul);
          qd->foworddata[ii].offervol =  q->offervol;//  + (mdq->swapdata[ii].offervol * m->swappmul);
          qd->foworddata[ii].offerbestvol =  q->offerbestvol;//  + (mdq->swapdata[ii].offervol * m->swappmul);
          
        }    
      
      }else
      {  
        
        if (dTempBidlast != 0)  
        {
          qd->foworddata[ii].bidlast = dTempBidlast + (mdq->swapdata[ii].bidlast * m->swappmul);    
          qd->foworddata[ii].bidbest = q->bidbest + (mdq->swapdata[ii].bidbest * m->swappmul);
          qd->foworddata[ii].bidvol  = q->bidvol;//  + (mdq->swapdata[ii].bidvol * m->swappmul);
          qd->foworddata[ii].bidbestvol  = q->bidbestvol;
        }
        
        if ( dTempOfferlast != 0)
        {
          qd->foworddata[ii].offerlast = dTempOfferlast + (mdq->swapdata[ii].offerlast * m->swappmul);
          qd->foworddata[ii].offerbest = q->offerbest + (mdq->swapdata[ii].offerbest * m->swappmul);
          qd->foworddata[ii].offervol =  q->offervol;//  + (mdq->swapdata[ii].offervol * m->swappmul);
          qd->foworddata[ii].offerbestvol =  q->offerbestvol;//  + (mdq->swapdata[ii].offervol * m->swappmul);
        }
      }
    }

//    q = &mdq->foworddata[ii];
    qFoward= &mdq->foworddata[ii];
    //insertPricedataTofolder(qFoward,mdq,&qd->foworddata[ii], m->pmul,0);
    
    //재정은 Swap, Markup을 다시 입력 합니다. 
    if (nType == 1){    
      insertPricedataTofolder(qFoward,mdq,&qd->foworddata[ii], m->pmul,1);  
      qSwap= &mdq->swapdata[ii];
      insertPricedataTofolder(qSwap,mdq,&qd->swapdata[ii], m->pmul,1);
      qMarkup = &mdq->markupdata[ii];      
      insertMarkupTofolder(qMarkup, &qd->markupdata[ii]);
    }
    else
    {
        
      insertPricedataTofolder(qFoward,mdq,&qd->foworddata[ii], m->pmul,0);
    }
  }
  
  
  // 1~10 호가의 테너별 시장가격을 설정 해준다. 잔량은 Spot과 공유하기때문에 동일하게 처리 한다. 
  for (jj = 0; jj < MAX_TENNER-1; jj++)
  {
    dTempBidlast  = qd->foworddata[jj].bidlast;
    dTempOfferlast = qd->foworddata[jj].offerlast;      
    for (ii = 0; ii < BOOK_LEVEL; ii++)
    {
      x.foworddata[jj].ask[ii].pask = qd->foworddata[jj].offerlast + (dpinc* ii);
      x.foworddata[jj].bid[ii].pbid = qd->foworddata[jj].bidlast - (dpinc * ii);            
      if   (m->iVirtualAmoutType == 2)/*가상잔량 산출기준 0:비율 , 1: 고정 */
      {
        x.foworddata[jj].ask[ii].vask=  nOfferAmount  * m->dVirtualOfferAmout[ii] / 100;
        x.foworddata[jj].bid[ii].vbid = nBidAmount   * m->dVirtualBidAmout[ii] /100;
      }
      else 
      {
        x.foworddata[jj].ask[ii].vask = m->dVirtualOfferAmout[ii];
        x.foworddata[jj].bid[ii].vbid = m->dVirtualBidAmout[ii];
      }  
    }
  }    
  
  memcpy(mdbook, &x, sizeof(MDBOOK));
  check[BOOK] = X_PUSH;
  
  return(0);
}



//
// cmbsqout()
// Receive orgin of cmbs format
//
MDFOLD *getSpotquot(MARKET *market, char *msgb, int msgl, char *check) 
{
  MDFOLD  *folder; 
  MDQUOT  *q;
  struct cmbsquote  *quote = (struct cmbsquote *)msgb;
  struct  q_data  qd;
  double dTemp;
  memset(&qd, 0x00, sizeof(struct q_data));
  str2s(qd.symb, SYMB_LEN, quote->symb,  sizeof(quote->symb));

  /*-------------------------
   * data set
   * -----------------------*/
  
  if ((folder = mds_getfolder(market, qd.symb)) == NULL)
    return(NULL);
    


  qd.tymd = str2i(quote->date, sizeof(quote->date));
  qd.xymd = str2i(quote->date, sizeof(quote->date));
  qd.xhms = str2i(quote->time, sizeof(quote->time));
  qd.kymd = str2i(quote->date, sizeof(quote->date));
  qd.khms = str2i(quote->time, sizeof(quote->time));
  qd.SMBSspotdate = str2i(quote->offerdate, sizeof(quote->offerdate));
  if (qd.SMBSspotdate == 0)
  {
    qd.SMBSspotdate = str2i(quote->biddate, sizeof(quote->biddate));
  }
 // mds_log(market, LOG_MUST, "[%-15s][%4d] qd.spotdata.bidlast [%s] ", __FUNCTION__, __LINE__, qd.symb);    

  if (memcmp(qd.symb,"MARKRW", 6) == 0)
  {

    qd.spotdata.bidlast = atof(quote->bidprice);
    qd.spotdata.bidbest = atof(quote->bidBestPx);
    qd.spotdata.bidvol = str2d(quote->bidQty, sizeof(quote->bidQty));
    qd.spotdata.bidbestvol = str2d(quote->bidBestSize, sizeof(quote->bidBestSize));    

    qd.spotdata.offerlast = atof(quote->offerprice);//, FlotePoint); //sizeof(quote->offerprice));
    qd.spotdata.offerbest = atof(quote->offerBestPx);//, FlotePoint); //sizeof(quote->offerBestPx));
    qd.spotdata.offervol = str2d(quote->offerQty, sizeof(quote->offerQty));
    qd.spotdata.offerbestvol = str2d(quote->offerBestSize, sizeof(quote->offerBestSize));
     

    MDFOLD  *krwfolder;
    krwfolder = mds_getfolder(market, "USDKRW");
  
    if (krwfolder == NULL)
    {
      return -3;
    }
    
    MDQUOT  *krwquot;
    krwquot = &krwfolder->quot;
    
    
    
    qd.usdkrwbidpirce = krwquot->spotdata.bidlast;
    qd.usdkreaskprice = krwquot->spotdata.offerlast;
//    mds_log(market, LOG_MUST, "[%-15s][%4d] qd.spotdata.bidlast [%f ", __FUNCTION__, __LINE__, qd.usdkrwbidpirce);    
//    mds_log(market, LOG_MUST, "[%-15s][%4d] qd.spotdata.bidlast [%f ", __FUNCTION__, __LINE__, qd.usdkreaskprice);    
  
  }else
  {
  
    if (str2f(quote->bidprice, sizeof(quote->bidprice)) != 0)  
    {
      qd.spotdata.bidlast = atof(quote->bidprice);
      qd.spotdata.bidbest = atof(quote->bidBestPx);
      qd.spotdata.bidvol = str2d(quote->bidQty, sizeof(quote->bidQty));
      qd.spotdata.bidbestvol = str2d(quote->bidBestSize, sizeof(quote->bidBestSize));    
    }else
    {
      return (NULL);
    }
  
    if ( str2f(quote->offerprice, sizeof(quote->offerprice)) != 0)  
    {
      qd.spotdata.offerlast = atof(quote->offerprice);//, FlotePoint); //sizeof(quote->offerprice));
      qd.spotdata.offerbest = atof(quote->offerBestPx);//, FlotePoint); //sizeof(quote->offerBestPx));
      qd.spotdata.offervol = str2d(quote->offerQty, sizeof(quote->offerQty));
      qd.spotdata.offerbestvol = str2d(quote->offerBestSize, sizeof(quote->offerBestSize));
    }else
    {
      return (NULL);
    }
  }
  qd.spotdata.midlast = (atof(quote->bidprice) + atof(quote->offerprice))/2;//, FlotePoint); //sizeof(quote->offerprice));
  qd.spotdata.midbest = (atof(quote->bidBestPx) + atof(quote->offerBestPx))/2;//, FlotePoint); //sizeof(quote->offerBestPx));
  quote_proc(market, folder, &qd, check, 0);
   //USDKRW환율 수신시 가상잔량 계산  
  if (memcmp(qd.symb,"USDKRW", 6) == 0)
    CalcCAmount(folder);
  return(folder);
}




MDFOLD *getCalcKRWquot(MARKET *market, char *symb, char *check)
{
	MDFOLD  *folder, *rfolder, *basefolder;
	MDFOLD  *krwfolder;
	MDQUOT  *q, *krwq, *baseq;
	MDMSTR  *m, *basem;
	struct  q_data  qd;
	char  krwsmb[SYMB_LEN];
	int    ii, jj;
	/*----------------------/
	 *  Get USD/KRW data
	 *----------------------*/
	memset(&qd, 0x00, sizeof(struct q_data));
	str2s(qd.symb, SYMB_LEN, symb, SYMB_LEN);
	memset(&krwsmb, 0x00, sizeof(krwsmb));
	str2s(krwsmb, SYMB_LEN, "USDKRW", SYMB_LEN);
	double krwprice;
	double baseprice;
	if ((krwfolder = mds_getfolder(market, krwsmb)) == NULL)
		return(NULL);
	else
	{
		krwq = &krwfolder->quot;
	}

	//KRW환율이 없으면 계산하지 않음 
	if (krwq->spotdata.bidlast == 0)
		return(NULL);
	// 기준환율 데이터를 가지고옴
	if ((basefolder = mds_getfolder(market, qd.symb)) == NULL)
	{
		return(NULL);
	}
	else
	{
		baseq = &basefolder->quot;
		//기준환율 미수신 상태시 환율처리 안함
		if (basefolder->mstr.trdf == 0)
			return(NULL);
	}

	/*-------------------------
	 * data set
	 * -----------------------*/



	 //ccy2가 KRW가 아닐경우 KRW환율 생성 

	 /* 재정환율 데이터 생성 */
	int nMux = 1;
	char ccy1[3];
	char ccy2[3];
	sprintf(ccy1, "%.3s", qd.symb);
	sprintf(ccy2, "%.3s", &qd.symb[3]);

	if (memcmp(ccy2, "CHN", 3) == 0)
		return(NULL);
	if (memcmp(ccy1, "CHN", 3) == 0)
		return(NULL);

	if (memcmp(ccy2, "JPY", 3) == 0)
		nMux = 100;

	struct timeb itb;
	struct tm *lt;
	ftime(&itb);
	lt = localtime(&itb.time);
	char ctimec[10];
	memset(&ctimec, 0x00, sizeof(ctimec));
	sprintf(ctimec, "%02d%02d%02d%03d", lt->tm_hour, lt->tm_min, lt->tm_sec, itb.millitm);

	//둘중 큰시간으로 시간 설정 
	if (baseq->khms > krwq->khms)
	{
		qd.tymd = baseq->tymd;
		qd.xymd = baseq->xymd;
		qd.xhms = atoi(ctimec);//baseq->xhms;
		qd.kymd = baseq->kymd;
		qd.khms = atoi(ctimec);//baseq->khms;    
	}
	else
	{
		qd.tymd = krwq->tymd;
		qd.xymd = krwq->xymd;
		qd.xhms = atoi(ctimec);//krwq->xhms;
		qd.kymd = krwq->kymd;
		qd.khms = atoi(ctimec);//krwq->khms;          

	}

	//mds_log(market, LOG_MUST, "[%-15s][%4d] qd.spotdata.bidlast [ ", __FUNCTION__, __LINE__);    
	if (memcmp(ccy1, "USD", 3) == 0 && memcmp(ccy2, "KRW", 3) != 0)
	{
		sprintf(qd.symb, "%.3sKRW", ccy2);
		if ((rfolder = mds_getfolder(market, qd.symb)) == NULL)
			return(NULL);

		//시세 중지중인 종목 재정채크 추가
		if (rfolder->mstr.pricestat == SOURCE_STOP - 1) //시세정지
		{
			mds_log(market, LOG_MUST, "[%-15s][%4d] 시세원천이 중지 중입니다. [%s] ", __FUNCTION__, __LINE__, qd.symb);
			return (NULL);
		}

		if (baseq->spotdata.offerlast == 0) //기존환율도 0이면 pass
			return(NULL);
		//1: USD/KRW원가로 /* 하여 재정환율 시장가격 생성
		qd.spotdata.bidlast = krwq->spotdata.bidlast / baseq->spotdata.offerlast * nMux;
		qd.spotdata.bidbest = krwq->spotdata.bidbest / baseq->spotdata.offerbest * nMux;
		qd.spotdata.offerlast = krwq->spotdata.offerlast / baseq->spotdata.bidlast * nMux;
		qd.spotdata.offerbest = krwq->spotdata.offerbest / baseq->spotdata.bidbest * nMux;


		//재정 마크업스팟  = 원가 - (마크업반영 krw / 마크업 반영 USD환율 )		
		for (jj = 0; jj < MARKUPGROUPCNT; jj++)
		{
			qd.markupdata[INDEXSPOT].markupSet[jj].bidMarkup = (((krwq->spotdata.bidlast + krwq->markupdata[INDEXSPOT].markupSet[jj].bidMarkup   * krwfolder->mstr.swappmul) / ((baseq->spotdata.offerlast + baseq->markupdata[INDEXSPOT].markupSet[jj].offerMarkup   * basefolder->mstr.swappmul))* nMux) - qd.spotdata.bidlast) * 100;
			qd.markupdata[INDEXSPOT].markupSet[jj].offerMarkup = (((krwq->spotdata.offerlast + krwq->markupdata[INDEXSPOT].markupSet[jj].offerMarkup * krwfolder->mstr.swappmul) / ((baseq->spotdata.bidlast + baseq->markupdata[INDEXSPOT].markupSet[jj].bidMarkup     * basefolder->mstr.swappmul))* nMux) - qd.spotdata.offerlast) * 100;
		}

		//재정 시장 Foward생성 + 재정 Foward 마크업 생성 
		for (ii = 0; ii < MAX_TENNER - 1; ii++)
		{
			//1: USD/KRW원가로 /* 하여 재정환율 시장가격 생성
			qd.foworddata[ii].bidlast = krwq->foworddata[ii].bidlast / baseq->foworddata[ii].offerlast    * nMux;
			qd.foworddata[ii].offerlast = krwq->foworddata[ii].offerlast / baseq->foworddata[ii].bidlast * nMux;

			//재정 마크업  = 원가 - (마크업반영 krw / 마크업 반영 USD환율 ) 
			for (jj = 0; jj < MARKUPGROUPCNT; jj++)
			{
				qd.markupdata[ii].markupSet[jj].bidMarkup = ((krwq->foworddata[ii].bidlast + (krwq->markupdata[ii].markupSet[jj].bidMarkup + krwq->markupdata[INDEXSPOT].markupSet[jj].bidMarkup)* krwfolder->mstr.swappmul) / (baseq->foworddata[ii].offerlast + (baseq->markupdata[ii].markupSet[jj].offerMarkup + baseq->markupdata[INDEXSPOT].markupSet[jj].offerMarkup) * basefolder->mstr.swappmul)* nMux);
				qd.markupdata[ii].markupSet[jj].offerMarkup = ((krwq->foworddata[ii].offerlast + (krwq->markupdata[ii].markupSet[jj].offerMarkup + krwq->markupdata[INDEXSPOT].markupSet[jj].offerMarkup)* krwfolder->mstr.swappmul) / (baseq->foworddata[ii].bidlast + (baseq->markupdata[ii].markupSet[jj].bidMarkup + baseq->markupdata[INDEXSPOT].markupSet[jj].bidMarkup) * basefolder->mstr.swappmul)* nMux);
				qd.markupdata[ii].markupSet[jj].bidMarkup = (qd.markupdata[ii].markupSet[jj].bidMarkup - qd.foworddata[ii].bidlast - (qd.markupdata[INDEXSPOT].markupSet[jj].bidMarkup   *0.01)) * 100;
				qd.markupdata[ii].markupSet[jj].offerMarkup = (qd.markupdata[ii].markupSet[jj].offerMarkup - qd.foworddata[ii].offerlast - (qd.markupdata[INDEXSPOT].markupSet[jj].offerMarkup *0.01)) * 100;
			}

			//재정 스왑 = (재정 포워드 - 재정 spot)/스왑조정계수 / m->swappmul
			qd.swapdata[ii].bidlast = (qd.foworddata[ii].bidlast - qd.spotdata.bidlast) / rfolder->mstr.swappmul;
			qd.swapdata[ii].offerlast = (qd.foworddata[ii].offerlast - qd.spotdata.offerlast) / rfolder->mstr.swappmul;
			//  mds_log(market, LOG_MUST, "[%-15s][%4d] symb [%s] ", __FUNCTION__, __LINE__,qd.symb);  
			//  mds_log(market, LOG_MUST, "[%-15s][%4d] qd.foworddata[ii].bidlast[%f] qd.spotdata.bidlast[%f] ", __FUNCTION__, __LINE__,qd.foworddata[ii].bidlast, qd.spotdata.bidlast);
			//  mds_log(market, LOG_MUST, "[%-15s][%4d] qd.swapdata[%d].offerlast [%f] qd.spotdata.offerlast[%f] ", __FUNCTION__, __LINE__,ii,qd.swapdata[ii].offerlast, qd.spotdata.offerlast );
			//  mds_log(market, LOG_MUST, "[%-15s][%4d] qd.swapdata[%d].bidlast [%f]", __FUNCTION__, __LINE__,ii,qd.swapdata[ii].bidlast);
			//  mds_log(market, LOG_MUST, "[%-15s][%4d] qd.swapdata[%d].offerlast [%f]", __FUNCTION__, __LINE__,ii,qd.swapdata[ii].bidlast);
			//mds_log(market, LOG_MUST, "[%-15s][%4d] symb [%s] ", __FUNCTION__, __LINE__,qd.symb);  
			//mds_log(market, LOG_MUST, "qd.markupdata[ii].markupSet[MARKUPINDEXEMP].bidMarkup    = (qd.foworddata[ii].bidlast   - ((krwq->foworddata[ii].bidlast    + krwq->markupdata[ii].markupSet[MARKUPINDEXEMP].bidMarkup   * krwfolder->mstr.swappmul ) /(baseq->foworddata[ii].offerlast    + baseq->markupdata[ii].markupSet[MARKUPINDEXEMP].offerMarkup   * basefolder->mstr.swappmul)* nMux))*-100;");
			//mds_log(market, LOG_MUST, "qd.markupdata[ii].markupSet[MARKUPINDEXEMP].offerMarkup  = (qd.foworddata[ii].offerlast - ((krwq->foworddata[ii].offerlast  + krwq->markupdata[ii].markupSet[MARKUPINDEXEMP].offerMarkup * krwfolder->mstr.swappmul) /(baseq->foworddata[ii].bidlast  + baseq->markupdata[ii].markupSet[MARKUPINDEXEMP].bidMarkup  * basefolder->mstr.swappmul)* nMux))*-100");
			//
			//mds_log(market, LOG_MUST, "[%-15s][%4d] [%f] [%f][%f][%f][%f][%f][%f][%f][%d] ", __FUNCTION__, __LINE__,
			//qd.markupdata[ii].markupSet[MARKUPINDEXEMP].bidMarkup ,
			//qd.foworddata[ii].bidlast   ,
			//krwq->foworddata[ii].bidlast ,
			// krwq->markupdata[ii].markupSet[MARKUPINDEXEMP].bidMarkup ,
			// krwfolder->mstr.swappmul ,
			// baseq->foworddata[ii].offerlast   ,
			//  baseq->markupdata[ii].markupSet[MARKUPINDEXEMP].offerMarkup   ,
			//   basefolder->mstr.swappmul,
			//    nMux);  
			  //
			//mds_log(market, LOG_MUST, "[%-15s][%4d] [%f] [%f][%f][%f][%f][%f][%f][%f][%d] ", __FUNCTION__, __LINE__,
			//qd.markupdata[ii].markupSet[MARKUPINDEXEMP].offerMarkup ,
			//qd.foworddata[ii].offerlast,
			//krwq->foworddata[ii].offerlast,
			//krwq->markupdata[ii].markupSet[MARKUPINDEXEMP].offerMarkup , 
			//krwfolder->mstr.swappmul,
			//baseq->foworddata[ii].bidlast  ,
			// baseq->markupdata[ii].markupSet[MARKUPINDEXEMP].bidMarkup  ,
			// basefolder->mstr.swappmul,
			//  nMux);


		}



	}
	else if (memcmp(ccy2, "USD", 3) == 0)
	{
		sprintf(qd.symb, "%.3sKRW", ccy1);
		if ((rfolder = mds_getfolder(market, qd.symb)) == NULL)
			return(NULL);

		//시세 중지중인 종목 재정채크 추가
		if (rfolder->mstr.pricestat == SOURCE_STOP - 1) //시세정지
		{
			mds_log(market, LOG_MUST, "[%-15s][%4d] 시세원천이 중지 중입니다. [%s] ", __FUNCTION__, __LINE__, qd.symb);
			return (NULL);
		}
		if (baseq->spotdata.offerlast == 0) //기존환율도 0이면 pass
			return(NULL);
		//1: USD/KRW원가로 /* 하여 재정환율 시장가격 생성
		qd.spotdata.bidlast = krwq->spotdata.bidlast  * baseq->spotdata.bidlast;
		qd.spotdata.bidbest = krwq->spotdata.bidbest * baseq->spotdata.bidbest;
		qd.spotdata.offerlast = krwq->spotdata.offerlast *baseq->spotdata.offerlast;
		qd.spotdata.offerbest = krwq->spotdata.offerbest * baseq->spotdata.offerbest;

		//재정 마크업  = 원가 - (마크업반영 krw * 마크업 반영 USD환율 )  
		for (jj = 0; jj < MARKUPGROUPCNT; jj++)
		{
			qd.markupdata[INDEXSPOT].markupSet[jj].bidMarkup = (((krwq->spotdata.bidlast + krwq->markupdata[INDEXSPOT].markupSet[jj].bidMarkup   * krwfolder->mstr.swappmul)*((baseq->spotdata.bidlast + baseq->markupdata[INDEXSPOT].markupSet[jj].bidMarkup   * basefolder->mstr.swappmul))* nMux) - qd.spotdata.bidlast) * 100;
			qd.markupdata[INDEXSPOT].markupSet[jj].offerMarkup = (((krwq->spotdata.offerlast + krwq->markupdata[INDEXSPOT].markupSet[jj].offerMarkup * krwfolder->mstr.swappmul)*((baseq->spotdata.offerlast + baseq->markupdata[INDEXSPOT].markupSet[jj].offerMarkup * basefolder->mstr.swappmul))* nMux) - qd.spotdata.offerlast) * 100;
		}
		//mds_log(market, LOG_MUST, "[%-15s][%4d] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].bidMarkup  [%f] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].offerMarkup  [%f] ", __FUNCTION__, __LINE__, qd.markupdata[INDEXSPOT].markupSet[MARKUPINDEXEMP].offerMarkup ,qd.markupdata[INDEXSPOT].markupSet[MARKUPINDEXEMP].bidMarkup );      
		//mds_log(market, LOG_MUST, "[%-15s][%4d] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].bidMarkup  [%f] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].offerMarkup  [%f] ", __FUNCTION__, __LINE__, qd.markupdata[INDEXSPOT].markupSet[MARKUPINDEXPRI].offerMarkup ,qd.markupdata[INDEXSPOT].markupSet[MARKUPINDEXPRI].bidMarkup );    
		//mds_log(market, LOG_MUST, "[%-15s][%4d] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].bidMarkup  [%f] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].offerMarkup  [%f] ", __FUNCTION__, __LINE__, qd.markupdata[INDEXSPOT].markupSet[MARKUPINDEXENT].offerMarkup ,qd.markupdata[INDEXSPOT].markupSet[MARKUPINDEXENT].bidMarkup );    


		//재정 시장 Foward생성 + 재정 Foward 마크업 생성 
		for (ii = 0; ii < MAX_TENNER - 1; ii++)
		{
			//1: USD/KRW원가로 /* 하여 재정환율 시장가격 생성
			qd.foworddata[ii].bidlast = krwq->foworddata[ii].bidlast   * baseq->foworddata[ii].bidlast   * nMux;
			qd.foworddata[ii].offerlast = krwq->foworddata[ii].offerlast * baseq->foworddata[ii].offerlast * nMux;

			//재정 마크업  = 원가 - (마크업반영 krw / 마크업 반영 USD환율 )
			for (jj = 0; jj < MARKUPGROUPCNT; jj++)
			{
				qd.markupdata[ii].markupSet[jj].bidMarkup = ((krwq->foworddata[ii].bidlast + (krwq->markupdata[ii].markupSet[jj].bidMarkup + krwq->markupdata[INDEXSPOT].markupSet[jj].bidMarkup)* krwfolder->mstr.swappmul) *(baseq->foworddata[ii].bidlast + (baseq->markupdata[ii].markupSet[jj].bidMarkup + baseq->markupdata[INDEXSPOT].markupSet[jj].bidMarkup) * basefolder->mstr.swappmul)* nMux);
				qd.markupdata[ii].markupSet[jj].offerMarkup = ((krwq->foworddata[ii].offerlast + (krwq->markupdata[ii].markupSet[jj].offerMarkup + krwq->markupdata[INDEXSPOT].markupSet[jj].offerMarkup)* krwfolder->mstr.swappmul) *(baseq->foworddata[ii].offerlast + (baseq->markupdata[ii].markupSet[jj].offerMarkup + baseq->markupdata[INDEXSPOT].markupSet[jj].offerMarkup) * basefolder->mstr.swappmul)* nMux);
				qd.markupdata[ii].markupSet[jj].bidMarkup = (qd.markupdata[ii].markupSet[jj].bidMarkup - qd.foworddata[ii].bidlast - (qd.markupdata[INDEXSPOT].markupSet[jj].bidMarkup   *0.01)) * 100;
				qd.markupdata[ii].markupSet[jj].offerMarkup = (qd.markupdata[ii].markupSet[jj].offerMarkup - qd.foworddata[ii].offerlast - (qd.markupdata[INDEXSPOT].markupSet[jj].offerMarkup *0.01)) * 100;
			}

			//재정 스왑 = (재정 포워드 - 재정 spot)/스왑조정계수 / m->swappmul
			qd.swapdata[ii].bidlast = (qd.foworddata[ii].bidlast - qd.spotdata.bidlast) / rfolder->mstr.swappmul;
			qd.swapdata[ii].offerlast = (qd.foworddata[ii].offerlast - qd.spotdata.offerlast) / rfolder->mstr.swappmul;
			//  mds_log(market, LOG_MUST, "[%-15s][%4d] qd.swapdata[%d].bidlast [%f]", __FUNCTION__, __LINE__,ii,qd.swapdata[ii].bidlast);
			//  mds_log(market, LOG_MUST, "[%-15s][%4d] qd.swapdata[%d].offerlast [%f]", __FUNCTION__, __LINE__,ii,qd.swapdata[ii].bidlast);


		}

		//mds_log(market, LOG_MUST, "[%-15s][%4d] qd.foworddata[INDEX1M].donedayofferswap [%s] ", __FUNCTION__, __LINE__,qd.symb);
		//mds_log(market, LOG_MUST, "[%-15s][%4d] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].bidMarkup  [%f] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].offerMarkup  [%f] ", __FUNCTION__, __LINE__, krwq->markupdata[INDEX1M].markupSet[MARKUPINDEXEMP].bidMarkup , baseq->markupdata[INDEX1M].markupSet[MARKUPINDEXEMP].bidMarkup );      
		//mds_log(market, LOG_MUST, "[%-15s][%4d] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].bidMarkup  [%f] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].offerMarkup  [%f] ", __FUNCTION__, __LINE__, krwq->markupdata[INDEX1M].markupSet[MARKUPINDEXPRI].bidMarkup , baseq->markupdata[INDEX1M].markupSet[MARKUPINDEXPRI].bidMarkup );      
		//mds_log(market, LOG_MUST, "[%-15s][%4d] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].bidMarkup  [%f] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].offerMarkup  [%f] ", __FUNCTION__, __LINE__, krwq->markupdata[INDEX1M].markupSet[MARKUPINDEXENT].bidMarkup , baseq->markupdata[INDEX1M].markupSet[MARKUPINDEXENT].bidMarkup );      
			//
		//mds_log(market, LOG_MUST, "[%-15s][%4d] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].bidMarkup  [%f] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].offerMarkup  [%f] ", __FUNCTION__, __LINE__, qd.markupdata[INDEX1M].markupSet[MARKUPINDEXEMP].offerMarkup ,qd.markupdata[INDEX1M].markupSet[MARKUPINDEXEMP].bidMarkup );      
		//mds_log(market, LOG_MUST, "[%-15s][%4d] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].bidMarkup  [%f] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].offerMarkup  [%f] ", __FUNCTION__, __LINE__, qd.markupdata[INDEX1M].markupSet[MARKUPINDEXPRI].offerMarkup ,qd.markupdata[INDEX1M].markupSet[MARKUPINDEXPRI].bidMarkup );    
		//mds_log(market, LOG_MUST, "[%-15s][%4d] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].bidMarkup  [%f] qd.markupdata[INDEXTOD].markupSet[MARKUPINDEXEMP].offerMarkup  [%f] ", __FUNCTION__, __LINE__, qd.markupdata[INDEX1M].markupSet[MARKUPINDEXENT].offerMarkup ,qd.markupdata[INDEX1M].markupSet[MARKUPINDEXENT].bidMarkup );    


	}

	//재정통화의 거래량은 원수량과 KRW수량중 적은것으로 설정해준다. 
	if (krwq->spotdata.bidvol >= baseq->spotdata.bidvol)
	{
		qd.spotdata.bidvol = baseq->spotdata.bidvol;
		qd.spotdata.bidbestvol = baseq->spotdata.bidbestvol;

	}
	else
	{
		qd.spotdata.bidvol = krwq->spotdata.bidvol;
		qd.spotdata.bidbestvol = krwq->spotdata.bidbestvol;
	}

	if (krwq->spotdata.offervol >= baseq->spotdata.offervol)
	{
		qd.spotdata.offervol = baseq->spotdata.offervol;
		qd.spotdata.offerbestvol = baseq->spotdata.offerbestvol;
	}
	else
	{
		qd.spotdata.offervol = krwq->spotdata.offervol;
		qd.spotdata.offerbestvol = krwq->spotdata.offerbestvol;
	}


	m = &rfolder->mstr;

	qd.foworddata[INDEX1W].donedaybidswap = (qd.swapdata[0].bidlast) / (m->expireNday1W);
	qd.foworddata[INDEX1W].donedayofferswap = (qd.swapdata[0].offerlast) / (m->expireNday1W);
	qd.foworddata[INDEX1M].donedaybidswap = (qd.swapdata[1].bidlast - qd.swapdata[0].bidlast) / (m->expireNday1M - m->expireNday1W);
	qd.foworddata[INDEX1M].donedayofferswap = (qd.swapdata[1].offerlast - qd.swapdata[0].offerlast) / (m->expireNday1M - m->expireNday1W);
	qd.foworddata[INDEX2M].donedaybidswap = (qd.swapdata[2].bidlast - qd.swapdata[1].bidlast) / (m->expireNday2M - m->expireNday1M);
	qd.foworddata[INDEX2M].donedayofferswap = (qd.swapdata[2].offerlast - qd.swapdata[1].offerlast) / (m->expireNday2M - m->expireNday1M);
	qd.foworddata[INDEX3M].donedaybidswap = (qd.swapdata[3].bidlast - qd.swapdata[2].bidlast) / (m->expireNday3M - m->expireNday2M);
	qd.foworddata[INDEX3M].donedayofferswap = (qd.swapdata[3].offerlast - qd.swapdata[2].offerlast) / (m->expireNday3M - m->expireNday2M);
	qd.foworddata[INDEX6M].donedaybidswap = (qd.swapdata[4].bidlast - qd.swapdata[3].bidlast) / (m->expireNday6M - m->expireNday3M);
	qd.foworddata[INDEX6M].donedayofferswap = (qd.swapdata[4].offerlast - qd.swapdata[3].offerlast) / (m->expireNday6M - m->expireNday3M);
	qd.foworddata[INDEX9M].donedaybidswap = (qd.swapdata[5].bidlast - qd.swapdata[4].bidlast) / (m->expireNday9M - m->expireNday6M);
	qd.foworddata[INDEX9M].donedayofferswap = (qd.swapdata[5].offerlast - qd.swapdata[4].offerlast) / (m->expireNday9M - m->expireNday6M);
	qd.foworddata[INDEX12M].donedaybidswap = (qd.swapdata[6].bidlast - qd.swapdata[5].bidlast) / (m->expireNday12M - m->expireNday9M);
	qd.foworddata[INDEX12M].donedayofferswap = (qd.swapdata[6].offerlast - qd.swapdata[5].offerlast) / (m->expireNday12M - m->expireNday9M);
	qd.foworddata[INDEXTOD].donedaybidswap = 0;
	qd.foworddata[INDEXTOD].donedayofferswap = 0;
	qd.foworddata[INDEXTOM].donedaybidswap = 0;
	qd.foworddata[INDEXTOM].donedayofferswap = 0;
	//  mds_log(market, LOG_MUST, "[%-15s][%4d] qd.foworddata[INDEX1M].donedayofferswap [%f] ", __FUNCTION__, __LINE__,qd.foworddata[INDEX1M].donedayofferswap);    
	//mds_log(market, LOG_MUST, "[%-15s][%4d] qd.foworddata[INDEX1M].donedayofferswap [%f] ", __FUNCTION__, __LINE__,qd.foworddata[INDEX2M].donedayofferswap);    
	//mds_log(market, LOG_MUST, "[%-15s][%4d] qd.foworddata[INDEX1M].donedayofferswap [%f] ", __FUNCTION__, __LINE__,qd.foworddata[INDEX3M].donedayofferswap);    
	//mds_log(market, LOG_MUST, "[%-15s][%4d] qd.foworddata[INDEX1M].donedayofferswap [%f] ", __FUNCTION__, __LINE__,qd.foworddata[INDEX6M].donedayofferswap);    

	for (ii = 0; ii < MARKUPGROUPCNT; ii++)
	{

		qd.foworddata[INDEX1W].onedayMarkup[ii].donedaybidMarkup = (qd.markupdata[INDEX1W].markupSet[ii].bidMarkup) / (m->expireNday1W);
		qd.foworddata[INDEX1W].onedayMarkup[ii].donedayofferMarkup = (qd.markupdata[INDEX1W].markupSet[ii].offerMarkup) / (m->expireNday1W);
		qd.foworddata[INDEX1M].onedayMarkup[ii].donedaybidMarkup = (qd.markupdata[INDEX1M].markupSet[ii].bidMarkup - qd.markupdata[INDEX1W].markupSet[ii].bidMarkup) / (m->expireNday1M - m->expireNday1W);
		qd.foworddata[INDEX1M].onedayMarkup[ii].donedayofferMarkup = (qd.markupdata[INDEX1M].markupSet[ii].offerMarkup - qd.markupdata[INDEX1W].markupSet[ii].offerMarkup) / (m->expireNday1M - m->expireNday1W);
		qd.foworddata[INDEX2M].onedayMarkup[ii].donedaybidMarkup = (qd.markupdata[INDEX2M].markupSet[ii].bidMarkup - qd.markupdata[INDEX1M].markupSet[ii].bidMarkup) / (m->expireNday2M - m->expireNday1M);
		qd.foworddata[INDEX2M].onedayMarkup[ii].donedayofferMarkup = (qd.markupdata[INDEX2M].markupSet[ii].offerMarkup - qd.markupdata[INDEX1M].markupSet[ii].offerMarkup) / (m->expireNday2M - m->expireNday1M);
		qd.foworddata[INDEX3M].onedayMarkup[ii].donedaybidMarkup = (qd.markupdata[INDEX3M].markupSet[ii].bidMarkup - qd.markupdata[INDEX2M].markupSet[ii].bidMarkup) / (m->expireNday3M - m->expireNday2M);
		qd.foworddata[INDEX3M].onedayMarkup[ii].donedayofferMarkup = (qd.markupdata[INDEX3M].markupSet[ii].offerMarkup - qd.markupdata[INDEX2M].markupSet[ii].offerMarkup) / (m->expireNday3M - m->expireNday2M);
		qd.foworddata[INDEX6M].onedayMarkup[ii].donedaybidMarkup = (qd.markupdata[INDEX6M].markupSet[ii].bidMarkup - qd.markupdata[INDEX3M].markupSet[ii].bidMarkup) / (m->expireNday6M - m->expireNday3M);
		qd.foworddata[INDEX6M].onedayMarkup[ii].donedayofferMarkup = (qd.markupdata[INDEX6M].markupSet[ii].offerMarkup - qd.markupdata[INDEX3M].markupSet[ii].offerMarkup) / (m->expireNday6M - m->expireNday3M);
		qd.foworddata[INDEX9M].onedayMarkup[ii].donedaybidMarkup = (qd.markupdata[INDEX9M].markupSet[ii].bidMarkup - qd.markupdata[INDEX6M].markupSet[ii].bidMarkup) / (m->expireNday9M - m->expireNday6M);
		qd.foworddata[INDEX9M].onedayMarkup[ii].donedayofferMarkup = (qd.markupdata[INDEX9M].markupSet[ii].offerMarkup - qd.markupdata[INDEX6M].markupSet[ii].offerMarkup) / (m->expireNday9M - m->expireNday6M);
		qd.foworddata[INDEX12M].onedayMarkup[ii].donedaybidMarkup = (qd.markupdata[INDEX12M].markupSet[ii].bidMarkup - qd.markupdata[INDEX9M].markupSet[ii].bidMarkup) / (m->expireNday12M - m->expireNday9M);
		qd.foworddata[INDEX12M].onedayMarkup[ii].donedayofferMarkup = (qd.markupdata[INDEX12M].markupSet[ii].offerMarkup - qd.markupdata[INDEX9M].markupSet[ii].offerMarkup) / (m->expireNday12M - m->expireNday9M);
		qd.foworddata[INDEXTOD].onedayMarkup[ii].donedaybidMarkup = 0;
		qd.foworddata[INDEXTOD].onedayMarkup[ii].donedayofferMarkup = 0;
		qd.foworddata[INDEXTOM].onedayMarkup[ii].donedaybidMarkup = 0;
		qd.foworddata[INDEXTOM].onedayMarkup[ii].donedayofferMarkup = 0;
	}
	//mds_log(market, LOG_MUST, "[%-15s][%4d] qd.spotdata.bidlast [ ", __FUNCTION__, __LINE__);    
	qd.spotdata.midlast = (qd.spotdata.bidlast + qd.spotdata.offerlast) / 2;
	qd.spotdata.midbest = (qd.spotdata.bidbest + qd.spotdata.offerbest) / 2;
	quote_proc(market, rfolder, &qd, check, 1);
	return(rfolder);


}


  
void ChangeSpotMarkup(struct q_Markup* q, int nMarkupType)
{
	int ii;
	for (ii = 0; ii < MARKUPGROUPCNT; ii++)
	{
		if (nMarkupType == 0)  //SMBS
		{
			q->markupSet[ii].bidMarkup = q->markupSet[ii].bidSMBSMarkup;
			q->markupSet[ii].offerMarkup = q->markupSet[ii].offerSMBSMarkup;
		}
		else if (nMarkupType == 1) //CMBS,수기
		{
			q->markupSet[ii].bidMarkup = q->markupSet[ii].bidCMBSMarkup;
			q->markupSet[ii].offerMarkup = q->markupSet[ii].offerCMBSMarkup;
		}
	}
}



int CheckQuotePrice(MARKET *market, int iSource, char *msgb, int msgl)
{
	MDFOLD  *folder;
	MDQUOT  *mdq;
	struct cmbsquote  *quote = (struct cmbsquote *)msgb;
	struct  q_data  qd;
	double dTemp;
	memset(&qd, 0x00, sizeof(struct q_data));
	str2s(qd.symb, SYMB_LEN, quote->symb, sizeof(quote->symb));

	//USDCNH의 시간은 따로 등록을 안하고 USDCNY의 시간을 사용한다. 
	if (memcmp(qd.symb, "USDCNH", 6) == 0)
	{
		if ((folder = mds_getfolder(market, "USDCNY")) == NULL)
			return -1;

	}
	else
	{
		if ((folder = mds_getfolder(market, qd.symb)) == NULL)
			return -1;
	}

	/*-------------------------
	 * data set
	 * -----------------------*/
	MDMSTR    *m;
	uint32_t ymd, hms;
	m = &folder->mstr;
	mdq = &folder->quot;
	///////  SOURCE_SMBS        0
	///////  SOURCE_CMBS        1
	///////  SOURCE_HANDWRTING  2
	///////  SOURCE_INITPRICE   3
	///////  SOURCE_STOP         4
	//해당시세 사용가능 시간 체크 시작
	if ((m->pricestat == SOURCE_STOP - 1) && iSource != SOURCE_INITPRICE) //시세정지
		return -1;
	if (iSource != SOURCE_INITPRICE) //시세초기화 이면 반영안함 
	{

		if (m->pricestat == SOURCE_HANDWRTING) // 수기입력 
		{
			if (memcmp(qd.symb, "USDKRW", 6) == 0)
			{
				// 수기입력인데  가상잔량이 SMBS면 가상잔량 다시 로드 
				if (m->iNowVirtualAmoutType == 0)
				{
					m->iNowVirtualAmoutType = 1;
					ReloadVirtualQty();
				}
				ChangeSpotMarkup(&mdq->markupdata[INDEXSPOT], m->iNowVirtualAmoutType);
			}
			else  //가상잔량타입을 시간구분으로 사용
			{
				m->iNowVirtualAmoutType = 1;
			}
			//  if (m->exrt != 2)
				//시간에 따른 SPOT마크업 설정값 반영 재정이 아닐때만
			ChangeSpotMarkup(&mdq->markupdata[INDEXSPOT], m->iNowVirtualAmoutType);
			if (iSource != SOURCE_HANDWRTING) //수기입력 환율이 아니면 처리 안함
				return -1;
		}
		else
		{
			if (iSource == SOURCE_HANDWRTING) //수기입력 환율이면 처리 안함
				return -1;
		}
		mds_time(market, 0, &ymd, &hms, NULL, NULL);  // get local date & time
	  //  mds_log(market, LOG_MUST, "[%-15s][%4d] hms [%d] ", __FUNCTION__, __LINE__, hms);


		 //전일대비 이상가격 필터 수기입력 제외 (전일대비 이격 50%)
		if (iSource != SOURCE_HANDWRTING)
		{
			double dBidPriceIN, dAskPriceIN, dPreBidPrice, dPreAskPrice;
			dBidPriceIN = atof(quote->bidprice);
			dAskPriceIN = atof(quote->offerprice);
			dPreBidPrice = mdq->spotdata.midbase;   //전일종가                                                                                                  ;
			dPreAskPrice = mdq->spotdata.midbase; //전일종가
			//bidPrice > askprice일때 필터 추가 
			if (dBidPriceIN > dAskPriceIN)
			{
				//	mds_log(market, LOG_DEBUG, "[%-15s][%4d] BIDPrice[%f] > ASKPrice[%f]", __FUNCTION__, __LINE__, dBidPriceIN,dAskPriceIN);
				return -1;
			}
			//기존값에 50%이상 차이날 경우 Pass
			if ((dBidPriceIN > (dPreBidPrice + (dPreBidPrice / CEHCKSTOPDIFFRATE))) && dPreBidPrice > 0.000000001&& dBidPriceIN > 0.000000001)
			{
				m->pricestat = SOURCE_STOP - 1;
				m->trdf = 0; //중지이후 해제하였을경우 시장가 체결방지 . 
				mds_log(market, LOG_MUST, "[%-15s][%4d] 전일대비  [50]%이상 이격발생  직전 BID [%f] 전일 BID [%f] 통화쌍 [%s][%d]", __FUNCTION__, __LINE__, dBidPriceIN, dPreBidPrice, qd.symb, mdq->seqn);
				if (StopPriceOrign(market, qd.symb) != 0)
				{
					l_db2rollback();
				}
				else
				{
					l_db2commit();
				}
				return -1;
			}
			if ((dBidPriceIN < (dPreBidPrice - (dPreBidPrice / CEHCKSTOPDIFFRATE))) && dPreBidPrice > 0.000000001&& dBidPriceIN > 0.000000001)
			{
				m->pricestat = SOURCE_STOP - 1;
				m->trdf = 0; //중지이후 해제하였을경우 시장가 체결방지 . 
				mds_log(market, LOG_MUST, "[%-15s][%4d] 전일대비 [50]%이상  이격발생  직전 BID [%f] 전일 BID [%f] 통화쌍 [%s][%d]", __FUNCTION__, __LINE__, dBidPriceIN, dPreBidPrice, qd.symb, mdq->seqn);
				if (StopPriceOrign(market, qd.symb) != 0)
				{
					l_db2rollback();
				}
				else
				{
					l_db2commit();
				}
				return -1;
			}

			if ((dAskPriceIN > (dPreAskPrice + (dPreAskPrice / CEHCKSTOPDIFFRATE))) && dPreAskPrice > 0.000000001&& dAskPriceIN > 0.000000001)
			{
				m->pricestat = SOURCE_STOP - 1;
				m->trdf = 0; //중지이후 해제하였을경우 시장가 체결방지 . 

				mds_log(market, LOG_MUST, "[%-15s][%4d] 전일대비 [50]%이상 이격발생  수신 ASK [%f] 전일 ASK [%f] 통화쌍 [%s][%d]", __FUNCTION__, __LINE__, dAskPriceIN, dPreAskPrice, qd.symb, mdq->seqn);
				if (StopPriceOrign(market, qd.symb) != 0)
				{
					l_db2rollback();
				}
				else
				{
					l_db2commit();
				}
				return -1;
			}
			if ((dAskPriceIN < (dPreAskPrice - (dPreAskPrice / CEHCKSTOPDIFFRATE))) && dPreAskPrice > 0.000000001&& dAskPriceIN > 0.000000001)
			{
				m->pricestat = SOURCE_STOP - 1;
				m->trdf = 0; //중지이후 해제하였을경우 시장가 체결방지 . 
				mds_log(market, LOG_MUST, "[%-15s][%4d] 전일대비 [50]%이상 이격발생  수신 ASK [%f] 전일 ASK [%f] 통화쌍 [%s][%d]", __FUNCTION__, __LINE__, dAskPriceIN, dPreAskPrice, qd.symb, mdq->seqn);
				if (StopPriceOrign(market, qd.symb) != 0)
				{
					l_db2rollback();
				}
				else
				{
					l_db2commit();
				}
				return -1;
			}
		}


		//이상가격 필터 수기입력 제외 (직전대비 이격 10%)
		if (iSource != SOURCE_HANDWRTING)
		{
			double dBidPriceIN, dAskPriceIN, dPreBidPrice, dPreAskPrice;
			dBidPriceIN = atof(quote->bidprice);
			dAskPriceIN = atof(quote->offerprice);
			dPreBidPrice = mdq->spotdata.bidlast; ;
			dPreAskPrice = mdq->spotdata.offerlast;
			//직전대비 동일 가격 pass
	   //     if ((dBidPriceIN == dPreBidPrice) && (dAskPriceIN == dPreAskPrice))
		 //   {
		//      mds_log(market, LOG_MUST, "[%-15s][%4d]  직전대비 동일가격  수신 BID [%f] 직전 BID [%f] 통화쌍 [%s][%d]", __FUNCTION__, __LINE__, dBidPriceIN,dPreBidPrice,qd.symb,mdq->seqn);
		//      mds_log(market, LOG_MUST, "[%-15s][%4d]  직전대비 동일가격  수신 ASK [%f] 직전 ASK [%f] 통화쌍 [%s][%d]", __FUNCTION__, __LINE__, dAskPriceIN,dPreAskPrice,qd.symb,mdq->seqn);
		 //     return -1;
		 //   }  
			//기존값에 50%이상 차이날 경우 Pass
			if ((dBidPriceIN > (dPreBidPrice + (dPreBidPrice / CHECKDIFFRATE))) && dPreBidPrice > 0.000000001&& dBidPriceIN > 0.000000001)
			{
				mds_log(market, LOG_MUST, "[%-15s][%4d]  직전대비 [%d]%이상 이격발생  수신 BID [%f] 직전 BID [%f] 통화쌍 [%s][%d]", __FUNCTION__, __LINE__, CHECKDIFFRATE, dBidPriceIN, dPreBidPrice, qd.symb, mdq->seqn);
				return -1;
			}
			if ((dBidPriceIN < (dPreBidPrice - (dPreBidPrice / CHECKDIFFRATE))) && dPreBidPrice > 0.000000001&& dBidPriceIN > 0.000000001)
			{
				mds_log(market, LOG_MUST, "[%-15s][%4d]  직전대비 [%d]%이상 이격발생  수신 BID [%f] 직전 BID [%f] 통화쌍 [%s][%d]", __FUNCTION__, __LINE__, CHECKDIFFRATE, dBidPriceIN, dPreBidPrice, qd.symb, mdq->seqn);
				return -1;
			}

			if ((dAskPriceIN > (dPreAskPrice + (dPreAskPrice / CHECKDIFFRATE))) && dPreAskPrice > 0.000000001&& dAskPriceIN > 0.000000001)
			{
				mds_log(market, LOG_MUST, "[%-15s][%4d]  직전대비 [%d]%이상 이격발생  수신 ASK [%f] 직전 ASK [%f] 통화쌍 [%s][%d]", __FUNCTION__, __LINE__, CHECKDIFFRATE, dAskPriceIN, dPreAskPrice, qd.symb, mdq->seqn);
				return -1;
			}
			if ((dAskPriceIN < (dPreAskPrice - (dPreAskPrice / CHECKDIFFRATE))) && dPreAskPrice > 0.000000001&& dAskPriceIN > 0.000000001)
			{
				mds_log(market, LOG_MUST, "[%-15s][%4d]  직전대비 [%d]%이상 이격발생  수신 ASK [%f] 직전 ASK  [%f] 통화쌍 [%s][%d]", __FUNCTION__, __LINE__, CHECKDIFFRATE, dAskPriceIN, dPreAskPrice, qd.symb, mdq->seqn);
				return -1;
			}
		}

		if (memcmp(qd.symb, "CNHKRW", 6) == 0)
		{
			return 0;
		}


		if (iSource == SOURCE_SMBS) //SMBS소스일경우 
		{
			//SMBS시간이후 CMBS시간 이전이 아닐경우 처리안함
			if (hms >= m->smbstime && hms < m->cmbstime)
			{
				if (memcmp(qd.symb, "USDKRW", 6) == 0)
				{
					//SMBS환율시간인데 가상잔량이 SMBS가 아니면 가상잔량 다시 로드 
					if (m->iNowVirtualAmoutType != 0)
					{
						m->iNowVirtualAmoutType = 0;
						ReloadVirtualQty();
					}
					ChangeSpotMarkup(&mdq->markupdata[INDEXSPOT], m->iNowVirtualAmoutType);
				}
				else  //가상잔량타입을 시간구분으로 사용
				{
					m->iNowVirtualAmoutType = 0;
				}
				//    if (m->exrt != 2)
					//시간에 따른 SPOT마크업 설정값 반영 
				ChangeSpotMarkup(&mdq->markupdata[INDEXSPOT], m->iNowVirtualAmoutType);
				return 0;
			}
			else
				return -1;

		}

		//CMBS시간이후 중단시간 이전이 아닐경우 처리안함  
		if (iSource == SOURCE_CMBS) //CMBS소스일경우 
		{
			if (hms >= m->cmbstime && hms < m->stoptime)
			{
				if (memcmp(qd.symb, "USDKRW", 6) == 0)
				{
					//CMBS환율시간인데 가상잔량이 SMBS면 가상잔량 다시 로드 
					if (m->iNowVirtualAmoutType == 0)
					{
						m->iNowVirtualAmoutType = 1;
						ReloadVirtualQty();
					}
					ChangeSpotMarkup(&mdq->markupdata[INDEXSPOT], m->iNowVirtualAmoutType);
				}
				else  //가상잔량타입을 시간구분으로 사용
				{
					m->iNowVirtualAmoutType = 1;
				}
				//    if (m->exrt != 2)
					//시간에 따른 SPOT마크업 설정값 반영 
				ChangeSpotMarkup(&mdq->markupdata[INDEXSPOT], m->iNowVirtualAmoutType);
				return 0;
			}
			else
			{
				return -1;
			}
		}

	}

	////해당시세 사용가능 시간체크 종료
	return 0;
}




// 가격을 저장하고 체결 호가정보를 만들기 위해 호출하는 함수 
int SendAndSavePriceData(MARKET *market, int iSource, char *msgb, int msgl)
{
	MDFOLD  *folder;
	MDFOLD  *rfolder;
	char  check[MAX_ISAM_F];
	memset(&check[0], 0, sizeof(check));
	int nConv = 0;

	//시장시세는 전송하고 끝낸다 
	if (iSource == SOURCE_MARKET)
	{
		// 1. 전달받은 시세를 반영
		folder = (*getSpotquot)(market, msgb, msgl, check);
		if (folder == NULL)
			return -1;
		//클라이언트로 전송 
		push_marketquot(market, &folder->quot, folder->mstr.zdiv);
		return;
	}
	// mds_log(market, LOG_MUST, "[%-15s][%4d] qd.spotdata.bidlast [%s] ", __FUNCTION__, __LINE__, msgb);        
	 //시세 가능 여부, 배드틱 체크 

	if (CheckQuotePrice(market, iSource, msgb, msgl) != 0)
	{
		return -1;
	}



	// 1. 전달받은 시세를 반영
	folder = (*getSpotquot)(market, msgb, msgl, check);
	if (folder == NULL)
		return -1;
	/// mds_log(market, LOG_MUST, "[%-15s][%4d]iSource [%d][%d] folder->mstr.trdf[%d]folder->quot.symb[%s] ", __FUNCTION__, __LINE__, iSource,SOURCE_INITPRICE,folder->mstr.trdf,folder->quot.symb);      

	char ccy1[3];
	char ccy2[3];
	sprintf(ccy1, "%.3s", folder->quot.symb);
	sprintf(ccy2, "%.3s", &folder->quot.symb[3]);

	if (iSource != SOURCE_INITPRICE || (memcmp(folder->quot.symb, "MARKRW", 6) == 0))
	{
		//MARKRW예외... 처음시작시 초기화는 상태변경하면 안됨.. 이루 입력되는 환율은 상태변경 3으로 해야함..
	//    if (folder->mstr.trdf < 3 && (memcmp(folder->quot.symb, "MARKRW", 6) == 0))
	 //     folder->mstr.trdf = 2;  //2:초기화만 수신 
	 //   else

		folder->mstr.trdf = 3;  //시세 수신됨 0:미개장 1:재정통화에 USDKRW만 수신 2:재정통화에 이종통화수신 3:시세수신완료   
	}
	//   if (iSource != SOURCE_INITPRICE) //시세초기화 이면 전송안함
   //  {  
	   // 2. 해당시세 클라이언트로 전송
	sendfolder(market, folder, check, iSource);
	//CMBS시세는 기본종목 커밋이후 재정계산
	if (iSource == SOURCE_CMBS)
		l_db2commit();
	//  }


	  // KRW시세가 아니면  해당 시세의 재정통화 계산 
	  // USDKRW 시세가 내려오면 모든 재정환율 시세`` 새로 다 만들어 준다. 
	if (memcmp(ccy2, "KRW", 3) != 0)
	{
		//CNH는 재정을 만들지 않는다. 직접받는 CNHKRW시세 사용 
		if (memcmp(ccy2, "CNH", 3) != 0)
		{
			//재정통화 계산
			memset(&check[0], 0, sizeof(check));
			rfolder = getCalcKRWquot(market, folder->quot.symb, check);
			if (rfolder != NULL)
			{

				//  if (iSource != SOURCE_INITPRICE) //시세초기화 이면 전송안함
				{
					// 2. 해당시세 클라이언트로 전송
					sendfolder(market, rfolder, check, iSource);
				}

				memset(&check[0], 0, sizeof(check));
				if (iSource != SOURCE_INITPRICE)
				{

					if (rfolder->mstr.trdf == 1 || rfolder->mstr.trdf == 3) //USDKRW수신된상태일경우 
						rfolder->mstr.trdf = 3;   //시세 수신됨 0:미개장 1:재정통화에 USDKRW만 수신 2:재정통화에 이종통화수신 3:시세수신완료   
					else
						rfolder->mstr.trdf = 2;
				}
			}
		}
	}
	else if (memcmp(ccy2, "KRW", 3) == 0 && (memcmp(ccy1, "USD", 3) == 0))   //USD/KRW일경우 KRW - 전체 시세가 변경된다.   
	{

		MARKET  *market2;
		market2 = mds_open("CMBS", O_RDWR);
		if (market2 == NULL)
			return 0;
		mds_setfolder(market2, NULL);

		while ((folder = mds_popfolder(market2)) != NULL)
		{
			MDMSTR *m = &folder->mstr;
			MDBOOK  *book = &folder->book;
			MDQUOT  *quot = &folder->quot;

			if (m == NULL)
				continue;

			int nPind = m->zdiv;
			memset(&g_quote, 0x00, sizeof(g_quote));
			if (m->exrt == 2) //재정환율 재계산 1: 재정환율종목
			{
				if (quot == NULL)
					continue;
				if (quot->spotdata.bidlast == 0)
					continue;

				//CNH는 재정을 만들지 않는다. 직접받는 CNHKRW시세 사용 
				if (memcmp(quot->symb, "USDCNH", 6) != 0)
				{

					memset(&check[0], 0, sizeof(check));

					rfolder = getCalcKRWquot(market, quot->symb, check);

					if (rfolder != NULL)
					{
						//  if (iSource != SOURCE_INITPRICE) //시세초기화 이면 전송안함
						  //{  
							// 2. 해당시세 클라이언트로 전송
						sendfolder(market, rfolder, check, iSource);
						//}

						memset(&check[0], 0, sizeof(check));
						if (iSource != SOURCE_INITPRICE)
						{

							if (rfolder->mstr.trdf == 2 || rfolder->mstr.trdf == 3) //이종수신된 상태일경우
								rfolder->mstr.trdf = 3;   //시세 수신됨 0:미개장 1:재정통화에 USDKRW만 수신 2:재정통화에 이종통화수신 3:시세수신완료   
							else
								rfolder->mstr.trdf = 1;
						}
						rfolder->mstr.iNowVirtualAmoutType = iSource;

					}


				}
			}
		}
		mds_close(market2);
	}
	//
	l_db2commit();
}


int GetDiffDate(char* startDate, char* toDate)
{
	time_t start, end;

	struct tm stime, etime;
	int tm_day, tm_hour, tm_min, tm_sec;
	double diff;
	int start_year, start_month, start_day;
	int end_year, end_month, end_day;
	char Temp[5];
	int  nDiffDay;

	memset(Temp, 0, sizeof(Temp));
	sprintf(Temp, "%.4s", startDate);
	start_year = atoi(Temp);

	memset(Temp, 0, sizeof(Temp));
	sprintf(Temp, "%.2s", &startDate[4]);
	start_month = atoi(Temp);

	memset(Temp, 0, sizeof(Temp));
	sprintf(Temp, "%.2s", &startDate[6]);
	start_day = atoi(Temp);

	memset(Temp, 0, sizeof(Temp));
	sprintf(Temp, "%.4s", toDate);
	end_year = atoi(Temp);

	memset(Temp, 0, sizeof(Temp));
	sprintf(Temp, "%.2s", &toDate[4]);
	end_month = atoi(Temp);

	memset(Temp, 0, sizeof(Temp));
	sprintf(Temp, "%.2s", &toDate[6]);
	end_day = atoi(Temp);



	stime.tm_year = start_year - 1900;
	stime.tm_mon = start_month - 1;
	stime.tm_mday = start_day;
	stime.tm_hour = 0;
	stime.tm_min = 0;

	stime.tm_sec = 0;
	stime.tm_isdst = 0;

	etime.tm_year = end_year - 1900;
	etime.tm_mon = end_month - 1;
	etime.tm_mday = end_day;
	etime.tm_hour = 0;
	etime.tm_min = 0;

	etime.tm_sec = 0;
	etime.tm_isdst = 0;
	start = mktime(&stime);
	end = mktime(&etime);
	nDiffDay = difftime(end, start);

	nDiffDay = nDiffDay / (24 * 60 * 60);

	return nDiffDay;
}



/******************************************************************************
* Function Name : GetBookToDate(char *symbol, char* Expiredate, BOOKSTRUCT* rtBook,int nMarkupType)
* Description   : 입력일자의 시세정보를 rtBook에 넣어줍니다. 마크업 포함 시세입니다. 
******************************************************************************/
int GetBookToDate(char *symbol, char* Expiredate, BOOKSTRUCT* rtBook, int nMarkupType, char ExpiredateType)
{
	MARKET  *market, m;
	MDFOLD  *folder;
	char  esym[24], exnm[20];
	char  dateType[2];
	int    len;

	memset(&m, 0, sizeof(MARKET));
	sprintf(m.procname, "Custom");
	STR2S(esym, symbol);
	STR2S(exnm, "CMBS");
	folder = pibo_getfolder2(exnm, esym, &market);
	if (market == NULL)
	{
		mds_log(&m, LOG_MUST, "Exchange is not found.  Exchange='%s'", exnm);
		return -2;
	}

	if (folder == NULL)
	{
		mds_log(&m, LOG_MUST, "[%-15s][%4d]Symbol is not found. esym='%s'  for '%s'  ", __FUNCTION__, __LINE__, esym, exnm);
		return -3;
	}

	//마크업 그룹을 가지고 온다
	if (nMarkupType == MARKUPEMP) //직원
		nMarkupType = MARKUPINDEXEMP;
	else if (nMarkupType == MARKUPPRI)//개인
		nMarkupType = MARKUPINDEXPRI;
	else if (nMarkupType == MARKUPENT) //기업
		nMarkupType = MARKUPINDEXENT;
	else if (nMarkupType == MARKUPENT2) //기관
		nMarkupType = MARKUPINDEXENT2;

	//  mds_log(&m, LOG_MUST, "[%-15s][%d] Input symbol ='%s'  nMarkupType ='%d'  Expiredate ='%s' ", __FUNCTION__, __LINE__, symbol, nMarkupType, Expiredate);   
	memset(&dateType, 0, sizeof(dateType));
	sprintf(dateType, "%c", ExpiredateType);
	MDMSTR  *mstr;
	MDQUOT  *quot;
	MDBOOK  *book;

	char    form[16], value[16];
	char    pind;
	int     price;
	int     diff, rate;
	int     rc;
	int    ii;
	double  dval;
	double  a, b;
	mstr = &folder->mstr;
	quot = &folder->quot;
	book = &folder->book;

	char   cSpotday[8];
	double dpinc = mstr->pinc * 10;

	//  sprintf(cToday, "%08d" ,folder->quot.kymd);
	sprintf(cSpotday, "%08d", atoi(mstr->expiredateSpot));
	int diffDate = GetDiffDate(cSpotday, Expiredate);
	int Basediff = 0; //기준환율과 일자차이  
	int Tnrindex = 0;
	//SPOT마크업 
	rtBook->bidSpotMarkup = quot->markupdata[INDEXSPOT].markupSet[nMarkupType].bidMarkup* mstr->swappmul;//bidSpotMarkup
	rtBook->askSpotMarkup = quot->markupdata[INDEXSPOT].markupSet[nMarkupType].offerMarkup *mstr->swappmul;  //askSpotMarkup    


  //  mds_log(&m, LOG_DEBUG, "[%-15s][%d] mstr->swappmul='%f'", __FUNCTION__, __LINE__, mstr->swappmul);
  //  mds_log(&m, LOG_DEBUG, "[%-15s][%d] diffDate='%d'", __FUNCTION__, __LINE__, diffDate);
   // mds_log(&m, LOG_DEBUG, "[%-15s][%d] mstr->expireNday1W=[%d] ", __FUNCTION__, __LINE__, mstr->expireNday1W);
	if ((memcmp(esym, "MARKRW", 6) == 0) || dateType[0] == '3' || dateType[0] == '6')  //spot시세 MAR는 무조건 spot )  //spot시세 MAR는 무조건 spot 
	{

		rtBook->bidMarkup = rtBook->bidSpotMarkup;//quot->markupdata[INDEXSPOT].markupSet[nMarkupType].bidMarkup* mstr->swappmul;
		rtBook->askMarkup = rtBook->askSpotMarkup;//quot->markupdata[INDEXSPOT].markupSet[nMarkupType].offerMarkup *mstr->swappmul;  
		b = book->spotdata.bid[0].pbid + rtBook->bidSpotMarkup;
		a = book->spotdata.ask[0].pask + rtBook->askSpotMarkup;
		char Temp[16];
		sprintf(Temp, "%d", 0);
		sprintf(rtBook->difftnrday, "%s", Temp);
		rtBook->bidswapperday = 0;
		rtBook->offerswapperday = 0;
		sprintf(rtBook->basesubcode, "%3s", subCodeSpot[0]);

	}
	else    //Foword시세 
	{
		if (dateType[0] == '1')  //Today시세
		{
			Basediff = 0;
			Tnrindex = 7;
		}
		else if (dateType[0] == '2')  //Tom시세
		{
			Basediff = 0;
			Tnrindex = 8;
		}
		else if (mstr->expireNday1W >= diffDate)   //1Week 시세 
		{
			Basediff = mstr->expireNday1W - diffDate;
			Tnrindex = 0;
		}
		else if (mstr->expireNday1M >= diffDate) //1M 시세 
		{
			Basediff = mstr->expireNday1M - diffDate;
			Tnrindex = 1;
		}
		else if (mstr->expireNday2M >= diffDate) //2M 시세 
		{
			Basediff = mstr->expireNday2M - diffDate;
			Tnrindex = 2;
		}
		else if (mstr->expireNday3M >= diffDate) //3M 시세 
		{
			Basediff = mstr->expireNday3M - diffDate;
			Tnrindex = 3;
		}
		else if (mstr->expireNday6M >= diffDate) //6M 시세 
		{
			Basediff = mstr->expireNday6M - diffDate;
			Tnrindex = 4;
		}
		else if (mstr->expireNday9M >= diffDate)  //9M 시세 
		{
			Basediff = mstr->expireNday9M - diffDate;
			Tnrindex = 5;
		}
		else if (mstr->expireNday12M >= diffDate) //12M 시세 
		{
			Basediff = mstr->expireNday12M - diffDate;
			Tnrindex = 6;
		}
		else if (mstr->expireNday12M < diffDate)
			return -5;  //가능기간 초과 ..

		  //마크업 = (테너마크업 *pip단위) - (테너1일마크업*일수*pip단위)
		if (nMarkupType == MARKUPINDEXALL)
		{
			rtBook->bidMarkup = 0;
			rtBook->askMarkup = 0;
		}
		else
		{
			rtBook->bidMarkup = (quot->markupdata[Tnrindex].markupSet[nMarkupType].bidMarkup    * mstr->swappmul) - (quot->foworddata[Tnrindex].onedayMarkup[nMarkupType].donedaybidMarkup* Basediff* mstr->swappmul) + rtBook->bidSpotMarkup;
			rtBook->askMarkup = (quot->markupdata[Tnrindex].markupSet[nMarkupType].offerMarkup  * mstr->swappmul) - (quot->foworddata[Tnrindex].onedayMarkup[nMarkupType].donedayofferMarkup* Basediff* mstr->swappmul) + rtBook->askSpotMarkup;
		}

		//금액 = 테너가격 - (1일스왑 *일수)    #제거  + 마크업 + SPot마크업
		b = book->foworddata[Tnrindex].bid[0].pbid - (quot->foworddata[Tnrindex].donedaybidswap * Basediff* mstr->swappmul) + rtBook->bidMarkup;
		a = book->foworddata[Tnrindex].ask[0].pask - (quot->foworddata[Tnrindex].donedayofferswap * Basediff* mstr->swappmul) + rtBook->askMarkup;

		char Temp[16];
		sprintf(Temp, "%d", Basediff);
		sprintf(rtBook->basesubcode, "%3s", subFowardode[Tnrindex]);
		sprintf(rtBook->difftnrday, "%4s", Temp);
		rtBook->bidswapperday = quot->foworddata[Tnrindex].donedaybidswap * mstr->swappmul;
		rtBook->offerswapperday = quot->foworddata[Tnrindex].donedayofferswap* mstr->swappmul;
		rtBook->onedayMarkup[nMarkupType].donedaybidMarkup = quot->foworddata[Tnrindex].onedayMarkup[nMarkupType].donedaybidMarkup*mstr->swappmul;        // bid마크업 1일환산
		rtBook->onedayMarkup[nMarkupType].donedayofferMarkup = quot->foworddata[Tnrindex].onedayMarkup[nMarkupType].donedayofferMarkup* mstr->swappmul;      // offer마크업 1일환산
		if (nMarkupType == MARKUPINDEXALL)
		{
			rtBook->onedayMarkup[nMarkupType].donedaybidMarkup = 0;
			rtBook->onedayMarkup[nMarkupType].donedayofferMarkup = 0;
		}
		//  mds_log(&m, LOG_DEBUG, "[%-15s][%d] donedaybidMarkup='%f'", __FUNCTION__, __LINE__, quot->foworddata[Tnrindex].onedayMarkup[nMarkupType].donedaybidMarkup);
		//  mds_log(&m, LOG_DEBUG, "[%-15s][%d] donedayofferMarkup='%f'", __FUNCTION__, __LINE__, quot->foworddata[Tnrindex].onedayMarkup[nMarkupType].donedayofferMarkup);
		rtBook->OrderAbleStat = SOURCE_STOP;
		//  mds_log(&m, LOG_DEBUG, "[%-15s][%d] quot->foworddata[Tnrindex].donedaybidswap=[%f] ", __FUNCTION__, __LINE__, quot->foworddata[Tnrindex].donedaybidswap);
		//  mds_log(&m, LOG_DEBUG, "[%-15s][%d] quot->foworddata[Tnrindex].donedaybidMarkup=[%f] ", __FUNCTION__, __LINE__, quot->foworddata[Tnrindex].onedayMarkup[nMarkupType].donedaybidMarkup);
		//  mds_log(&m, LOG_DEBUG, "[%-15s][%d] quot->foworddata[Tnrindex].donedayaskswap=[%f] ", __FUNCTION__, __LINE__, quot->foworddata[Tnrindex].donedayofferswap);
		//  mds_log(&m, LOG_DEBUG, "[%-15s][%d] quot->foworddata[Tnrindex].donedayaskMarkup=[%f] ", __FUNCTION__, __LINE__, quot->foworddata[Tnrindex].onedayMarkup[nMarkupType].donedayofferMarkup);

	}
	sprintf(rtBook->QuotimeStamp, "%-30d", quot->xhms);
	int ibidzeroCount = 0, iaskzeroCount = 0;
	double dCask = book->spotdata.cask;
	double dCbid = book->spotdata.cbid;

	rtBook->bidSpotPrice = book->spotdata.bid[0].pbid + rtBook->bidSpotMarkup;
	rtBook->askSpotPrice = book->spotdata.ask[0].pask + rtBook->askSpotMarkup;

	rtBook->bidFowardPoint = (b - rtBook->bidSpotPrice);
	rtBook->askFowardPoint = (a - rtBook->askSpotPrice);
	rtBook->swapAdjestValue = mstr->swappmul;
	//  rtBook->dTickSize        =   dpinc;      //가격 최소단위
	rtBook->CustPind = mstr->zCustdiv;      //고객 소수점자리수
	if ((memcmp(esym, "MARKRW", 6) == 0) || dateType[0] == '3' || dateType[0] == '6')
		rtBook->EmpPind = mstr->zdiv;      //직원 소수점자리수.
	else
		rtBook->EmpPind = mstr->swapzdiv;      //직원 스왑소수점자리수. 

	//  mds_log(&m, LOG_DEBUG, "[%-15s][%d] Tnrindex ='%d'", __FUNCTION__, __LINE__, Tnrindex);
	// mds_log(&m, LOG_DEBUG, "[%-15s][%d] nMarkupType ='%d'", __FUNCTION__, __LINE__, nMarkupType);
	// mds_log(&m, LOG_DEBUG, "[%-15s][%d] Basediff ='%d'", __FUNCTION__, __LINE__, Basediff);
	// mds_log(&m, LOG_DEBUG, "[%-15s][%d] Bid1호가 ='%f'", __FUNCTION__, __LINE__, b);
	// mds_log(&m, LOG_DEBUG, "[%-15s][%d] Ask1호가='%f'", __FUNCTION__, __LINE__, a);
	// mds_log(&m, LOG_DEBUG, "[%-15s][%d] Bid ='%f'", __FUNCTION__, __LINE__, book->foworddata[Tnrindex].bid[0].pbid);
	// mds_log(&m, LOG_DEBUG, "[%-15s][%d] Ask ='%f'", __FUNCTION__, __LINE__, book->foworddata[Tnrindex].ask[0].pask);
	// mds_log(&m, LOG_DEBUG, "[%-15s][%d] rtBook->bidMarkup='%f'", __FUNCTION__, __LINE__, rtBook->bidMarkup);
	// mds_log(&m, LOG_DEBUG, "[%-15s][%d] rtBook->askMarkup='%f'", __FUNCTION__, __LINE__, rtBook->askMarkup);
	// mds_log(&m, LOG_DEBUG, "[%-15s][%d] rtBook->bidSpotMarkup='%f'", __FUNCTION__, __LINE__, rtBook->bidSpotMarkup);
	// mds_log(&m, LOG_DEBUG, "[%-15s][%d] rtBook->askSpotMarkup='%f'", __FUNCTION__, __LINE__, rtBook->askSpotMarkup);
	// mds_log(&m, LOG_DEBUG, "[%-15s][%d] quot->markupdata[Tnrindex].markupSet[nMarkupType].bidMarkup='%f'", __FUNCTION__, __LINE__, quot->markupdata[Tnrindex].markupSet[nMarkupType].bidMarkup);
	// mds_log(&m, LOG_DEBUG, "[%-15s][%d] quot->markupdata[Tnrindex].markupSet[nMarkupType].offerMarkup='%f'", __FUNCTION__, __LINE__, quot->markupdata[Tnrindex].markupSet[nMarkupType].offerMarkup);
	// mds_log(&m, LOG_DEBUG, "[%-15s][%d] quot->foworddata[Tnrindex].donedaybidswap ='%f'", __FUNCTION__, __LINE__, quot->foworddata[Tnrindex].donedaybidswap );
	// mds_log(&m, LOG_DEBUG, "[%-15s][%d] quot->foworddata[Tnrindex].donedayofferswap='%f'", __FUNCTION__, __LINE__, quot->foworddata[Tnrindex].donedayofferswap);
	// mds_log(&m, LOG_DEBUG, "[%-15s][%d]bidFowardPoint ='%f'", __FUNCTION__, __LINE__,  rtBook->bidFowardPoint);
	// mds_log(&m, LOG_DEBUG, "[%-15s][%d] rtBook->askFowardPoint='%f'", __FUNCTION__, __LINE__, rtBook->askFowardPoint);


	  //가상잔량용 호가 생성 
	for (ii = 0; ii < BOOK_LEVEL; ii++)
	{

		if (book->spotdata.bid[ii].vbid - dCbid < 0.00000001)
		{
			rtBook->bid[ii].spotbid = book->spotdata.bid[0].pbid - (dpinc* ii) + (quot->markupdata[INDEXSPOT].markupSet[nMarkupType].bidMarkup* mstr->swappmul);
			rtBook->bid[ii].pbid = b - (dpinc* ii);
			rtBook->bid[ii].vbid = folder->mstr.dBidBaseAmount;
		}
		else
		{

			rtBook->bid[ii].spotbid = book->spotdata.bid[0].pbid - (dpinc* ii) + (quot->markupdata[INDEXSPOT].markupSet[nMarkupType].bidMarkup* mstr->swappmul);
			rtBook->bid[ii].pbid = b - (dpinc* ii);
			rtBook->bid[ii].vbid = book->spotdata.bid[ii].vbid - dCbid;
		}

		if (book->spotdata.ask[ii].vask - dCask < 0.00000001)
		{
			rtBook->ask[ii].spotask = book->spotdata.ask[0].pask + (dpinc* ii) + (quot->markupdata[INDEXSPOT].markupSet[nMarkupType].offerMarkup *mstr->swappmul);
			rtBook->ask[ii].pask = a + (dpinc* ii);
			rtBook->ask[ii].vask = folder->mstr.dAskBaseAmount;
			//  dCask = dCask - book->spotdata.ask[ii].vask;
		}
		else
		{
			rtBook->ask[ii].spotask = book->spotdata.ask[0].pask + (dpinc* ii) + (quot->markupdata[INDEXSPOT].markupSet[nMarkupType].offerMarkup *mstr->swappmul);
			rtBook->ask[ii].pask = a + (dpinc* ii);
			rtBook->ask[ii].vask = book->spotdata.ask[ii].vask - dCask;
			//  dCask = dCask - book->spotdata.ask[ii].vask;
		}

		dCbid = dCbid - book->spotdata.bid[ii].vbid;
		if (dCbid < 0.0000001)
			dCbid = 0;
		dCask = dCask - book->spotdata.ask[ii].vask;
		if (dCask < 0.0000001)
			dCask = 0;

	}
	//  mds_log(&m, LOG_DEBUG, "[%-15s][%d] book->spotdata.bid[0].pbid='%f'", __FUNCTION__, __LINE__, b);
	//  mds_log(&m, LOG_DEBUG, "[%-15s][%d] book->spotdata.ask[0].pask='%f'", __FUNCTION__, __LINE__, a);  
	  //MARKRW종목은 마환율이라 Spot환율에 USDKRW의 환율을 넣어주어야 한다. 
	if (memcmp(esym, "MARKRW", 6) == 0)
	{
		rtBook->bidSpotPrice = quot->usdkrwbidpirce;
		rtBook->askSpotPrice = quot->usdkreaskprice;
	}

	//최초시세 미수신 종목 
	if (mstr->trdf != 3)
	{
		//   mds_log(&m, LOG_MUST, "[%-15s][%4d] 당일 시세가 수신되지 않았습니다. [%s] ", __FUNCTION__, __LINE__, esym);  로그다량출력으로 삭제 
		return -10;
	}
	//시세 중지중인 종목 
	if (mstr->pricestat == SOURCE_STOP - 1) //시세정지
	{
		mds_log(&m, LOG_MUST, "[%-15s][%4d] 시세원천이 중지 중입니다. [%s] ", __FUNCTION__, __LINE__, esym);
		return -11;
	}


	return 0;
}




/******************************************************************************
* Function Name : GetBookToDateNMarkup(char *symbol, char* Expiredate, BOOKSTRUCT* rtBook,int nMarkupType)
* Description   : 입력일자의 시세정보를 rtBook에 넣어줍니다. 마크업 미포함 시세입니다. 
******************************************************************************/
int GetBookToDateNMarkup(char *symbol, char* Expiredate, BOOKSTRUCT* rtBook ,int nMarkupType)
{
  return GetBookToDateNMarkupType(symbol, Expiredate, rtBook ,nMarkupType, nMarkupType,9999);
}
/******************************************************************************
* Function Name : GetBookToDateNMarkupType(char *symbol, char* Expiredate, BOOKSTRUCT* rtBook,int nMarkupType, int nProductdstcd)
* Description   : 입력일자의 시세정보를 rtBook에 넣어줍니다. 마크업 미포함 시세입니다. 
******************************************************************************/
int GetBookToDateNMarkupType(char *symbol, char* Expiredate, BOOKSTRUCT* rtBook ,int nMarkupType, int fx_prdct_dstcd)
{
  //내부거래
  //마크업 그룹을 가지고 온다
  if (nMarkupType == MARKUPEMP) //직원
    nMarkupType = MARKUPINDEXEMP;
  else if (nMarkupType == MARKUPPRI)//개인
    nMarkupType = MARKUPINDEXPRI;
  else if (nMarkupType == MARKUPENT) //기업
    nMarkupType = MARKUPINDEXENT;
  else if (nMarkupType == MARKUPENT2) //기관
    nMarkupType = MARKUPINDEXENT2;  
  MARKET  *market, m;
  MDFOLD  *folder;
  
  char  esym[24], exnm[20];
  int    len;
  memset(&m, 0, sizeof(MARKET));
  sprintf(m.procname, "Custom");
  
  STR2S(esym, symbol);
  esym[6] = 0x00;
  STR2S(exnm, "CMBS");
  
  folder = pibo_getfolder2(exnm, esym, &market); 
  if (market == NULL)
  {
    mds_log(&m, LOG_MUST, "Exchange is not found.  Exchange='%s'", exnm);
    return -2;
  }
  
  if (folder == NULL)
  {
    mds_log(&m, LOG_MUST, "[%-15s][%4d]Symbol is not found. esym='%s'  for '%s'  ", __FUNCTION__, __LINE__, esym, exnm);
    return -3;
  }

  MDMSTR  *mstr;
  MDQUOT  *quot;
  MDBOOK  *book;
  
  char    form[16], value[16];
  char    pind;
  int     price;
  int     diff, rate;
  int     rc;
  int    ii;
  double  dval;
  double  a=0.00000000,b=0.00000000;
  mstr = &folder->mstr;
  quot = &folder->quot;
  book = &folder->book;
  char   cSpotday[8];
  double dpinc = mstr->pinc *10;
  
  //sprintf(cSpotday, "%08d" ,folder->quot.kymd);
  sprintf(cSpotday, "%08d" ,atoi(mstr->expiredateSpot));
  
  int diffDate = GetDiffDate(cSpotday ,Expiredate);
  int Basediff =0; //기준환율과 일자차이  
  int Tnrindex = 0;

  rtBook->bidSpotMarkup   =  quot->markupdata[INDEXSPOT].markupSet[nMarkupType].bidMarkup* mstr->swappmul;//bidSpotMarkup
  rtBook->askSpotMarkup  =  quot->markupdata[INDEXSPOT].markupSet[nMarkupType].offerMarkup *mstr->swappmul;  //askSpotMarkup    
  


//atoi(Expiredate) == atoi(mstr->expiredateSpot)  ||
  if ( (memcmp(esym, "MARKRW",6)  == 0) || fx_prdct_dstcd == 3|| fx_prdct_dstcd == 6)  //spot시세 MAR는 무조건 spot 
  {

    rtBook->bidMarkup =  rtBook->bidSpotMarkup  ;//quot->markupdata[INDEXSPOT].markupSet[nMarkupType].bidMarkup* mstr->swappmul;
    rtBook->askMarkup = rtBook->askSpotMarkup;//quot->markupdata[INDEXSPOT].markupSet[nMarkupType].offerMarkup *mstr->swappmul;  
    b = book->spotdata.bid[0].pbid;// + rtBook->bidSpotMarkup;
    a = book->spotdata.ask[0].pask;// + rtBook->askSpotMarkup;
    char Temp[16];      
    sprintf(Temp, "%d", 0);  
    sprintf(rtBook->difftnrday, "%s", Temp);  
    rtBook->bidswapperday = 0;
    rtBook->offerswapperday = 0;
    sprintf(rtBook->basesubcode, "%3s", subCodeSpot[0]);    

  }
  else    //Foword시세 
  {
    if ( fx_prdct_dstcd == 1)  //Today시세
    {
      Basediff = 0; 
      Tnrindex = 7;
    }
    else if (fx_prdct_dstcd == 2)  //Tom시세
    {
      Basediff = 0; 
      Tnrindex = 8;
    }
    else if (mstr->expireNday1W >= diffDate)   //1Week 시세 
    {
      Basediff = mstr->expireNday1W - diffDate; 
      Tnrindex = 0;
    }
    else if (mstr->expireNday1M >= diffDate) 
    {
      Basediff =  mstr->expireNday1M - diffDate; 
      Tnrindex = 1;
    }
    else if (mstr->expireNday2M >= diffDate) 
    {
      Basediff = mstr->expireNday2M - diffDate; 
      Tnrindex = 2;
    }
    else if (mstr->expireNday3M >= diffDate) 
    {
      Basediff = mstr->expireNday3M - diffDate; 
      Tnrindex = 3;
    }
    else if (mstr->expireNday6M >= diffDate) 
    {
      Basediff = mstr->expireNday6M - diffDate; 
      Tnrindex = 4;
    }
    else if (mstr->expireNday9M >= diffDate)  
    {
      Basediff = mstr->expireNday9M - diffDate; 
      Tnrindex = 5;
    }
    else if (mstr->expireNday12M >= diffDate) 
    {
      Basediff = mstr->expireNday12M - diffDate; 
      Tnrindex = 6;
    }
    else if  (mstr->expireNday12M < diffDate) 
      return -5;  //가능기간 초과 ..

    //마크업 = (테너마크업 *pip단위) - (테너1일마크업*일수*pip단위)
    if (nMarkupType == MARKUPINDEXALL)
    {
      rtBook->bidMarkup = 0;
      rtBook->askMarkup = 0;
    }
    else
    {
      mds_log(&m, LOG_DEBUG, "[%-15s][%d] donedaybidMarkup='%f' '%f'", __FUNCTION__, __LINE__, quot->markupdata[Tnrindex].markupSet[nMarkupType].bidMarkup ,  mstr->swappmul);
      rtBook->bidMarkup = ( quot->markupdata[Tnrindex].markupSet[nMarkupType].bidMarkup    * mstr->swappmul ) - (quot->foworddata[Tnrindex].onedayMarkup[nMarkupType].donedaybidMarkup* Basediff* mstr->swappmul);//+ rtBook->bidSpotMarkup;
      rtBook->askMarkup = ( quot->markupdata[Tnrindex].markupSet[nMarkupType].offerMarkup  * mstr->swappmul ) - (quot->foworddata[Tnrindex].onedayMarkup[nMarkupType].donedayofferMarkup* Basediff* mstr->swappmul);//+ rtBook->askSpotMarkup  ;  
    }
    
    //금액 = 테너가격 - (1일스왑 *일수)    #제거  + 마크업 + SPot마크업
    b = book->foworddata[Tnrindex].bid[0].pbid - (quot->foworddata[Tnrindex].donedaybidswap * Basediff* mstr->swappmul) ; //  + rtBook->bidMarkup + rtBook->bidSpotMarkup  ;
    a = book->foworddata[Tnrindex].ask[0].pask - (quot->foworddata[Tnrindex].donedayofferswap * Basediff* mstr->swappmul); //+ rtBook->askMarkup + rtBook->askSpotMarkup  ;  
  
    char Temp[16];  
    sprintf(Temp, "%d", Basediff);  
    sprintf(rtBook->basesubcode, "%3s", subFowardode[Tnrindex]);  
    sprintf(rtBook->difftnrday, "%4s", Temp);      
    rtBook->bidswapperday = quot->foworddata[Tnrindex].donedaybidswap * mstr->swappmul;
    rtBook->offerswapperday = quot->foworddata[Tnrindex].donedayofferswap* mstr->swappmul;
    rtBook->onedayMarkup[nMarkupType].donedaybidMarkup = quot->foworddata[Tnrindex].onedayMarkup[nMarkupType].donedaybidMarkup*mstr->swappmul;        // bid마크업 1일환산
    rtBook->onedayMarkup[nMarkupType].donedayofferMarkup = quot->foworddata[Tnrindex].onedayMarkup[nMarkupType].donedayofferMarkup* mstr->swappmul;      // offer마크업 1일환산
    if (nMarkupType ==MARKUPINDEXALL)
    {
      rtBook->onedayMarkup[nMarkupType].donedaybidMarkup   = 0;
      rtBook->onedayMarkup[nMarkupType].donedayofferMarkup = 0;
    }
    mds_log(&m, LOG_DEBUG, "[%-15s][%d] donedaybidMarkup='%f'", __FUNCTION__, __LINE__, quot->foworddata[Tnrindex].onedayMarkup[nMarkupType].donedaybidMarkup);
    mds_log(&m, LOG_DEBUG, "[%-15s][%d] donedayofferMarkup='%f'", __FUNCTION__, __LINE__, quot->foworddata[Tnrindex].onedayMarkup[nMarkupType].donedayofferMarkup);
    rtBook->OrderAbleStat = SOURCE_STOP;  
  }
  sprintf(rtBook->QuotimeStamp, "%-30d", quot->xhms);    
  int ibidzeroCount = 0, iaskzeroCount = 0;
  double dCask = book->spotdata.cask;
  double dCbid = book->spotdata.cbid;
  rtBook->bidSpotPrice  = book->spotdata.bid[0].pbid ;
  rtBook->askSpotPrice  = book->spotdata.ask[0].pask ;
  


  rtBook->bidFowardPoint = (b - rtBook->bidSpotPrice) / mstr->swappmul;
  rtBook->askFowardPoint = (a - rtBook->askSpotPrice) / mstr->swappmul;
  rtBook->swapAdjestValue = mstr->swappmul;
//  rtBook->dTickSize        =   dpinc;      //가격 최소단위
  rtBook->CustPind    = mstr->zCustdiv;      //고객 소수점자리수
  rtBook->EmpPind      = mstr->zdiv    ;      //직원 소수점자리수.
  
  rtBook->bidopenprice  = quot->spotdata.bidopen;    // BIDopen price
  rtBook->bidhighprice  = quot->spotdata.bidhigh;    // BIDhigh price
  rtBook->bidlowprice    = quot->spotdata.bidlow;  // BIDlow price  
  rtBook->askopenprice  = quot->spotdata.offeropen;      // ASKopen price
  rtBook->askhighprice  = quot->spotdata.offerhigh;      // ASKhigh price
  rtBook->asklowprice    = quot->spotdata.offerlow;    // ASKlow price  
  mds_log(&m, LOG_DEBUG, "[%-15s][%d] Tnrindex=[%d] ", __FUNCTION__, __LINE__, Tnrindex);
  

  mds_log(&m, LOG_DEBUG, "[%-15s][%d] book->spotdata.bid[0].pbid='%f'", __FUNCTION__, __LINE__, book->spotdata.bid[0].pbid);
  mds_log(&m, LOG_DEBUG, "[%-15s][%d] book->spotdata.ask[0].pask='%f'", __FUNCTION__, __LINE__, book->spotdata.ask[0].pask);

  for (ii = 0; ii < BOOK_LEVEL; ii++)
  {
    if (book->spotdata.bid[ii].vbid -  dCbid < 0.00000001)
    {
      rtBook->bid[ii].spotbid  = book->spotdata.bid[0].pbid   - (dpinc* ii);//+ (quot->markupdata[INDEXSPOT].markupSet[nMarkupType].bidMarkup* mstr->swappmul) ;
      rtBook->bid[ii].pbid      = b - (dpinc* ii);
      rtBook->bid[ii].vbid     = folder->mstr.dBidBaseAmount;
    }else
    {
      
      rtBook->bid[ii].spotbid  = book->spotdata.bid[0].pbid   - (dpinc* ii);//+ (quot->markupdata[INDEXSPOT].markupSet[nMarkupType].bidMarkup* mstr->swappmul)
      rtBook->bid[ii].pbid      = b - (dpinc* ii);
      rtBook->bid[ii].vbid     = book->spotdata.bid[ii].vbid -  dCbid ;      
    }

    if (book->spotdata.ask[ii].vask -  dCask  < 0.00000001)
    {
      rtBook->ask[ii].spotask  = book->spotdata.ask[0].pask+ (dpinc* ii);// +(quot->markupdata[INDEXSPOT].markupSet[nMarkupType].offerMarkup *mstr->swappmul)    
      rtBook->ask[ii].pask = a + (dpinc* ii);    
      rtBook->ask[ii].vask = folder->mstr.dAskBaseAmount;
    //  dCask = dCask - book->spotdata.ask[ii].vask;
    }else
    {
      rtBook->ask[ii].spotask  = book->spotdata.ask[0].pask+ (dpinc* ii);  // +(quot->markupdata[INDEXSPOT].markupSet[nMarkupType].offerMarkup *mstr->swappmul)  
      rtBook->ask[ii].pask = a + (dpinc* ii);    
      rtBook->ask[ii].vask = book->spotdata.ask[ii].vask -  dCask;
    //  dCask = dCask - book->spotdata.ask[ii].vask;
    }
    
    dCbid = dCbid - book->spotdata.bid[ii].vbid;
    if (dCbid< 0.0000001)
      dCbid = 0;
    dCask = dCask - book->spotdata.ask[ii].vask;
    if (dCask< 0.0000001)
      dCask = 0;
      
  } 
  //MARKRW종목은 마환율이라 Spot환율에 USDKRW의 환율을 넣어주어야 한다. 
  if (memcmp(esym, "MARKRW",6)  == 0)
  {
    rtBook->bidSpotPrice     =  quot->usdkrwbidpirce ;
    rtBook->askSpotPrice     =  quot->usdkreaskprice ;  

  }  

  return 0;
}
  
  
/******************************************************************************
* Function Name : GetSwapPriceToDate(char *symbol, char* NearExpiredate, char* FarExpiredate,  char* sLgenNo, QUOTMSTRUCT* rtQuot, int nMarkupType)
* Description   : 입력일자의 Swap가격 정보를 가지고 오는 함수 rtQuot에 넣어줍니다.  
******************************************************************************/
int GetSwapPriceToDate(char *symbol, char* NearExpiredate, char* FarExpiredate,  char* sLgenNo, QUOTMSTRUCT* rtQuot, int nMarkupType)
{
  MARKET  *market, m;
  MDFOLD  *folder;
  char  esym[24], exnm[20];
  int    len;
  char   o_NearExpiredateType[2];
  char    o_FarExpiredateType[2];  
  MDMSTR  *mstr;
  MDQUOT  *quot;
  char    form[16], value[16];
  char    pind;
  int     price;
  int     diff, rate;
  int     rc;
  int    ii;
  double  dval;
  char   s_base_cncycd[3+1];
  char   s_cnprt_cncycd[3+1];
  char   sToday[8+1];
  char  v_USDPrdctCd[6+1];
  char      o_pyacc_ymd[ 8+1];

      
  memset(&m, 0, sizeof(MARKET));
  sprintf(m.procname, "Custom");
  STR2S(esym, symbol);
  esym[6] = 0x00;
  STR2S(exnm, "CMBS");
  folder = pibo_getfolder2(exnm, esym, &market);
  if (market == NULL)
  {
    mds_log(&m, LOG_MUST, "[%-15s][%d] Exchange is not found.  Exchange='%s'", __FUNCTION__, __LINE__, exnm);
    return(-1);
  }

  if (folder == NULL)
  {
    mds_log(&m, LOG_MUST, "[%-15s][%d] Symbol is not found.  esym='%s' for '%s' ", __FUNCTION__, __LINE__, esym, exnm);
    return(-2);
  }
  
  mstr = &folder->mstr;
  quot = &folder->quot;
  if (nMarkupType > 10) // 조회는 10보다 작은값 OMS는 10보다 큰값이 온다. 조회는 통과 OMS는 체크
  {
    if (mstr->trdf != 3)
    {
      mds_log(&m, LOG_MUST, "[%-15s][%4d] 당일 시세가 수신되지 않았습니다. [%s] ", __FUNCTION__, __LINE__, esym);
      return -10;      
    }
    //시세 중지중인 종목 
    if (mstr->pricestat == SOURCE_STOP-1) //시세정지
    {
      mds_log(&m, LOG_MUST, "[%-15s][%4d] 시세원천이 중지 중입니다. [%s] ", __FUNCTION__, __LINE__, esym);
      return -11;      
    }  
  }
  
  //내부거래
  //마크업 그룹을 가지고 온다
  if (nMarkupType == MARKUPEMP) //직원
    nMarkupType = MARKUPINDEXEMP;
  else if (nMarkupType == MARKUPPRI)//개인
    nMarkupType = MARKUPINDEXPRI;
  else if (nMarkupType == MARKUPENT) //기업
    nMarkupType = MARKUPINDEXENT;
  else if (nMarkupType == MARKUPENT2) //기관
    nMarkupType = MARKUPINDEXENT2;  
    
    
  char   cSpotday[8];
  sprintf(cSpotday, "%08d" ,atoi(mstr->expiredateSpot));
  char v_symbol[6+1];
  char v_NearExpiredate[8+1];
  char v_FarExpiredate[8+1];
  char v_sLgenNo[10+1];
  
  memset(v_symbol , 0x00, sizeof(v_symbol ));
  memset(v_NearExpiredate    , 0x00, sizeof(v_NearExpiredate    ));
  memset(v_FarExpiredate    , 0x00, sizeof(v_FarExpiredate    ));
  memset(v_sLgenNo    , 0x00, sizeof(v_sLgenNo    ));
  
  memcpy(v_symbol , symbol , sizeof(v_symbol )-1);
  memcpy(v_NearExpiredate , NearExpiredate , sizeof(v_NearExpiredate )-1);
  memcpy(v_FarExpiredate , FarExpiredate , sizeof(v_FarExpiredate )-1);
  memcpy(v_sLgenNo , sLgenNo , sizeof(v_sLgenNo )-1);
  l_rtrim(v_sLgenNo);

  if (strlen(v_sLgenNo) ==0 )
  {  
    if (nMarkupType == 0)
    {
      sprintf(v_sLgenNo,"%s","9999999");
    }
    else
    {  
      mds_log(&m, LOG_MUST, "[%-15s][%d] v_sLgenNo is NULL ",  __FUNCTION__, __LINE__);  
      return -4;
    }
  }
  mds_log(&m, LOG_DEBUG, "[%-15s][%d] nMarkupType [%d] ",  __FUNCTION__, __LINE__,nMarkupType);  
  mds_log(&m, LOG_DEBUG, "[%-15s][%d] v_sLgenNo [%s] ",  __FUNCTION__, __LINE__,v_sLgenNo);  
  //스왑조회 함수 호출로 결과를 받아온다. 
  //fsfxwin/mds/src/lib/mds/mdsrdb.sqc
  rc = f_GetSwapPriceToDate(v_symbol, v_NearExpiredate, v_FarExpiredate,  v_sLgenNo, rtQuot, nMarkupType,m);
  if (rc!= 0)
  {
    mds_log(&m, LOG_DEBUG, "[%-15s][%d] f_GetSwapPriceToDate 실패 [%s] [%d]", __FUNCTION__, __LINE__,symbol,rc);  
    
    return (-3);
  }
  
  
  return 0;

}


/******************************************************************************
* Function Name : GetPriceToDate(char* sbGubn, char *symbol, char* Expiredate)
* Description   : 입력일자의 시세정보를 반환합니다. oms나 서비스에서 현재 가격을 얻을때 사용합니다. 현재 사용(X)
******************************************************************************/
double GetPriceToDate(char* sbGubn, char *symbol, char* Expiredate)
{
  MARKET  *market, m;
  MDFOLD  *folder;
  char  esym[24], exnm[20];
  int    len;
  memset(&m, 0, sizeof(MARKET));
  sprintf(m.procname, "Custom");
  STR2S(esym, symbol);
  esym[6] = 0x00;
  STR2S(exnm, "CMBS");

  folder = pibo_getfolder2(exnm, esym, &market);
  if (market == NULL)
  {
    //  sprintf(content->errm, "[%s]거래소를 찾을 수 없습니다.", exnm);
    mds_log(&m, LOG_MUST, "Exchange is not found.  Exchange='%s'", exnm);
    return(-1);
  }

  if (folder == NULL)
  {
  //  sprintf(content->errm, "[%s]종목을 찾을 수 없습니다.", isym);
    mds_log(&m, LOG_MUST, "[%-15s][%4d]Symbol is not found. esym='%s'  for '%s'  ", __FUNCTION__, __LINE__, esym, exnm);
    return(-1);
  }
  
  MDMSTR  *mstr;
  MDQUOT  *quot;
  char    form[16], value[16];
  char    pind;
  int     price;
  int     diff, rate;
  int     rc;
  int    ii;
  double  dval;
  //struct q_price QuotData; 
  
  mstr = &folder->mstr;
  quot = &folder->quot;
  char   cSpotday[8];
  
  
 // sprintf(cSpotday, "%08d" ,folder->quot.kymd);
  sprintf(cSpotday, "%08d" ,atoi(mstr->expiredateSpot));
  int diffDate = GetDiffDate(cSpotday ,Expiredate);
  //mds_log(&m, LOG_MUST, "diffdate = %d", diffDate);  
  int Basediff =0; //기준환율과 일자차이  
  int Tnrindex = 0;
  
  

  if (atoi(Expiredate) == atoi(mstr->expiredateSpot))  //spot시세 
  {
    if (sbGubn[0] == '1') //매수 
      return quot->spotdata.bidlast;
    else if (sbGubn[0] == '2') //매도 
      return quot->spotdata.offerlast;

  }
  else if (atoi(Expiredate) == atoi(mstr->expireToday))  //Today시세
  {
    Basediff = 0; 
    Tnrindex = 7;
  }
  else if (atoi(Expiredate) == atoi(mstr->expireTom))  //Tom시세
  {
    Basediff = 0; 
    Tnrindex = 8;
  }
  else if (mstr->expireNday1W >= diffDate)   //1Week 시세 
  {
    Basediff = mstr->expireNday1W - diffDate; 
    Tnrindex = 0;
  }
  else if (mstr->expireNday1M > diffDate) 
  {
    Basediff =  mstr->expireNday1M - diffDate; 
    Tnrindex = 1;
  }
  else if (mstr->expireNday2M > diffDate) 
  {
    Basediff = mstr->expireNday2M - diffDate; 
    Tnrindex = 2;
  }
  else if (mstr->expireNday3M > diffDate) 
  {
    Basediff = mstr->expireNday3M - diffDate; 
    Tnrindex = 3;
  }
  else if (mstr->expireNday6M > diffDate) 
  {
    Basediff = mstr->expireNday6M - diffDate; 
    Tnrindex = 4;
  }
  else if (mstr->expireNday9M > diffDate)  
  {
    Basediff = mstr->expireNday9M - diffDate; 
    Tnrindex = 5;
  }
  else if (mstr->expireNday12M > diffDate) 
  {
    Basediff = mstr->expireNday12M - diffDate; 
    Tnrindex = 6;
  }
  else if  (mstr->expireNday12M < diffDate) 
    return -1;  //가능기간 초과 ..
    

  
  if (sbGubn[0] == '2') //매수 
    return quot->foworddata[Tnrindex].bidlast - (quot->foworddata[Tnrindex].donedaybidswap * Basediff* mstr->swappmul);
  else if (sbGubn[0] == '1') //매도 
    return quot->foworddata[Tnrindex].offerlast - (quot->foworddata[Tnrindex].donedayofferswap * Basediff* mstr->swappmul);
    
  


}



/******************************************************************************
* Function Name : GetQuotToDate(char *symbol, char* Expiredate, S_SENDQUOT* rtQuot)
* Description   : SPOT시세를 rtQuot에 반환합니다.  
******************************************************************************/
int GetQuotToDate(char *symbol, char* Expiredate, S_SENDQUOT* rtQuot)
{
	MARKET  *market, m;
	MDFOLD  *folder;


	char  esym[24], exnm[20];
	int    len;
	memset(&m, 0, sizeof(MARKET));
	sprintf(m.procname, "Custom");

	STR2S(esym, symbol);
	esym[6] = 0x00;
	STR2S(exnm, "CMBS");

	folder = pibo_getfolder2(exnm, esym, &market);
	if (market == NULL)
	{
		mds_log(&m, LOG_MUST, "Exchange is not found.  Exchange='%s'", exnm);
		return -2;
	}

	if (folder == NULL)
	{
		mds_log(&m, LOG_MUST, "[%-15s][%4d]Symbol is not found. esym='%s'  for '%s'  ", __FUNCTION__, __LINE__, esym, exnm);
		return -3;
	}

	MDMSTR  *mstr;
	MDQUOT  *quot;
	MDBOOK  *book;

	char    form[16], value[16];

	char    pind;
	int     price;
	int     diff, rate;
	int     rc;
	int    ii;
	double  dval;
	double  a, b;
	mstr = &folder->mstr;
	quot = &folder->quot;
	book = &folder->book;
	char   cSpotday[8];


	// sprintf(cSpotday, "%08d" ,folder->quot.kymd);
	sprintf(cSpotday, "%08d", atoi(mstr->expiredateSpot));
	int diffDate = GetDiffDate(cSpotday, Expiredate);
	int Basediff = 0; //기준환율과 일자차이  
	int Tnrindex = 0;


	sprintf(rtQuot->symb, "%s", symbol);
	rtQuot->sseq = quot->sseq;
	rtQuot->seqn = quot->seqn;
	rtQuot->tymd = quot->tymd;
	rtQuot->xymd = quot->xymd;
	rtQuot->xhms = quot->xhms;
	rtQuot->kymd = quot->kymd;
	rtQuot->khms = quot->kymd;
	memcpy(&rtQuot->pricedata, &quot->spotdata, sizeof(quot->spotdata));
	return 0;
}



/******************************************************************************
* Function Name : GetCodePind(char *symbol)
* Description   : 종목 소수점 자리수 정보 조회 
******************************************************************************/
int  GetCodePind(char *symbol)
{
	MARKET  *market, m;
	MDFOLD  *folder;
	char  esym[24], exnm[20];
	int    len;
	memset(&m, 0, sizeof(MARKET));
	sprintf(m.procname, "Custom");
	STR2S(esym, symbol);
	esym[6] = 0x00;
	STR2S(exnm, "CMBS");

	folder = pibo_getfolder2(exnm, esym, &market);
	if (market == NULL)
	{
		mds_log(&m, LOG_ERROR, "Exchange is not found.  Exchange='%s'", exnm);
		return 0;
	}

	if (folder == NULL)
	{
		//  sprintf(content->errm, "[%s]종목을 찾을 수 없습니다.", isym);
		mds_log(&m, LOG_ERROR, "Symbol is not found.  esym='%s' for '%s' ", esym, exnm);
		return 0;
	}


	return folder->mstr.zdiv;

}

/******************************************************************************
* Function Name : GetCodeInfo(char *symbol, S_CODEINFO* sCodeinfo)
* Description   : 종목 기본 정보 조회 
******************************************************************************/
int  GetCodeInfo(char *symbol, S_CODEINFO* sCodeinfo)
{
	MARKET  *market, m;
	MDFOLD  *folder;
	char  esym[24], exnm[20];
	int    len;
	memset(&m, 0, sizeof(MARKET));
	sprintf(m.procname, "Custom");
	//CNH->CNY로 변경 
	if (memcmp(symbol, "USDCNH", 6) == 0)
	{
		STR2S(esym, "USDCNY");
	}
	else if (memcmp(symbol, "CNHKRW", 6) == 0)
	{
		STR2S(esym, "CNYKRW");
	}
	else
	{
		STR2S(esym, symbol);
	}
	esym[6] = 0x00;
	STR2S(exnm, "CMBS");

	folder = pibo_getfolder2(exnm, esym, &market);
	if (market == NULL)
	{
		//  sprintf(content->errm, "[%s]거래소를 찾을 수 없습니다.", exnm);
		mds_log(&m, LOG_ERROR, "Exchange is not found.  Exchange='%s'", exnm);
		return -1;
	}

	if (folder == NULL)
	{
		//  sprintf(content->errm, "[%s]종목을 찾을 수 없습니다.", isym);
		mds_log(&m, LOG_ERROR, "Symbol is not found.  esym='%s' for '%s' ", esym, exnm);
		return -2;
	}

	sCodeinfo->nSwapPind = folder->mstr.swapzdiv;

	if (memcmp(symbol, "USDKRW", 6) != 0)
	{
		sCodeinfo->nPind = folder->mstr.zdiv;
		sCodeinfo->nCpind = folder->mstr.zCustdiv;
	}
	else
	{
		sCodeinfo->nPind = folder->mstr.zdiv;
		sCodeinfo->nCpind = folder->mstr.zCustdiv;
	}

	sCodeinfo->dSwapAdjust = folder->mstr.swappmul;
	sCodeinfo->nAmountUnit = folder->mstr.nAmountUnit;
	sprintf(sCodeinfo->dispUnit, "%s", folder->mstr.dispUnit);
	sprintf(sCodeinfo->spotExpireDate, "%s", folder->mstr.expiredateSpot);
	return 0;

}





/******************************************************************************
* Function Name : CustomPriceSetting(int iSource, int iPriceType, char *symbol, char* date, char* time, char* bidPrice, char* offerPrice)
* Description   : 가격 수동 입력.  수기 SMBS 초기가격은 이곳을 통해 생성후 입력이 된다. 
******************************************************************************/
int CustomPriceSetting(int iSource, int iPriceType, char *symbol, char* date, char* time, char* bidPrice, char* offerPrice)
{
	MARKET  *market, m;
	char  exnm[40];
	int    rc;

	memset(&m, 0, sizeof(MARKET));
	sprintf(m.procname, "Custom");
	if ((symbol == NULL) || (date == NULL) || (time == NULL) || (bidPrice == NULL) || (offerPrice == NULL))
	{
		mds_log(&m, LOG_MUST, "CustomPriceSetting : 입력되지 않은 항목이 있습니다.\n");
		return -1;
	}

	if ((market = mds_open("CMBS", O_RDWR)) == NULL)
	{
		mds_log(&m, LOG_MUST, "Cannot open market '%s'\n", "Custom");
		return(-1);
	}

	if (strlen(symbol) < 6)
	{
		mds_log(&m, LOG_MUST, "SymbolCode is Too Short '%s'\n", symbol);
		return(-1);
	}


	struct cmbsquote  quote;
	memset(&quote, 0, sizeof(quote));
	quote.type = 'Q';
	sprintf(quote.symb, "%.*s", sizeof(quote.symb), symbol);
	sprintf(quote.ccy1, "%.3s", &quote.symb[0]);
	sprintf(quote.ccy2, "%.3s", &quote.symb[3]);
	sprintf(quote.date, "%.*s", sizeof(quote.date), date);
	sprintf(quote.time, "%.*s", sizeof(quote.time), time);
	sprintf(quote.cMDReqID, "%.*s", sizeof(quote.cMDReqID), "");
	sprintf(quote.bidprice, "%.*s", sizeof(quote.bidprice), bidPrice);
	sprintf(quote.bidQty, "%.*s", sizeof(quote.bidQty), "0");
	sprintf(quote.biddate, "%.*s", sizeof(quote.biddate), date);
	sprintf(quote.bidQuoteID, "%.*s", sizeof(quote.bidQuoteID), "");
	sprintf(quote.bidSettType, "%.*s", sizeof(quote.bidSettType), "");
	sprintf(quote.bidBestPx, "%.*s", sizeof(quote.bidBestPx), bidPrice);
	sprintf(quote.bidBestSize, "%.*s", sizeof(quote.bidBestSize), "0");
	sprintf(quote.offerprice, "%.*s", sizeof(quote.offerprice), offerPrice);
	sprintf(quote.offerQty, "%.*s", sizeof(quote.offerQty), "0");
	sprintf(quote.offerdate, "%.*s", sizeof(quote.offerdate), date);
	sprintf(quote.offerQuoteID, "%.*s", sizeof(quote.offerQuoteID), "");
	sprintf(quote.offerSettType, "%.*s", sizeof(quote.offerSettType), "");
	sprintf(quote.offerBestPx, "%.*s", sizeof(quote.offerBestPx), offerPrice);
	sprintf(quote.offerBestSize, "%.*s", sizeof(quote.offerBestSize), "0");
	//  mds_log(&m, LOG_MUST, "[%-15s][%4d] CustomPriceSetting. [%s]  [%s] [%s] [%s] [%s]   ", __FUNCTION__, __LINE__,symbol, date, time,bidPrice, offerPrice);
	SendAndSavePriceData(market, iSource, &quote, sizeof(quote));
	//  mds_log(&m, LOG_MUST, "[%-15s][%4d] CustomPriceSetting. ", __FUNCTION__, __LINE__);
	mds_close(market);
	return 0;
}




/******************************************************************************
* Function Name : SwapPriceSetting(int iSource, S_SENDSWAP swapquot)
* Description   : 스왑환율을 저장한다.iSource => 1: From CMBS 2: From 수기입력
******************************************************************************/
int SwapPriceSetting(int iSource, S_SENDSWAP swapquot)
{
	MARKET  *market, mc;
	char  exnm[40];
	int    rc;
	int ii = 0;
	int count = 0;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");
	swapquot.symb[6] = 0x00;

	//if ((symbol==NULL) || (date==NULL) || (time==NULL) || (bidPrice==NULL) || (offerPrice==NULL) )
	if (strlen(swapquot.symb) == 0)
	{
		mds_log(&mc, LOG_MUST, "CustomPriceSetting : 입력되지 않은 항목이 있습니다.\n");
		return -1;
	}

	if ((market = mds_open("CMBS", O_RDWR)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "Cannot open market '%s'\n", "Custom");
		return(-2);
	}


	MDFOLD  *folder;//, *rfolder; 
	//MDFOLD  *krwfolder;
	MDQUOT  *mdquot, mdq;
	MDMSTR  *m;
	struct  q_data  qd;
	/*-------------------------
	 * data set
	 * -----------------------*/
	if ((folder = mds_getfolder(market, swapquot.symb)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "[%-15s][%4d] 등록된 종목정보가 없습니다. [%s]", __FUNCTION__, __LINE__, swapquot.symb);
		return(NULL);
	}

	//1.메모리에 값 반영. 
	mdquot = &folder->quot;
	m = &folder->mstr;
	mds_log(&mc, LOG_DEBUG, "[%-15s][%4d] 스왑환율 메모리저장. [%s]", __FUNCTION__, __LINE__, swapquot.symb);
	memcpy(&mdq, mdquot, sizeof(MDQUOT));
	m->pmul = 1;
	mdq.tymd = str2i(swapquot.date, sizeof(swapquot.date));
	mdq.seqn++;
	mdq.xymd = str2i(swapquot.date, sizeof(swapquot.date));
	mdq.xhms = str2i(swapquot.time, sizeof(swapquot.time)) * 1000;
	mdq.kymd = str2i(swapquot.date, sizeof(swapquot.date));
	mdq.khms = str2i(swapquot.time, sizeof(swapquot.time)) * 1000;

	struct q_price* q = mdq.swapdata;


	mdq.swapdata[INDEX1W].bidlast = str2f(swapquot.swapbid1W, FlotePoint);//  
	mdq.swapdata[INDEX1W].bidbest = str2f(swapquot.swapbid1W, FlotePoint);//   
	mdq.swapdata[INDEX1W].offerlast = str2f(swapquot.swapoffer1W, FlotePoint);//   
	mdq.swapdata[INDEX1W].offerbest = str2f(swapquot.swapoffer1W, FlotePoint);//   
	mdq.swapdata[INDEX1M].bidlast = str2f(swapquot.swapbid1M, FlotePoint);//   
	mdq.swapdata[INDEX1M].bidbest = str2f(swapquot.swapbid1M, FlotePoint);//   
	mdq.swapdata[INDEX1M].offerlast = str2f(swapquot.swapoffer1M, FlotePoint);//   
	mdq.swapdata[INDEX1M].offerbest = str2f(swapquot.swapoffer1M, FlotePoint);//   
	mdq.swapdata[INDEX2M].bidlast = str2f(swapquot.swapbid2M, FlotePoint);//   
	mdq.swapdata[INDEX2M].bidbest = str2f(swapquot.swapbid2M, FlotePoint);//   
	mdq.swapdata[INDEX2M].offerlast = str2f(swapquot.swapoffer2M, FlotePoint);//   
	mdq.swapdata[INDEX2M].offerbest = str2f(swapquot.swapoffer2M, FlotePoint);//   
	mdq.swapdata[INDEX3M].bidlast = str2f(swapquot.swapbid3M, FlotePoint);//   
	mdq.swapdata[INDEX3M].bidbest = str2f(swapquot.swapbid3M, FlotePoint);//   
	mdq.swapdata[INDEX3M].offerlast = str2f(swapquot.swapoffer3M, FlotePoint);//   
	mdq.swapdata[INDEX3M].offerbest = str2f(swapquot.swapoffer3M, FlotePoint);//   
	mdq.swapdata[INDEX6M].bidlast = str2f(swapquot.swapbid6M, FlotePoint);//  
	mdq.swapdata[INDEX6M].bidbest = str2f(swapquot.swapbid6M, FlotePoint);//   
	mdq.swapdata[INDEX6M].offerlast = str2f(swapquot.swapoffer6M, FlotePoint);//   
	mdq.swapdata[INDEX6M].offerbest = str2f(swapquot.swapoffer6M, FlotePoint);//   
	mdq.swapdata[INDEX9M].bidlast = str2f(swapquot.swapbid9M, FlotePoint);//   
	mdq.swapdata[INDEX9M].bidbest = str2f(swapquot.swapbid9M, FlotePoint);//   
	mdq.swapdata[INDEX9M].offerlast = str2f(swapquot.swapoffer9M, FlotePoint);//   
	mdq.swapdata[INDEX9M].offerbest = str2f(swapquot.swapoffer9M, FlotePoint);//   
	mdq.swapdata[INDEX12M].bidlast = str2f(swapquot.swapbid12M, FlotePoint);//   
	mdq.swapdata[INDEX12M].bidbest = str2f(swapquot.swapbid12M, FlotePoint);//    
	mdq.swapdata[INDEX12M].offerlast = str2f(swapquot.swapoffer12M, FlotePoint);//    
	mdq.swapdata[INDEX12M].offerbest = str2f(swapquot.swapoffer12M, FlotePoint);//    
	mdq.swapdata[INDEXTOD].bidlast = (str2f(swapquot.swapbidTOM, FlotePoint) + str2f(swapquot.swapbidSpot, FlotePoint));//    ,sizeof(swapquot.swapbid12M  ));
	mdq.swapdata[INDEXTOD].bidbest = (str2f(swapquot.swapbidTOM, FlotePoint) + str2f(swapquot.swapbidSpot, FlotePoint));//    ,sizeof(swapquot.swapbid12M  ));
	mdq.swapdata[INDEXTOD].offerlast = (str2f(swapquot.swapofferTOM, FlotePoint) + str2f(swapquot.swapofferSpot, FlotePoint));//    ,sizeof(swapquot.swapoffer12M));
	mdq.swapdata[INDEXTOD].offerbest = (str2f(swapquot.swapofferTOM, FlotePoint) + str2f(swapquot.swapofferSpot, FlotePoint));//    ,sizeof(swapquot.swapoffer12M));  
	mdq.swapdata[INDEXTOM].bidlast = str2f(swapquot.swapbidSpot, FlotePoint);//    ,sizeof(swapquot.swapbid12M  ));
	mdq.swapdata[INDEXTOM].bidbest = str2f(swapquot.swapbidSpot, FlotePoint);//    ,sizeof(swapquot.swapbid12M  ));
	mdq.swapdata[INDEXTOM].offerlast = str2f(swapquot.swapofferSpot, FlotePoint);//    ,sizeof(swapquot.swapoffer12M));
	mdq.swapdata[INDEXTOM].offerbest = str2f(swapquot.swapofferSpot, FlotePoint);//    ,sizeof(swapquot.swapoffer12M));  
	mdq.swapdata[INDEXSPOT].bidlast = 0;// str2f(swapquot.swapbidSpot  ,FlotePoint);//    ,sizeof(swapquot.swapbid12M  ));
	mdq.swapdata[INDEXSPOT].bidbest = 0;//str2f(swapquot.swapbidSpot  ,FlotePoint);//    ,sizeof(swapquot.swapbid12M  ));
	mdq.swapdata[INDEXSPOT].offerlast = 0;//str2f(swapquot.swapofferSpot,FlotePoint);//    ,sizeof(swapquot.swapoffer12M));
	mdq.swapdata[INDEXSPOT].offerbest = 0;//str2f(swapquot.swapofferSpot,FlotePoint);//    ,sizeof(swapquot.swapoffer12M));  

	mdq.foworddata[INDEX1W].donedaybidswap      = (mdq.swapdata[0].bidlast) / (m->expireNday1W);
	mdq.foworddata[INDEX1W].donedayofferswap    = (mdq.swapdata[0].offerlast) / (m->expireNday1W);
	mdq.foworddata[INDEX1M].donedaybidswap      = (mdq.swapdata[1].bidlast - mdq.swapdata[0].bidlast) / (m->expireNday1M - m->expireNday1W);
	mdq.foworddata[INDEX1M].donedayofferswap    = (mdq.swapdata[1].offerlast - mdq.swapdata[0].offerlast) / (m->expireNday1M - m->expireNday1W);
	mdq.foworddata[INDEX2M].donedaybidswap      = (mdq.swapdata[2].bidlast - mdq.swapdata[1].bidlast) / (m->expireNday2M - m->expireNday1M);
	mdq.foworddata[INDEX2M].donedayofferswap    = (mdq.swapdata[2].offerlast - mdq.swapdata[1].offerlast) / (m->expireNday2M - m->expireNday1M);
	mdq.foworddata[INDEX3M].donedaybidswap      = (mdq.swapdata[3].bidlast - mdq.swapdata[2].bidlast) / (m->expireNday3M - m->expireNday2M);
	mdq.foworddata[INDEX3M].donedayofferswap    = (mdq.swapdata[3].offerlast - mdq.swapdata[2].offerlast) / (m->expireNday3M - m->expireNday2M);
	mdq.foworddata[INDEX6M].donedaybidswap      = (mdq.swapdata[4].bidlast - mdq.swapdata[3].bidlast) / (m->expireNday6M - m->expireNday3M);
	mdq.foworddata[INDEX6M].donedayofferswap    = (mdq.swapdata[4].offerlast - mdq.swapdata[3].offerlast) / (m->expireNday6M - m->expireNday3M);
	mdq.foworddata[INDEX9M].donedaybidswap      = (mdq.swapdata[5].bidlast - mdq.swapdata[4].bidlast) / (m->expireNday9M - m->expireNday6M);
	mdq.foworddata[INDEX9M].donedayofferswap    = (mdq.swapdata[5].offerlast - mdq.swapdata[4].offerlast) / (m->expireNday9M - m->expireNday6M);
	mdq.foworddata[INDEX12M].donedaybidswap     = (mdq.swapdata[6].bidlast - mdq.swapdata[5].bidlast) / (m->expireNday12M - m->expireNday9M);
	mdq.foworddata[INDEX12M].donedayofferswap   = (mdq.swapdata[6].offerlast - mdq.swapdata[5].offerlast) / (m->expireNday12M - m->expireNday9M);
	mdq.foworddata[INDEXTOD].donedaybidswap     = 0;
	mdq.foworddata[INDEXTOD].donedayofferswap   = 0;
	mdq.foworddata[INDEXTOM].donedaybidswap     = 0;
	mdq.foworddata[INDEXTOM].donedayofferswap   = 0;
	mdq.foworddata[INDEXSPOT].donedaybidswap    = 0;
	mdq.foworddata[INDEXSPOT].donedayofferswap  = 0;


	mds_log(&mc, LOG_DEBUG, "[%-15s][%4d] 스왑환율 메모리저장완료. [%s]", __FUNCTION__, __LINE__, swapquot.symb);
	memcpy(mdquot, &mdq, sizeof(MDQUOT));
	mds_close(market);
	return 0;
}

int initSwapPriceData()
{
	return initSwapPrice();
}

/******************************************************************************
* Function Name : MarkupSetting(S_SENDMARKUP markupData, int nMarkupType)
* Description   : 마크업 정보를 메모리에 입력 합니다.
******************************************************************************/
int MarkupSetting(S_SENDMARKUP markupData, int nMarkupType)
{
	MARKET  *market, mc;
	char  exnm[40];
	int    rc;
	int ii = 0;
	int count = 0;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");


	markupData.symb[6] = 0x00;

	//if ((symbol==NULL) || (date==NULL) || (time==NULL) || (bidPrice==NULL) || (offerPrice==NULL) )
	if (strlen(markupData.symb) == 0)
	{
		mds_log(&mc, LOG_MUST, "CustomPriceSetting : 입력되지 않은 항목이 있습니다.\n");
		return -1;
	}

	if ((market = mds_open("CMBS", O_RDWR)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "Cannot open market '%s'\n", "Custom");
		return(-2);
	}


	MDFOLD  *folder;//, *rfolder; 
	//MDFOLD  *krwfolder;
	MDQUOT  *mdquot, mdq;
	MDMSTR  *m;
	struct  q_data  qd;
	/*-------------------------
	 * data set
	 * -----------------------*/
	if ((folder = mds_getfolder(market, markupData.symb)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "[%-15s][%4d] 등록된 종목정보가 없습니다. [%s]", __FUNCTION__, __LINE__, markupData.symb);
		return(NULL);
	}

	//1.메모리에 값 반영. 
	mdquot = &folder->quot;
	m = &folder->mstr;

	memcpy(&mdq, mdquot, sizeof(MDQUOT));
	m->pmul = 1;
	//  mdq.tymd = str2i(markupData.time, sizeof(markupData.time));
	mdq.seqn++;
	//  mdq.xymd = str2i(markupData.date, sizeof(markupData.date));
	//  mdq.xhms = str2i(markupData.time, sizeof(markupData.time))* 1000;
	//  mdq.kymd = str2i(markupData.date, sizeof(markupData.date));
	//  mdq.khms = str2i(markupData.time, sizeof(markupData.time))* 1000;

	  //struct q_price* q = mdq.swapdata;

	mdq.markupdata[INDEX1W].markupSet[nMarkupType].bidMarkup = str2f(markupData.markUpbid1W, FlotePoint);//  
	mdq.markupdata[INDEX1W].markupSet[nMarkupType].bidCMBSMarkup = str2f(markupData.markUpbid1W, FlotePoint);//  
	mdq.markupdata[INDEX1W].markupSet[nMarkupType].offerMarkup = str2f(markupData.markUpoffer1W, FlotePoint);//  
	mdq.markupdata[INDEX1W].markupSet[nMarkupType].offerCMBSMarkup = str2f(markupData.markUpoffer1W, FlotePoint);//  
	mdq.markupdata[INDEX1M].markupSet[nMarkupType].bidMarkup = str2f(markupData.markUpbid1M, FlotePoint);//  
	mdq.markupdata[INDEX1M].markupSet[nMarkupType].bidCMBSMarkup = str2f(markupData.markUpbid1M, FlotePoint);//  
	mdq.markupdata[INDEX1M].markupSet[nMarkupType].offerMarkup = str2f(markupData.markUpoffer1M, FlotePoint);//  
	mdq.markupdata[INDEX1M].markupSet[nMarkupType].offerCMBSMarkup = str2f(markupData.markUpoffer1M, FlotePoint);//  
	mdq.markupdata[INDEX2M].markupSet[nMarkupType].bidMarkup = str2f(markupData.markUpbid2M, FlotePoint);//  
	mdq.markupdata[INDEX2M].markupSet[nMarkupType].bidCMBSMarkup = str2f(markupData.markUpbid2M, FlotePoint);//  
	mdq.markupdata[INDEX2M].markupSet[nMarkupType].offerMarkup = str2f(markupData.markUpoffer2M, FlotePoint);//  
	mdq.markupdata[INDEX2M].markupSet[nMarkupType].offerCMBSMarkup = str2f(markupData.markUpoffer2M, FlotePoint);//    
	mdq.markupdata[INDEX3M].markupSet[nMarkupType].bidMarkup = str2f(markupData.markUpbid3M, FlotePoint);//  
	mdq.markupdata[INDEX3M].markupSet[nMarkupType].bidCMBSMarkup = str2f(markupData.markUpbid3M, FlotePoint);//  
	mdq.markupdata[INDEX3M].markupSet[nMarkupType].offerMarkup = str2f(markupData.markUpoffer3M, FlotePoint);//  
	mdq.markupdata[INDEX3M].markupSet[nMarkupType].offerCMBSMarkup = str2f(markupData.markUpoffer3M, FlotePoint);//  
	mdq.markupdata[INDEX6M].markupSet[nMarkupType].bidMarkup = str2f(markupData.markUpbid6M, FlotePoint);//  
	mdq.markupdata[INDEX6M].markupSet[nMarkupType].bidCMBSMarkup = str2f(markupData.markUpbid6M, FlotePoint);//  
	mdq.markupdata[INDEX6M].markupSet[nMarkupType].offerMarkup = str2f(markupData.markUpoffer6M, FlotePoint);//  
	mdq.markupdata[INDEX6M].markupSet[nMarkupType].offerCMBSMarkup = str2f(markupData.markUpoffer6M, FlotePoint);//  
	mdq.markupdata[INDEX9M].markupSet[nMarkupType].bidMarkup = str2f(markupData.markUpbid9M, FlotePoint);//  
	mdq.markupdata[INDEX9M].markupSet[nMarkupType].bidCMBSMarkup = str2f(markupData.markUpbid9M, FlotePoint);//  
	mdq.markupdata[INDEX9M].markupSet[nMarkupType].offerMarkup = str2f(markupData.markUpoffer9M, FlotePoint);//  
	mdq.markupdata[INDEX9M].markupSet[nMarkupType].offerCMBSMarkup = str2f(markupData.markUpoffer9M, FlotePoint);//  
	mdq.markupdata[INDEX12M].markupSet[nMarkupType].bidMarkup = str2f(markupData.markUpbid12M, FlotePoint);//  
	mdq.markupdata[INDEX12M].markupSet[nMarkupType].bidCMBSMarkup = str2f(markupData.markUpbid12M, FlotePoint);//  
	mdq.markupdata[INDEX12M].markupSet[nMarkupType].offerMarkup = str2f(markupData.markUpoffer12M, FlotePoint);//  
	mdq.markupdata[INDEX12M].markupSet[nMarkupType].offerCMBSMarkup = str2f(markupData.markUpoffer12M, FlotePoint);//  
	//TOD,TOM은 마크업 스왑방향이 반대다
	mdq.markupdata[INDEXTOD].markupSet[nMarkupType].bidMarkup = str2f(markupData.markUpofferTOD, FlotePoint);//  
	mdq.markupdata[INDEXTOD].markupSet[nMarkupType].bidCMBSMarkup = str2f(markupData.markUpofferTOD, FlotePoint);//  
	mdq.markupdata[INDEXTOD].markupSet[nMarkupType].offerMarkup = str2f(markupData.markUpbidTOD, FlotePoint);//  
	mdq.markupdata[INDEXTOD].markupSet[nMarkupType].offerCMBSMarkup = str2f(markupData.markUpbidTOD, FlotePoint);//    
	mdq.markupdata[INDEXTOM].markupSet[nMarkupType].bidMarkup = str2f(markupData.markUpofferTOM, FlotePoint);//  
	mdq.markupdata[INDEXTOM].markupSet[nMarkupType].bidCMBSMarkup = str2f(markupData.markUpofferTOM, FlotePoint);//  
	mdq.markupdata[INDEXTOM].markupSet[nMarkupType].offerMarkup = str2f(markupData.markUpbidTOM, FlotePoint);//  
	mdq.markupdata[INDEXTOM].markupSet[nMarkupType].offerCMBSMarkup = str2f(markupData.markUpbidTOM, FlotePoint);//  
	//TOD,TOM은 마크업 스왑방향이 반대다
    if (m->iNowVirtualAmoutType == 0) //2024.03.08 초기화 마크업 분기
	{
		mdq.markupdata[INDEXSPOT].markupSet[nMarkupType].bidMarkup = str2f(markupData.markUpbidSMBSSpot, FlotePoint);//  
		mdq.markupdata[INDEXSPOT].markupSet[nMarkupType].offerMarkup = str2f(markupData.markUpofferSMBSSpot, FlotePoint);//  
	}else
	{
		mdq.markupdata[INDEXSPOT].markupSet[nMarkupType].bidMarkup = str2f(markupData.markUpbidSpot, FlotePoint);//  
		mdq.markupdata[INDEXSPOT].markupSet[nMarkupType].offerMarkup = str2f(markupData.markUpofferSpot, FlotePoint);//  
	}
	mdq.markupdata[INDEXSPOT].markupSet[nMarkupType].bidCMBSMarkup = str2f(markupData.markUpbidSpot, FlotePoint);//  
	mdq.markupdata[INDEXSPOT].markupSet[nMarkupType].bidSMBSMarkup = str2f(markupData.markUpbidSMBSSpot, FlotePoint);//  
	mdq.markupdata[INDEXSPOT].markupSet[nMarkupType].offerCMBSMarkup = str2f(markupData.markUpofferSpot, FlotePoint);//  
	mdq.markupdata[INDEXSPOT].markupSet[nMarkupType].offerSMBSMarkup = str2f(markupData.markUpofferSMBSSpot, FlotePoint);//    
	mds_log(&mc, LOG_DEBUG, "[%-15s][%4d] mdq.markupdata[INDEXSPOT].markupSet[%d].bidMarkup [%f] [%s]", __FUNCTION__, __LINE__, nMarkupType, mdq.markupdata[INDEXSPOT].markupSet[nMarkupType].bidMarkup, markupData.symb);
	mds_log(&mc, LOG_DEBUG, "[%-15s][%4d] mdq.markupdata[INDEXSPOT].markupSet[%d].offerMarkup [%f] [%s]", __FUNCTION__, __LINE__, nMarkupType, mdq.markupdata[INDEXSPOT].markupSet[nMarkupType].offerMarkup, markupData.symb);

	mdq.foworddata[INDEX1W].onedayMarkup[nMarkupType].donedaybidMarkup = (mdq.markupdata[INDEX1W].markupSet[nMarkupType].bidMarkup) / (m->expireNday1W);
	mdq.foworddata[INDEX1W].onedayMarkup[nMarkupType].donedayofferMarkup = (mdq.markupdata[INDEX1W].markupSet[nMarkupType].offerMarkup) / (m->expireNday1W);
	mdq.foworddata[INDEX1M].onedayMarkup[nMarkupType].donedaybidMarkup = (mdq.markupdata[INDEX1M].markupSet[nMarkupType].bidMarkup - mdq.markupdata[INDEX1W].markupSet[nMarkupType].bidMarkup) / (m->expireNday1M - m->expireNday1W);
	mdq.foworddata[INDEX1M].onedayMarkup[nMarkupType].donedayofferMarkup = (mdq.markupdata[INDEX1M].markupSet[nMarkupType].offerMarkup - mdq.markupdata[INDEX1W].markupSet[nMarkupType].offerMarkup) / (m->expireNday1M - m->expireNday1W);
	mdq.foworddata[INDEX2M].onedayMarkup[nMarkupType].donedaybidMarkup = (mdq.markupdata[INDEX2M].markupSet[nMarkupType].bidMarkup - mdq.markupdata[INDEX1M].markupSet[nMarkupType].bidMarkup) / (m->expireNday2M - m->expireNday1M);
	mdq.foworddata[INDEX2M].onedayMarkup[nMarkupType].donedayofferMarkup = (mdq.markupdata[INDEX2M].markupSet[nMarkupType].offerMarkup - mdq.markupdata[INDEX1M].markupSet[nMarkupType].offerMarkup) / (m->expireNday2M - m->expireNday1M);
	mdq.foworddata[INDEX3M].onedayMarkup[nMarkupType].donedaybidMarkup = (mdq.markupdata[INDEX3M].markupSet[nMarkupType].bidMarkup - mdq.markupdata[INDEX2M].markupSet[nMarkupType].bidMarkup) / (m->expireNday3M - m->expireNday2M);
	mdq.foworddata[INDEX3M].onedayMarkup[nMarkupType].donedayofferMarkup = (mdq.markupdata[INDEX3M].markupSet[nMarkupType].offerMarkup - mdq.markupdata[INDEX2M].markupSet[nMarkupType].offerMarkup) / (m->expireNday3M - m->expireNday2M);
	mdq.foworddata[INDEX6M].onedayMarkup[nMarkupType].donedaybidMarkup = (mdq.markupdata[INDEX6M].markupSet[nMarkupType].bidMarkup - mdq.markupdata[INDEX3M].markupSet[nMarkupType].bidMarkup) / (m->expireNday6M - m->expireNday3M);
	mdq.foworddata[INDEX6M].onedayMarkup[nMarkupType].donedayofferMarkup = (mdq.markupdata[INDEX6M].markupSet[nMarkupType].offerMarkup - mdq.markupdata[INDEX3M].markupSet[nMarkupType].offerMarkup) / (m->expireNday6M - m->expireNday3M);
	mdq.foworddata[INDEX9M].onedayMarkup[nMarkupType].donedaybidMarkup = (mdq.markupdata[INDEX9M].markupSet[nMarkupType].bidMarkup - mdq.markupdata[INDEX6M].markupSet[nMarkupType].bidMarkup) / (m->expireNday9M - m->expireNday6M);
	mdq.foworddata[INDEX9M].onedayMarkup[nMarkupType].donedayofferMarkup = (mdq.markupdata[INDEX9M].markupSet[nMarkupType].offerMarkup - mdq.markupdata[INDEX6M].markupSet[nMarkupType].offerMarkup) / (m->expireNday9M - m->expireNday6M);
	mdq.foworddata[INDEX12M].onedayMarkup[nMarkupType].donedaybidMarkup = (mdq.markupdata[INDEX12M].markupSet[nMarkupType].bidMarkup - mdq.markupdata[INDEX9M].markupSet[nMarkupType].bidMarkup) / (m->expireNday12M - m->expireNday9M);
	mdq.foworddata[INDEX12M].onedayMarkup[nMarkupType].donedayofferMarkup = (mdq.markupdata[INDEX12M].markupSet[nMarkupType].offerMarkup - mdq.markupdata[INDEX9M].markupSet[nMarkupType].offerMarkup) / (m->expireNday12M - m->expireNday9M);
	mdq.foworddata[INDEXTOD].onedayMarkup[nMarkupType].donedaybidMarkup = 0;
	mdq.foworddata[INDEXTOD].onedayMarkup[nMarkupType].donedayofferMarkup = 0;
	mdq.foworddata[INDEXTOM].onedayMarkup[nMarkupType].donedaybidMarkup = 0;
	mdq.foworddata[INDEXTOM].onedayMarkup[nMarkupType].donedayofferMarkup = 0;
	mdq.foworddata[INDEXSPOT].onedayMarkup[nMarkupType].donedaybidMarkup = 0;
	mdq.foworddata[INDEXSPOT].onedayMarkup[nMarkupType].donedayofferMarkup = 0;
	//mds_log(&mc, LOG_MUST, "[%-15s][%4d] 0[%f]", __FUNCTION__, __LINE__, mdq.foworddata[INDEX1W].onedayMarkup[nMarkupType].donedaybidMarkup);  
	//mds_log(&mc, LOG_MUST, "[%-15s][%4d] 1[%f]", __FUNCTION__, __LINE__, mdq.foworddata[INDEX1M].onedayMarkup[nMarkupType].donedaybidMarkup);  
	//mds_log(&mc, LOG_MUST, "[%-15s][%4d] 2[%f]", __FUNCTION__, __LINE__, mdq.foworddata[INDEX2M].onedayMarkup[nMarkupType].donedaybidMarkup);  
	//mds_log(&mc, LOG_MUST, "[%-15s][%4d] 3[%f]", __FUNCTION__, __LINE__, mdq.foworddata[INDEX3M].onedayMarkup[nMarkupType].donedaybidMarkup);  
	//mds_log(&mc, LOG_MUST, "[%-15s][%4d] 4[%f]", __FUNCTION__, __LINE__, mdq.foworddata[INDEX6M].onedayMarkup[nMarkupType].donedaybidMarkup);  
	//mds_log(&mc, LOG_MUST, "[%-15s][%4d] 마크업 메모리저장완료. [%s]", __FUNCTION__, __LINE__, markupData.symb);  
	int nRet = 0;
	memcpy(mdquot, &mdq, sizeof(MDQUOT));
	mds_close(market);
	return nRet;
}

//마크업 그룹 3개를 로드 한다. 
int initMarkupPriceData()
{
	//내부거래
	if (initMarkupPrice(MARKUPEMP, MARKUPINDEXEMP) != 0)
		return -1;
	//중소기업
	if (initMarkupPrice(MARKUPPRI, MARKUPINDEXPRI) != 0)
		return -1;
	//대기업
	if (initMarkupPrice(MARKUPENT, MARKUPINDEXENT) != 0)
		return -1;
	//기관 
	if (initMarkupPrice(MARKUPENT2, MARKUPINDEXENT2) != 0)
		return -1;
	return 0;
}

int initPriceOrign()
{
	MARKET mc;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");
	return roadPriceOrign(&mc);

}

/******************************************************************************
* Function Name : int SetPriceOrign(PRICEORIGN* bp)
* Description   : 입력받음 가격원천 정보를 메모리에 반영, 시세정지일경우 환율수신 상태도 초기화 한다. 
******************************************************************************/
int SetPriceOrign(PRICEORIGN* bp)
{

	MARKET  *market, mc;
	char  exnm[40];
	int    rc;
	int ii = 0;
	int count = 0;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");
	bp->fx_prdct_cd[6] = 0x00;

	if (strlen(bp->fx_prdct_cd) == 0)
	{
		mds_log(&mc, LOG_MUST, "SetPriceOrign : 입력되지 않은 항목이 있습니다.\n");
		return -1;
	}

	if ((market = mds_open("CMBS", O_RDWR)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "Cannot open market '%s'\n", "Custom");
		return(-2);
	}

	MDFOLD  *folder;

	MDQUOT  *mdq;
	MDMSTR  *m;
	struct  q_data  qd;
	mds_log(&mc, LOG_MUST, "[%-15s][%4d] SetPriceOrign [%s]", __FUNCTION__, __LINE__, bp->fx_prdct_cd);
	/*-------------------------
	 * data set
	 * -----------------------*/
	if ((folder = mds_getfolder(market, bp->fx_prdct_cd)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "등록된 종목정보가 없습니다. [%s]", bp->fx_prdct_cd);
		return(NULL);
	}

	m = &folder->mstr;
	m->pricestat = atoi(bp->prc_orign_dstic);
	m->smbstime = atoi(bp->smbs_aply_start_hms);
	m->cmbstime = atoi(bp->cmbs_aply_start_hms);
	m->stoptime = atoi(bp->exrt_ofer_dscn_hms);
	if (m->pricestat == SOURCE_STOP - 1) //시세정지
	{
		m->trdf = 0; //중지이후 해제하였을경우 시장가 체결방지 . 
	}
	mds_close(market);
	return 0;
}

/******************************************************************************
* Function Name : int initBasePriceData()
* Description   : 기본 데이터 적재 함수 .
******************************************************************************/
int initBasePriceData()
{
	MARKET mc;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");
	return initBasePrice(&mc);
}


/******************************************************************************
* Function Name : int SetBasePrice(BASEPRICEINPUT* bp)
* Description   : 현재적용된 가격을 읽어온다. 재시작 또는 일자 변경시 실행된다. reloadmaster를 통해 실행된다. 
******************************************************************************/
int SetBasePrice(BASEPRICEINPUT* bp)
{
	MARKET  *market, mc;
	char  exnm[40];
	int    rc;
	int ii = 0;
	int count = 0;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");
	bp->symb[6] = 0x00;

	if (strlen(bp->symb) == 0)
	{
		mds_log(&mc, LOG_MUST, "SetBasePrice : 입력되지 않은 항목이 있습니다.\n");
		return -1;
	}

	if ((market = mds_open("CMBS", O_RDWR)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "Cannot open market '%s'\n", "Custom");
		return(-2);
	}
	MDFOLD  *folder;

	MDQUOT  *mdq;
	MDMSTR  *m;
	struct  q_data  qd;

	/*-------------------------
	 * data set
	 * -----------------------*/
	if ((folder = mds_getfolder(market, bp->symb)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "등록된 종목정보가 없습니다. [%s]", bp->symb);
		return(NULL);
	}

	//1.메모리에 값 반영. 
	mdq = &folder->quot;
	m = &folder->mstr;
	m->iNowVirtualAmoutType = 0;  //시세초기화시 가상잔량타입. 0으로 설정 (SMBS 첫데이터 수신시 지연방지)
	m->trdf = 0;    //시세초기화시 수신여부도 초기화
	mdq->spotdata.bidbase = str2f(bp->bidspotbase, FlotePoint);//,sizeof(bp->bidspotbase    ));
	mdq->spotdata.offerbase = str2f(bp->offerspotbase, FlotePoint);//,sizeof(bp->offerspotbase  ));
	mdq->spotdata.midbase = str2f(bp->midspotbase, FlotePoint);//,sizeof(bp->midspotbase    ));

	mdq->spotdata.bidopen = str2f(bp->bidOpenprice, FlotePoint);//,sizeof(bp->bidspotbase    ));
	mdq->spotdata.bidhigh = str2f(bp->bidHighprice, FlotePoint);//,sizeof(bp->offerspotbase  ));
	mdq->spotdata.bidlow = str2f(bp->bidLowprice, FlotePoint);//,sizeof(bp->midspotbase    ));

	mdq->spotdata.offeropen = str2f(bp->offerOpenprice, FlotePoint);//,sizeof(bp->bidspotbase    ));
	mdq->spotdata.offerhigh = str2f(bp->offerHighprice, FlotePoint);//,sizeof(bp->offerspotbase  ));
	mdq->spotdata.offerlow = str2f(bp->offerLowprice, FlotePoint);//,sizeof(bp->midspotbase    ));


	int nRet = reloadmaster(market, m);
	if (nRet != 1)
	{
		mds_log(&mc, LOG_MUST, "[%-15s][%4d] reloadmaster Fail . [%s] [%d]", __FUNCTION__, __LINE__, bp->symb, nRet);

	}

	mds_log(&mc, LOG_MUST, "[%-15s][%4d] 전일종가 로드. [%s]", __FUNCTION__, __LINE__, bp->symb);
	mds_log(&mc, LOG_MUST, "[%-15s][%4d]midbase 전일종가 로드. [%f]", __FUNCTION__, __LINE__, mdq->spotdata.midbase);
	mds_log(&mc, LOG_MUST, "[%-15s][%4d]bidbase 전일종가 로드. [%f]", __FUNCTION__, __LINE__, mdq->spotdata.bidbase);
	mds_log(&mc, LOG_MUST, "[%-15s][%4d]offerbase 전일종가 로드. [%f]", __FUNCTION__, __LINE__, mdq->spotdata.offerbase);
	mds_log(&mc, LOG_MUST, "[%-15s][%4d] 가상잔량 타입. [%d]", __FUNCTION__, __LINE__, m->iNowVirtualAmoutType);
	mds_close(market);
	return 0;
}


/******************************************************************************
* Function Name : int ReloadVirtualQty()
* Description   : 가상잔량 적용 기준 갱신시 해당 정보를 다시 읽어온다. 마스터파일 새로 읽기(reloadmaster)를 통해 수행된다. 
******************************************************************************/
int ReloadVirtualQty()
{
	MARKET  *market, mc;
	char  exnm[40];
	int    rc;
	int ii = 0;
	int count = 0;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");
	if ((market = mds_open("CMBS", O_RDWR)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "[%-15s][%4d] Cannot open market '%s'\n", __FUNCTION__, __LINE__, "Custom");
		return -2;
	}
	MDFOLD  *folder;
	MDMSTR  *m;
	/*-------------------------
	 * USDKRW만 반영합니다. 
	 * -----------------------*/
	if ((folder = mds_getfolder(market, "USDKRW")) == NULL)
	{
		mds_log(&mc, LOG_MUST, "[%-15s][%4d] 등록된 종목정보가 없습니다. [%s]", __FUNCTION__, __LINE__, "USDKRW");
		return -1;
	}

	//1.메모리에 값 반영. 
	m = &folder->mstr;
	int nRet = reloadmaster(market, m);
	if (nRet != 1)
	{
		mds_log(&mc, LOG_MUST, "[%-15s][%4d] reloadmaster Fail . [%s] [%d]", __FUNCTION__, __LINE__, "USDKRW", nRet);

	}
	mds_close(market);
	return 0;
}


/******************************************************************************
* Function Name : int insertCalcCAmount(char *symbol, DATAFIELD df)
* Description   : 체결시 체결수량 정보를 메모리에 반영을 한다. 이후 가상잔량을 계산한다. 
******************************************************************************/
int insertCalcCAmount(char *symbol, DATAFIELD df)
{
	MARKET  *market, mc;
	MDFOLD  *folder;
	char  exnm[40];
	int    rc;
	int ii = 0;
	int count = 0;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");

	if (strlen(symbol) == 0)
	{
		mds_log(&mc, LOG_MUST, "[%-15s][%4d] CustomPriceSetting : 입력되지 않은 항목이 있습니다.\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if ((market = mds_open("CMBS", O_RDWR)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "[%-15s][%4d] Cannot open market '%s'\n", __FUNCTION__, __LINE__, "Custom");
		return(-2);
	}



	/*-------------------------
	 * data set USDKRW 만 가상잔량 반영으로 해당 폴더만 반영
	 * -----------------------*/
	if ((folder = mds_getfolder(market, symbol)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "[%-15s][%4d] 등록된 종목정보가 없습니다. [%s]", __FUNCTION__, __LINE__, symbol);
		return(-3);
	}


	MDTRAD  *mdtrad;
	MDMSTR  *m;
	struct  q_data  qd;
	//1.메모리에 값 반영. 
	mdtrad = &folder->trad;
	m = &folder->mstr;
	int nEnd = mdtrad->nEnd;

	mds_plussecound(m->nOrderrecy_ttm, df.exymd, df.xhms, &mdtrad->orderdata[nEnd].exymd, &mdtrad->orderdata[nEnd].exhms);
	mdtrad->orderdata[nEnd].xhms = df.xhms;
	//mdtrad->orderdata[nEnd].exymd    = df.exymd;
	//mdtrad->orderdata[nEnd].exhms    = df.exhms;
	mdtrad->orderdata[nEnd].nhoga = df.nhoga;
	mdtrad->orderdata[nEnd].dAmount = df.dAmount;
	// 20230619 nEnd값이 최대개수를 초과해도 계속 커지는 현상 수정
	mdtrad->nEnd = (mdtrad->nEnd + 1)% MAX_SAVEORDER;
	mds_log(&mc, LOG_DEBUG, "[%-15s][%4d] xhms [%d] xhms [%d] xhms [%d] xhms [%d] ", __FUNCTION__, __LINE__, df.exymd, df.xhms, mdtrad->orderdata[nEnd].exymd, mdtrad->orderdata[nEnd].exhms);
	CalcCAmount(folder);
	//잔량이 변경되어서 시세를 내려줘야 한다. 
	char  check[MAX_ISAM_F];
	memset(&check[0], 0, sizeof(check));
	check[BOOK] = X_PUSH;
	sendfolder(market, folder, check, SOURCE_HANDWRTING);
	mds_close(market);
	return 0;

}
  
  
/******************************************************************************
* Function Name : int CalcCAmount(MDFOLD  *folder)
* Description   : 메모리의 체결 수량 정보를 기초로 가상잔량을 반영한다. 지정시간 이내의 체결수량을 모두 더한다. 
******************************************************************************/
int CalcCAmount(MDFOLD  *folder)
{
	int ii = 0, jj = 0;
	int  nStart = 0;
	int  nEnd = 0, nRealIndex = 0;

	MARKET  *market2, mc;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");

	uint32_t kymd = 0;
	uint32_t khms = 0;
	uint32_t exymd = 0;
	uint32_t exhms = 0;
	time_t  clock;
	time(&clock);
	mds_ktime(NULL, clock, &kymd, &khms);
	//mds_plussecound(50, kymd, khms, &exymd, &exhms);
	//
	MDMSTR  *m = &folder->mstr;
	MDBOOK  *book = &folder->book;
	MDTRAD  *trad = &folder->trad;
	nStart = trad->nStart;
	nEnd = trad->nEnd;

	if (nStart > nEnd) //한바퀴돌아서 처음부터 데이터 저장하는 경우 
	{
		nEnd = nEnd + MAX_SAVEORDER;
	}
	double dTotalask = 0;
	double dTotalbid = 0;
	// 주문리스트 처음부터 끝까지 읽어온다. 
	//mds_log(&mc, LOG_MUST, "[%-15s][%4d] nStart [%d] nEnd [%d] ", __FUNCTION__, __LINE__, nStart, nEnd);    
	for (ii = nStart; ii < nEnd; ii++)
	{
		nRealIndex = ii % MAX_SAVEORDER;
		//유효일자가 더 현재일자 보다 더크면 24시간을 더해서 계산한다. 
		exymd = trad->orderdata[nRealIndex].exymd;
		exhms = trad->orderdata[nRealIndex].exhms;

		if (exymd > kymd)
			exhms = exhms + 240000;
		// 현재시간이 주문 유효시간보다 작으면 유효한 주문이르로 가상체결량에 더해준다 
		//mds_log(&mc, LOG_MUST, "[%-15s][%4d] kymd [%d] khms [%d] exymd [%d] exhms [%d] ", __FUNCTION__, __LINE__, kymd, khms, exymd, exhms);       
		if (exhms > khms)
		{
			if (trad->orderdata[nRealIndex].nhoga >= 0)
			{
				dTotalask = dTotalask + trad->orderdata[nRealIndex].dAmount;
			}
			else
			{
				dTotalbid = dTotalbid + trad->orderdata[nRealIndex].dAmount;
			}

		}
		else    //아니면 다음번에 현재 위치에서부터 계산하도록 처음을 변경해준다. 
		{
			trad->nStart = nRealIndex;
		}
	}

	//  mds_log(&mc, LOG_MUST, "[%-15s][%4d] dTotalask [%f] dTotalbid [%f]", __FUNCTION__, __LINE__, dTotalask, dTotalbid);    
	book->spotdata.cask = dTotalask;
	book->spotdata.cbid = dTotalbid;
}
  
  

/******************************************************************************
* Function Name : double GetMid(double bidprice, double askprice, int zdiv)
* Description   : Mid가격 생성
******************************************************************************/
double GetMid(double bidprice, double askprice, int zdiv)
{
	return (bidprice + askprice) / 2;
}

 
/******************************************************************************
* Function Name : int color(double val, double base)
* Description   : 가격의 업다운 색상 뒤의가격 기준 상승 2 하락 5 보합 3 을 반환
******************************************************************************/
int color(double val, double base)
{

	if (val > base) return 2;
	else if (val < base) return 5;
	else return 3;
}


/******************************************************************************
* Function Name : int SetPriceSource(char *symbol, int iSource)
* Description   : 통화코드의 가격 원천을 변경  한다 
******************************************************************************/
int SetPriceSource(char *symbol, int iSource)
{
	char  esym[24], exnm[20];
	MARKET  *market, mc;
	MDFOLD  *folder;
	MDMSTR  *mstr;
	memset(&mc, 0, sizeof(MARKET));
	sprintf(mc.procname, "Custom");
	int    len;
	int   nRet;
	STR2S(exnm, "CMBS");
	STR2S(esym, symbol);
	esym[6] = 0x00;

	if (symbol == NULL)
	{
		mds_log(&mc, LOG_MUST, "SetPriceSource : 입력되지 않은 항목이 있습니다.\n");
		return -1;
	}

	if (strlen(symbol) < 6)
	{
		mds_log(&mc, LOG_MUST, "SymbolCode is Too Short '%s'\n", symbol);
		return(-3);
	}

	if ((iSource < SOURCE_SMBS) || (iSource > SOURCE_STOP))
	{
		mds_log(&mc, LOG_MUST, "Source vaoue is too big or too short '%d'\n", iSource);
		return(-3);
	}


	if ((market = mds_open("CMBS", O_RDWR)) == NULL)
	{
		mds_log(&mc, LOG_MUST, "Cannot open market '%s'\n", "Custom");
		return(-2);
	}

	if (market == NULL)
	{
		mds_log(&mc, LOG_MUST, "[%-15s][%4d]거래소를 찾을 수 없습니다. [%s]", __FUNCTION__, __LINE__, exnm);
		return(-4);
	}

	if (folder == NULL)
	{
		mds_log(&mc, LOG_MUST, "[%-15s][%4d]Symbol is not found. [%s]  ", __FUNCTION__, __LINE__, esym);
		return(-5);
	}

	mstr = &folder->mstr;
	if (mstr == NULL)
	{
		mds_log(&market, LOG_MUST, "[%-15s][%4d]종목정보가 없습니다.", __FUNCTION__, __LINE__);
		return -6;
	}

	mstr->pricestat = iSource;
	if (mstr->pricestat == SOURCE_STOP - 1) //시세정지
	{
		mstr->trdf = 0; //중지이후 해제하였을경우 시장가 체결방지 . 
	}
	// 이후 해당 시세원천에 맞게 처리 필요
	mds_close(market);
	return 0;
}

/******************************************************************************
* Function Name : int UpdateMinutePrice()
* Description   : 현재 환율 정보를 분데이터 테이블에 업데이트 한다. 
******************************************************************************/
int UpdateMinutePrice(int iMarkupGroup)
{
	MARKET  *market2;
	MDFOLD  *folder;
	S_SENDQUOT sendquot;
	int nRet = 0;
	int count = 0;
	market2 = mds_open("CMBS", O_RDONLY);
	if (market2 == NULL)
		return 0;
	mds_setfolder(market2, NULL);

	while ((folder = mds_popfolder(market2)) != NULL)
	{
		MDMSTR *m = &folder->mstr;
		MDBOOK  *book = &folder->book;
		MDQUOT  *quot = &folder->quot;

		if (m == NULL)
			continue;


		//시세 중지중인 종목 
		if (m->pricestat == SOURCE_STOP - 1) //시세정지
		{
			mds_log(market2, LOG_MUST, "[%-15s][%4d] 시세원천이 중지 중입니다. [%s] ", __FUNCTION__, __LINE__, folder->quot.symb);
			continue;
		}
		//시세안들어온종목
		if (m->trdf != 3)
		{
			continue;
		}
		int nPind = m->zdiv;
		memset(&g_quote, 0x00, sizeof(g_quote));

		if (quot == NULL)
			continue;
		if (memcmp(folder->quot.symb, "MARKRW", 6) != 0)
		{
			if (quot->spotdata.bidlast == 0)
				continue;
			if (quot->spotdata.offerlast == 0)
				continue;
		}

		memset(&sendquot, 0, sizeof(S_SENDQUOT));
		sprintf(sendquot.symb, "%.*s%.3s", SYMB_LEN, folder->quot.symb, subCodeSpot[0]);
		sendquot.xymd = folder->quot.xymd;
		sendquot.xhms = folder->quot.xhms;
		sendquot.kymd = folder->quot.kymd;
		sendquot.khms = folder->quot.khms;

		memcpy(&sendquot.pricedata, &folder->quot.spotdata, sizeof(folder->quot.spotdata));
		/// 마크업정보 추가  20230707
        sendquot.pricedata.onedayMarkup[0].dbidMarkup   = folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXENT2].bidMarkup* m->swappmul;
        sendquot.pricedata.onedayMarkup[1].dbidMarkup   = folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXENT].bidMarkup* m->swappmul;
        sendquot.pricedata.onedayMarkup[2].dbidMarkup   = folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXPRI].bidMarkup* m->swappmul;
        sendquot.pricedata.onedayMarkup[3].dbidMarkup   = folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXEMP].bidMarkup* m->swappmul;
   //     sendquot.pricedata.onedayMarkup[0].dofferMarkup   = folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXENT2].bidMarkup* m->swappmul;
        sendquot.pricedata.onedayMarkup[0].dofferMarkup     = folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXENT2].offerMarkup* m->swappmul;
        sendquot.pricedata.onedayMarkup[1].dofferMarkup     = folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXENT].offerMarkup* m->swappmul;
        sendquot.pricedata.onedayMarkup[2].dofferMarkup     = folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXPRI].offerMarkup* m->swappmul;
        sendquot.pricedata.onedayMarkup[3].dofferMarkup     = folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXEMP].offerMarkup* m->swappmul;
     //   sendquot.pricedata.onedayMarkup[0].dbidMarkup     = folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXENT2].offerMarkup* m->swappmul;
        /// 마크업정보 추가  
		nRet = UPDATEDAYMINDATA(market2, &sendquot, sizeof(S_SENDQUOT), folder->mstr.pind, iMarkupGroup);//folder->mstr.iNowVirtualAmoutType);
		if (nRet != 0)
		{
			mds_log(market2, LOG_DEBUG, "[%-15s][%4d] 업데이트실패 [%s][%d] ", __FUNCTION__, __LINE__, folder->quot.symb, nRet);
			l_db2rollback();
			return nRet;
		}
		for (count = 0; count < MAX_TENNER - 1; count++)
		{
			sprintf(sendquot.symb, "%.*s%.*s", SYMB_LEN, folder->quot.symb, SYMB_SUBLEN, subFowardode[count]);
			memcpy(&sendquot.pricedata, &folder->quot.foworddata[count], sizeof(folder->quot.foworddata[count]));
			/// 마크업정보 추가      20230707 spot + swap마크업
            sendquot.pricedata.onedayMarkup[0].dbidMarkup   =(folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXENT2].bidMarkup     +   folder->quot.markupdata[count].markupSet[MARKUPINDEXENT2].bidMarkup   )* m->swappmul;     
            sendquot.pricedata.onedayMarkup[1].dbidMarkup   =(folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXENT].bidMarkup      +   folder->quot.markupdata[count].markupSet[MARKUPINDEXENT].bidMarkup    )* m->swappmul;      
            sendquot.pricedata.onedayMarkup[2].dbidMarkup   =(folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXPRI].bidMarkup      +   folder->quot.markupdata[count].markupSet[MARKUPINDEXPRI].bidMarkup    )* m->swappmul;      
            sendquot.pricedata.onedayMarkup[3].dbidMarkup   =(folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXEMP].bidMarkup      +   folder->quot.markupdata[count].markupSet[MARKUPINDEXEMP].bidMarkup    )* m->swappmul;      
          //  sendquot.pricedata.onedayMarkup[0].dbidMarkup   =(folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXENT2].bidMarkup     +   folder->quot.markupdata[count].markupSet[MARKUPINDEXENT2].bidMarkup   )* m->swappmul;     
            sendquot.pricedata.onedayMarkup[0].dofferMarkup     =(folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXENT2].offerMarkup   +   folder->quot.markupdata[count].markupSet[MARKUPINDEXENT2].offerMarkup )* m->swappmul;   
            sendquot.pricedata.onedayMarkup[1].dofferMarkup     =(folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXENT].offerMarkup    +   folder->quot.markupdata[count].markupSet[MARKUPINDEXENT].offerMarkup  )* m->swappmul;    
            sendquot.pricedata.onedayMarkup[2].dofferMarkup     =(folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXPRI].offerMarkup    +   folder->quot.markupdata[count].markupSet[MARKUPINDEXPRI].offerMarkup  )* m->swappmul;    
            sendquot.pricedata.onedayMarkup[3].dofferMarkup     =(folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXEMP].offerMarkup    +   folder->quot.markupdata[count].markupSet[MARKUPINDEXEMP].offerMarkup  )* m->swappmul;    
        //    sendquot.pricedata.onedayMarkup[0].dofferMarkup     =(folder->quot.markupdata[INDEXSPOT].markupSet[MARKUPINDEXENT2].offerMarkup   +   folder->quot.markupdata[count].markupSet[MARKUPINDEXENT2].offerMarkup )* m->swappmul; 
            /// 마크업정보 추가  
			nRet = UPDATEDAYMINDATA(market2, &sendquot, sizeof(S_SENDQUOT), folder->mstr.pind,  iMarkupGroup);//folder->mstr.iNowVirtualAmoutType);
			if (nRet != 0)
			{
				mds_log(market2, LOG_DEBUG, "[%-15s][%4d] 업데이트실패  [%s][%d] ", __FUNCTION__, __LINE__, folder->quot.symb, nRet);
				l_db2rollback();
				return nRet;
			}
		}

		l_db2commit();
	}
	return 0;
}



/******************************************************************************
* Function Name : int UpdateDayPrice()
* Description   : 현재 환율 정보를 일간데이터 테이블에 업데이트 한다. 
******************************************************************************/
int UpdateDayPrice()
{
	MARKET  *market2;
	MDFOLD  *folder;
	S_SENDQUOT sendquot;
	int nRet = 0;
	int count = 0;
	market2 = mds_open("CMBS", O_RDONLY);
	if (market2 == NULL)
		return 0;
	mds_setfolder(market2, NULL);

	while ((folder = mds_popfolder(market2)) != NULL)
	{
		MDMSTR *m = &folder->mstr;
		MDBOOK  *book = &folder->book;
		MDQUOT  *quot = &folder->quot;

		if (m == NULL)
			continue;
		//최초시세 미수신 종목 

		//시세 중지중인 종목 
		if (m->pricestat == SOURCE_STOP - 1) //시세정지
		{
			mds_log(market2, LOG_MUST, "[%-15s][%4d] 시세원천이 중지 중입니다. [%s] ", __FUNCTION__, __LINE__, folder->quot.symb);
			continue;
		}
		int nPind = m->zdiv;
		memset(&g_quote, 0x00, sizeof(g_quote));

		if (quot == NULL)
			continue;
		if (memcmp(folder->quot.symb, "MARKRW", 6) != 0)
		{
			if (quot->spotdata.bidlast == 0)
				continue;
			if (quot->spotdata.offerlast == 0)
				continue;
		}

		memset(&sendquot, 0, sizeof(S_SENDQUOT));
		sprintf(sendquot.symb, "%.*s%.3s", SYMB_LEN, folder->quot.symb, subCodeSpot[0]);
		sendquot.xymd = folder->quot.xymd;
		sendquot.xhms = folder->quot.xhms;
		sendquot.kymd = folder->quot.kymd;
		sendquot.khms = folder->quot.khms;

		if (m->trdf != 3)
		{
			sendquot.xymd = folder->quot.xymd;
			sendquot.xhms = 0;
			sendquot.kymd = folder->quot.kymd;
			sendquot.khms = 0;
		}


		memcpy(&sendquot.pricedata, &folder->quot.spotdata, sizeof(folder->quot.spotdata));

		nRet = UPDATEDAYDATA(market2, &sendquot, sizeof(S_SENDQUOT), folder->mstr.pind, folder->mstr.iNowVirtualAmoutType);
		if (nRet != 0)
		{
			mds_log(market2, LOG_MUST, "[%-15s][%4d] UPDATEDAYDATA [%d] ", __FUNCTION__, __LINE__, nRet);
			l_db2rollback();
			return nRet;
		}
		for (count = 0; count < MAX_TENNER - 1; count++)
		{
			sprintf(sendquot.symb, "%.*s%.*s", SYMB_LEN, folder->quot.symb, SYMB_SUBLEN, subFowardode[count]);
			memcpy(&sendquot.pricedata, &folder->quot.foworddata[count], sizeof(folder->quot.foworddata[count]));
			nRet = UPDATEDAYDATA(market2, &sendquot, sizeof(S_SENDQUOT), folder->mstr.pind, folder->mstr.iNowVirtualAmoutType);
			if (nRet != 0)
			{
				mds_log(market2, LOG_MUST, "[%-15s][%4d] UPDATEDAYDATA [%d] ", __FUNCTION__, __LINE__, nRet);
				l_db2rollback();
				return nRet;
			}
		}
		l_db2commit();
	}
	return 0;

}

/******************************************************************************
* Function Name : int InitTrade()
* Description   : 호가 가상잔량 인덱스를 초기화 한다. 
******************************************************************************/
int InitTrade()
{  
    MARKET  *market;
    MDFOLD  *folder;
    if ((market = mds_open("CMBS", O_RDWR)) == NULL)
    {
        return(-2);
    }
    /*-------------------------
     * data set USDKRW 만 가상잔량 반영으로 해당 폴더만 반영 
     * -----------------------*/
    if ((folder = mds_getfolder(market, "USDKRW")) == NULL)
    {
        return(-3);
    }


    MDTRAD  *mdtrad; 
    mdtrad = &folder->trad;
    //1.메모리에 인덱스 초기화.  
    mdtrad->nEnd = 0;
    mdtrad->nStart = 0;
    mds_close(market);
    return(0);
}


/******************************************************************************
* Function Name : int InitMarketDate()
* Description   : 일자변경시 작업할내용을 넣어준다. 
******************************************************************************/
int InitMarketDate(MARKET *market)
{
	int nRet = 0;
	//1.전일 환율을 반영한다. 
	nRet = initBasePriceData();
	if (nRet == 0)
		mds_log(market, LOG_MUST, "[%-15s][%4d] InitBase Data OK", __FUNCTION__, __LINE__);
	else
	{
		mds_log(market, LOG_MUST, "[%-15s][%4d] InitBase Data Fail [%d]", __FUNCTION__, __LINE__, nRet);
		return -1;
	}
	//2. 스왑 환율을 메모리에 저장한다. 
	nRet = initSwapPriceData();
	if (nRet == 0)
		mds_log(market, LOG_MUST, "[%-15s][%4d] initSwapPriceData Data OK", __FUNCTION__, __LINE__);
	else
	{
		mds_log(market, LOG_MUST, "[%-15s][%4d] initSwapPriceData Data Fail [%d]", __FUNCTION__, __LINE__, nRet);
		return -1;
	}
    //3. 마크업 가격을 메모리에 저장한다.  
    nRet = initMarkupPriceData();
	if (nRet == 0)
		mds_log(market, LOG_MUST, "[%-15s][%4d] initMarkupPriceData Data OK", __FUNCTION__, __LINE__);
	else
	{
		mds_log(market, LOG_MUST, "[%-15s][%4d] initMarkupPriceData Data Fail [%d]", __FUNCTION__, __LINE__, nRet);
		return -1;
	}
	//4. 가격원천 정보를 메모리에 저장한다.
	nRet = initPriceOrign();
	if (nRet == 0)
		mds_log(market, LOG_MUST, "[%-15s][%4d] initPriceOrign Data OK", __FUNCTION__, __LINE__);
	else
	{
		mds_log(market, LOG_MUST, "[%-15s][%4d] initPriceOrign Data Fail [%d]", __FUNCTION__, __LINE__, nRet);
		return -1;
	}
	
	nRet = InitTrade();
	if (nRet == 0)
		mds_log(market, LOG_MUST, "[%-15s][%4d] InitTrade Data OK", __FUNCTION__, __LINE__);
	else
	{
		mds_log(market, LOG_MUST, "[%-15s][%4d] InitTrade Data Fail [%d]", __FUNCTION__, __LINE__, nRet);
	}
		
	
	return 0;
}


/******************************************************************************
* Function Name : int CheckStopPriceTime()
* Description   : 시간체크후 중지시간이후는 거래불가상태로 변경. 
******************************************************************************/
int CheckStopPriceTime()
{
	MARKET  *market2;
	MDFOLD  *folder;
	market2 = mds_open("CMBS", O_RDWR);
	if (market2 == NULL)
		return 0;
	mds_setfolder(market2, NULL);
	while ((folder = mds_popfolder(market2)) != NULL)
	{
		MDMSTR *m = &folder->mstr;
		if (m == NULL)
			continue;

		uint32_t ymd, hms;
		mds_time(market2, 0, &ymd, &hms, NULL, NULL);  // get local date & time

		if ((hms > m->stoptime) && (m->stoptime > 200))
		{
			if (m->trdf != 0)
			{
				mds_log(market2, LOG_MUST, "[%-15s][%4d] Symbol [%s] nowtime [%d] stoptime [%d] ", __FUNCTION__, __LINE__, m->symb, hms, m->stoptime);
				m->trdf = 0;
			}
		}
	}
	mds_close(market2);
	return 0;
}
