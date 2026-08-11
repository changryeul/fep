/*------------------------------------------------------------------------
#	Module	: connect to a socket (TCP/IP)
#	File	: tcpip_connect.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"

/*************************************************************************
	Function		: . connect to a socket
	Parameters IN	: . p_sfd		: file descriptor associated with the socket
					  . p_ip_addr	: IP address
					  . p_port_no	: port number
	Prameters OUT	: .
	Return Code		: . int (0: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Connect (int p_sfd, char *p_ip_addr, int p_port_no)
/*----------------------------------------------------------------------*/
{
	int					rt;
	struct sockaddr_in	address;

	memset ((char *)&address, 0, sizeof (address));

	address.sin_family = AF_INET;
/*
	address.sin_addr.s_addr = inet_addr (p_ip_addr);
*/
	inet_pton(AF_INET, p_ip_addr, &address.sin_addr.s_addr);
	address.sin_port = htons (p_port_no);

	rt = connect (p_sfd, (struct sockaddr *)&address, sizeof (struct sockaddr));

	return (rt);
}	/* End of Connect ()	*/

/*************************************************************************
    Function        : . connect to a socket
    Parameters IN   : . p_sfd       : file descriptor associated with the socket
                      . p_ip_addr   : IP address
                      . p_port_no   : port number
                      . p_timeout   : alarm timeout
    Prameters OUT   : .
    Return Code     : . int (0: success, -1: failure)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Connect2 (int p_sfd, char *p_ip_addr, int p_port_no, int p_timeout)
/*----------------------------------------------------------------------*/
{
    int                 rt;
    struct sockaddr_in  address;

    memset ((char *)&address, 0, sizeof (address));

    address.sin_family = AF_INET;
/*
    address.sin_addr.s_addr = inet_addr (p_ip_addr);
*/
	inet_pton(AF_INET, p_ip_addr, &address.sin_addr.s_addr);
    address.sin_port = htons (p_port_no);

    alarm (p_timeout);

    rt = connect (p_sfd, (struct sockaddr *)&address, sizeof (struct sockaddr));
    if (rt != 0)
    {
        alarm (0);

        return (-1);
    }

    alarm (0);

    return (rt);
}   /* End of Connect2 ()   */
/*************************************************************************
	End of Program (tcpip_connect.c)
*************************************************************************/
