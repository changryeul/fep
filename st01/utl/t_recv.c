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

char    *Get_MicroTime(char *p_time);
char    t_time[20];

int main(int argc, char *argv[]) {
    int     Sockfd, rcvfd;
    int     rt, i;
    const   int on=1;
    struct  timeval timeout;
    fd_set  read_set;
    char    R_Buff[1024];

    printf("start\n");
    Sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (Sockfd < 0) {
        printf("socket[%d] {%d:%s}\n", Sockfd, errno, strerror(errno));
        return -1;
    }
    printf("socket created:Sockfd[%d]\n", Sockfd);

    /* Set Socket Option */
    rt = setsockopt(Sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof (on));
    if (rt < 0) {
        printf("setsockopt[%d] {%d:%s}\n", rt, errno, strerror(errno));
        shutdown(Sockfd, SHUT_RDWR);
        close(Sockfd);
        return 1;
    }

    /* Bind */
    struct sockaddr_in  address;

    memset((char *)&address, 0, sizeof (address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(3222);

    rt = bind(Sockfd, (struct sockaddr *)&address, sizeof (struct sockaddr));
    if (rt < 0) {
        printf("bind[%d][%d] {%d:%s}\n", Sockfd, rt, errno, strerror(errno));
        close(Sockfd);
        return 1;
    }
    printf("bind[%d]\n", Sockfd);

    listen(Sockfd, 5);
    printf("listen[%d]\n", Sockfd);

    printf("accepting.... \n");
    int     Newfd;
    Newfd = accept(Sockfd, (struct sockaddr *)NULL, NULL);
    if (Newfd < 0) {
        printf("accept[%d][%d] {%d:%s}\n", Newfd, rt, errno, strerror(errno));
        close(Sockfd);
        return 1;
    }

    int size = 100000;

    /* read */
    while (1) {
        FD_ZERO(&read_set);
        FD_SET(Newfd, &read_set);

        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        rt = select(Newfd+1, NULL, &read_set, NULL, &timeout);
        if (rt < 0) {
            printf("Select_Send:select failure[%d] {%d:%s}\n", rt, errno, strerror(errno));
            return 1;
        }

        if (!FD_ISSET(Newfd, &read_set)) {
            printf("Select_Send:FD_ISSET {%d:%s}\n", errno, strerror(errno));
            return 1;
        }

        memset(R_Buff, 0, sizeof (R_Buff));
        rt = recv(Newfd, R_Buff, sizeof(R_Buff), 0);
        Get_MicroTime(t_time);
        printf("Recv [%2.2s:%2.2s:%2.2s:%6.6s] R_Buff[%s][%d] rt[%d]\n",
                t_time, t_time+2, t_time+4, t_time+6, R_Buff, strlen(R_Buff), rt);
        if (rt < 0) {
            if (errno == ECONNRESET)
                printf("recv : socket rest\n");
            else
                printf("recv : length:receive fail[%d] {%d:%s}\n", rt, errno, strerror(errno));

            return;
        }
        else if (rt == 0) {
            printf("recv : socket disconnected[%d] {%d:%s}\n", rt, errno, strerror(errno));
            return;
        }
    }

    close(Newfd);
    close(Sockfd);
}

/*************************************************************************
    Function        : . get time to the unit of microsec
    Parameters IN   : .
    Parameters OUT  : . p_time  : time string
    Return Code     : . char * (time string)
*************************************************************************/
/*----------------------------------------------------------------------*/
char    *Get_MicroTime(char *p_time)
/*----------------------------------------------------------------------*/
{
    struct timeval  tv;
    struct tm       *date, date1;

    gettimeofday(&tv, NULL);
    date = (struct tm *)localtime_r(&(tv.tv_sec), &date1);

    /* HHMMSSmmmmmm (12) = 12 bytes  */
    sprintf(p_time, "%02d%02d%02d%06ld",
            date->tm_hour, date->tm_min, date->tm_sec, tv.tv_usec);

    return (p_time);
}   /* End of Get_MicroTime ()  */
