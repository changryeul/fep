#include <stdio.h>
#include <stdarg.h>
#include "context.h"


//void	mds_postmsg(MARKET *market, int exid, const char *msg)
void	mds_postmsg(int exid, const char *msg)
{
	XCHG	*xchg;
	struct	sockaddr_in sockudp;
	char	msgb[10*1024];
	int		msgl;
	int		sock;
	struct  timeval tv;
	struct  tm tm;
	struct {
		char filler1[30];
		char xymd[8];
		char xhms[6];
		char usec[6];
	} *header;

//	if (exid == market->exid)
//		return;
	xchg = mds_exchange_by_exid(exid);
	if (xchg == NULL)
		return;

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return;

	memset(&sockudp, 0, sizeof(sockudp));   
	sockudp.sin_family = AF_INET;       
	sockudp.sin_addr.s_addr = inet_addr("127.0.0.1");
	sockudp.sin_port = htons(xchg->from[0].port);

	gettimeofday(&tv, NULL);
	tm = *localtime(&tv.tv_sec);

	header = msgb;

	memset(header->filler1, 0x20, sizeof(header->filler1));
	sprintf(header->xymd, "%04d%02d%02d", tm.tm_year+1900, tm.tm_mon + 1, tm.tm_mday);
	sprintf(header->xhms, "%02d%02d%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
	sprintf(header->usec, "%06ld", tv.tv_usec);

	msgl = sizeof(*header);
	memcpy(&msgb[msgl], msg, strlen(msg));
	msgl += strlen(msg);
	
    sendto(sock, msgb, msgl, 0, (struct sockaddr *)&sockudp, sizeof(sockudp));
	close(sock);
}
