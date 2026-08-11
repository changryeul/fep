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
    fd_set  write_set;
    char    S_Buff[1024], tmp[1024];

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
    address.sin_port = htons(3222);

    rt = connect(Sockfd, (struct sockaddr *)&address, sizeof (struct sockaddr));
    printf("connect ok \n");

    i = 0;
    while (i < 10) {
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

        memset(S_Buff, 0, sizeof (S_Buff));
        sprintf(S_Buff, "%s%02d", "TEST SEND", ++i);
        Get_MicroTime(t_time);
        printf("Recv [%2.2s:%2.2s:%2.2s:%6.6s] S_Buff[%s][%d] rt[%d]\n",
                t_time, t_time+2, t_time+4, t_time+6, S_Buff, strlen(S_Buff), rt);
        rt = send(Sockfd, S_Buff, strlen(S_Buff), 0);
        if (rt <= 0) {
            printf("Select_Send:send failure[%d] {%d:%s}\n", errno, strerror(errno));
            return 1;
        }
        sleep(1);
    }

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
