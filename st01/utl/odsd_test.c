#include    <arpa/inet.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <pthread.h>
#include    <errno.h>
#include    <sys/socket.h>
#include    <sys/stat.h>
#include    <sys/select.h>
#include    <sys/uio.h>
#include    <time.h>

int main(int argc, char *argv[]) {
    int     Sockfd, rcvfd;
    int     rt, i;
    const   int on=1;
    struct  timeval timeout;
    fd_set  write_set;
    char    SendPkt[1024], tmp[1024];

    if (argc != 2) {
        printf("Should Check the argument count. We are Allow 2 argc\n");
        printf("Ex) t_mtsk_cli 2\n");
        exit(1);
    }

    printf("start\n");
    Sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (Sockfd < 0) {
        printf("socket[%d] {%d:%s}\n", Sockfd, errno, strerror(errno));
        return -1;
    }
    printf("socket created:Sockfd[%d]\n", Sockfd);

    /* connect */
    struct  sockaddr_in address;

    memset((char *)&address, 0, sizeof (address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(32011);

    rt = connect(Sockfd, (struct sockaddr *)&address, sizeof (struct sockaddr));
    printf("connect ok \n");

    FD_ZERO(&write_set);
    FD_SET(Sockfd, &write_set);

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    rt = select(Sockfd+1, NULL, &write_set, NULL, &timeout);
    if (rt < 0) {
        printf("Select_Send:select failure[%d] {%d:%s}\n", rt, errno, strerror(errno));
        return 1;
    }

    if (!FD_ISSET(Sockfd, &write_set)) {
        printf("Select_Send:FD_ISSET {%d:%s}\n", errno, strerror(errno));
        return 1;
    }

    /* LINK */
    memset(tmp, 0, sizeof (tmp));
    sprintf(tmp, "0046LINK000020211015%08d                      ", atoi(argv[1]));
    memset(SendPkt, 0, sizeof (SendPkt));
    memcpy(SendPkt, tmp, strlen(tmp));
    rt = send(Sockfd, SendPkt, strlen(SendPkt), 0);
    if (rt <= 0) {
        printf("select_Send:send failure[%d] {%d:%s}\n", rt, errno, strerror(errno));
        return 1;
    }
    sleep(3);

    /* ORDER */
    memset(tmp, 0, sizeof (SendPkt));
    sprintf(tmp, "0307LINK000020211015%08d                      00000000001TCHODR10001G100010029991530000001          KR70059300031100000000182400000001230000007050020000000000000    0011030     00            000041010000011720172121276805CAE61CDC202110261317010280101                                                        0", atoi(argv[1]));
    memset(SendPkt, 0, sizeof (SendPkt));
    /* 주문번호 채번도 argv[1]추가 */
    memcpy(SendPkt, tmp, strlen(tmp));
    rt = send(Sockfd, SendPkt, strlen(SendPkt), 0);
    if (rt <= 0) {
        printf("select_Send:send failure[%d] {%d:%s}\n", rt, errno, strerror(errno));
        return 1;
    }
    sleep(5);

}
