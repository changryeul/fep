#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#ifdef	SunOS
#include <sys/sockio.h>
#endif

#define	MAX_IF	8

static 	int getifconfig();

static	struct	ifconfig {
	char	ifnm[16];			// ifname
	char	mask[24];			// net mask
	char	ipad[24];			// ip address
	char	bcad[24];			// broadcasting ipad
} ifconfig[MAX_IF];

static	int	n_if = 0;			// no of ifconfig

//
// getinterface()
// Interface checking
//
int getinterface(char *ipad, char *ifnm, char *ifba)
{
	struct  in_addr me, ck;
	int	howto;
	int	ii;

	if (n_if <= 0)
		getifconfig();
	me.s_addr = 0;
	if (ipad != NULL && strlen(ipad) > 0)
	{
		howto = 0;
		me.s_addr = inet_addr(ipad);
	}
	else if (ifnm != NULL && strlen(ifnm) > 0)
		howto = 1;
	else if (ifba != NULL)
		howto = 2;
	else
		return(-1);

	for (ii = 0; ii < n_if; ii++)
	{
		switch (howto)
		{
		case 0: // by ip address
			ck.s_addr = inet_addr(ifconfig[ii].ipad);
			if (me.s_addr != ck.s_addr)
				break;
			if (ifnm != NULL)
				strcpy(ifnm, ifconfig[ii].ifnm);
			if (ifba != NULL)
				strcpy(ifba, ifconfig[ii].bcad);
			return(0);
		case 1: // by interface name
			if (strcasecmp(ifconfig[ii].ifnm, ifnm) != 0)
				break;
			if (ipad != NULL)
				strcpy(ipad, ifconfig[ii].ipad);
			if (ifba != NULL)
				strcpy(ifba, ifconfig[ii].bcad);
			return(0);
		case 2: // no ip addrsss && no interface name
			if (strlen(ifconfig[ii].bcad) > 0)
			{
				strcpy(ifba, ifconfig[ii].bcad);
				return(0);
			}
		}
	}
	return(-1);
}

//
// getbroadaddr()
// Get broadcasting address
//
int getbroadaddr(char *broadcast_addr)
{
	struct	hostent *hostent;
	char	hostname[40];
	struct	in_addr in_addr;
	char	*addr, ipad[32];

	if (gethostname(hostname, sizeof(hostname)) != 0)
		return(-1);
	if ((hostent = gethostbyname(hostname)) == NULL)
		return(-1);
	memcpy(&in_addr, hostent->h_addr, hostent->h_length);
	addr = inet_ntoa(in_addr);
	strcpy(ipad, addr);
	return(getinterface(ipad, NULL, broadcast_addr));
}


//
// get configured inet config
//
static int getifconfig()
{
	struct	ifconf	ifconf;
	struct	ifreq	ifreq[MAX_IF];
	struct	sockaddr_in *socknet;
	int	sock, retv;
	char	*ipad;
	int	ii;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	ifconf.ifc_req = ifreq;
	ifconf.ifc_len = sizeof(struct ifreq) * MAX_IF;
	retv = ioctl(sock, SIOCGIFCONF, &ifconf);
	close(sock);
	if (retv != 0)
		return(0);

	n_if = ifconf.ifc_len / sizeof(struct ifreq);
	for (ii = 0; ii < n_if; ii++)
	{
		strcpy(ifconfig[ii].ifnm, ifreq[ii].ifr_name);
		socknet = (struct sockaddr_in *)&ifreq[ii].ifr_addr;
		ipad = inet_ntoa(socknet->sin_addr);
		strcpy(ifconfig[ii].ipad, ipad);
#ifdef	LINUX
		socknet = (struct sockaddr_in *)&ifreq[ii].ifr_netmask;
		ipad = inet_ntoa(socknet->sin_addr);
		strcpy(ifconfig[ii].mask, ipad);
#endif
		socknet = (struct sockaddr_in *)&ifreq[ii].ifr_broadaddr;
		ipad = inet_ntoa(socknet->sin_addr);
		strcpy(ifconfig[ii].bcad, ipad);
	}
	return(0);
}
