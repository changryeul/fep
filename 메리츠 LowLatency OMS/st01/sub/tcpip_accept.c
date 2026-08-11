/*------------------------------------------------------------------------
#	Module	: accept a connection on a socket (TCP/IP)
#	File	: tcpip_accept.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"

/*************************************************************************
	Function		: . accept a connection on a socket
	Parameters IN	: . p_sfd	: socket file descriptor
	Parameters OUT	: . p_ip	: client IP address
					  . p_port	: client port
	Return Code		: . int (a descriptor for the accepted socket: success,
						-1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Accept (int p_sfd, char *p_ip, u_short *p_port)
/*----------------------------------------------------------------------*/
{
	char				*hostname, *hostip;
	int					rt;
#if defined __hpux || _AIX || __linux
	socklen_t			addrlen;
#else
	size_t				addrlen;
#endif
	struct sockaddr_in	address;
	struct hostent		*hp;

#if defined __linux
    addrlen = sizeof (address);
#else
    addrlen = sizeof (struct sockaddr);
#endif
	memset ((char *)&address, 0, sizeof (address));

	rt = accept (p_sfd, (struct sockaddr *)&address, &addrlen);

	if (rt < 0)
		return (rt);

	hp = gethostbyaddr ((char *)&address.sin_addr,
		sizeof (struct in_addr), address.sin_family);
	hostip = (char *)inet_ntoa (address.sin_addr);
	*p_port = ntohs (address.sin_port);
	sprintf (p_ip, "%s", hostip);
#ifdef sun
	Log (TCP_OK, "request from (%s:%u)", hostip, *p_port);
#else
	hostname = hp->h_name;
	Log (TCP_OK, "request from %s (%s:%u)", hostname, hostip, *p_port);
#endif

	return (rt);
}	/* End of Accept ()	*/

/*************************************************************************
	End of Program (tcpip_accept.c)
*************************************************************************/
