/*------------------------------------------------------------------------
#   Module  : Shared utility functions
#   File    : udp_init.c
------------------------------------------------------------------------*/

#include    "fep_fepp.h"
#include    <sys/socket.h>
#include    <netinet/in.h>
#include    <arpa/inet.h>

/*------------------------------------------------------------------------
    Function    : init_udp_socket
    Description : Create UDP socket, set options, bind to client address
    Parameters  : svr_addr    - server address struct to fill
                  clnt_addr   - client address struct to fill
                  svr_ip      - server IP string (dotted notation)
                  svr_port    - server port number
                  sndbuf_size - SO_SNDBUF size in bytes (0 to skip)
                  rcvbuf_size - SO_RCVBUF size in bytes (0 to skip)
                  broadcast   - 1 to enable SO_BROADCAST, 0 to skip
    Return      : socket fd on success, -1 on failure
------------------------------------------------------------------------*/
int init_udp_socket(struct sockaddr_in *svr_addr, struct sockaddr_in *clnt_addr,
        const char *svr_ip, int svr_port,
        int sndbuf_size, int rcvbuf_size, int broadcast) {
    int     sockfd, rt;
    int     optval, optlen;

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        Log(UDP_FATAL, "Socket Open Error[%d:%s]", SYS_NO, SYS_STR);
        return -1;
    }

    svr_addr->sin_family      = AF_INET;
    if (inet_pton(AF_INET, svr_ip, &svr_addr->sin_addr) != 1) {
        Log(UDP_FATAL, "inet_pton fail svr_ip[%s] {%d:%s}", svr_ip, SYS_NO, SYS_STR);
        close(sockfd);
        return -1;
    }
    svr_addr->sin_port        = htons(svr_port);

    clnt_addr->sin_family      = AF_INET;
    clnt_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    clnt_addr->sin_port        = htons(0);

    optlen = sizeof(optval);

    if (sndbuf_size > 0) {
        optval = sndbuf_size;
        rt = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (void *)&optval, optlen);
        if (rt < 0)
            Log(UDP_WARN, "setsockopt SO_SNDBUF fail {%d:%s}", SYS_NO, SYS_STR);
    }

    if (rcvbuf_size > 0) {
        optval = rcvbuf_size;
        rt = setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (void *)&optval, optlen);
        if (rt < 0)
            Log(UDP_WARN, "setsockopt SO_RCVBUF fail {%d:%s}", SYS_NO, SYS_STR);
    }

    if (broadcast) {
        optval = 1;
        rt = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (void *)&optval, optlen);
        if (rt < 0)
            Log(UDP_WARN, "setsockopt SO_BROADCAST fail {%d:%s}", SYS_NO, SYS_STR);
    }

    rt = bind(sockfd, (struct sockaddr *)clnt_addr, sizeof(*clnt_addr));
    if (rt < 0) {
        Log(UDP_FATAL, "Socket Bind Error[%d:%s]", SYS_NO, SYS_STR);
        close(sockfd);
        return -1;
    }

    return sockfd;
}
