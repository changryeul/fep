#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

/* For Queue */
#define READ_BUF_SIZE       1024
#define QUEUE_MAX_BYTES     65535
#define MAXSIZE             4096
#define PERM                0x1B6
#define WAIT_TIME           60
#define QUEUE_READY         -1
#define QUEUE_WAIT          -2
#define QUEUE_FULL          -3
#define QUEUE_NOT_EXIST     -4
#define QUEUE_TIME_OUT      -5
#define QUEUE_SEND_ERROR    -6
#define TRUE                1
#define FALSE               0

#define TCP_HEAD_LEN        50

int AtoIf(char *, int);
int Recvn(int, char *, int);
int Select_Receive_Cli(int, char *);

typedef struct{
    long mtype;
    unsigned char mtext[MAXSIZE];
} Msgbuf;

int     INT_SEQ;
char    Date[10];

int main(int argc, char *argv[]) {
    int     Sockfd, rcvfd;
    int     rt, i;
    const   int on=1;
    struct  timeval timeout;
    fd_set  write_set;
    fd_set  read_set;
    char    SendPkt[1024], R_Buff[1024], tmp[1024];

    printf("start\n");
    Sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(Sockfd < 0) {
        printf("socket[%d] {%d:%s}\n", Sockfd, errno, strerror(errno));
        close(Sockfd);
        exit(1);
    }
    printf("socket created:Sockfd[%d]\n", Sockfd);

    /* connect */
    struct sockaddr_in address;

    memset((char *)&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(32011);

    rt = connect(Sockfd, (struct sockaddr *)&address, sizeof(struct sockaddr));
    printf("connect ok \n");

    /* **************** */
    /* LINK             */
    /* Link 먼저 처리한다,    */
    /* seq와 date는 LIOK를 수신받아서 사용하고 LINK시에는 0으로 처리한다. */
    printf("OK01\n");
    memset(tmp, 0, sizeof(tmp));
    sprintf(tmp, "0046LINK0000%08d%08d                      ", 0, 0);
    memset(SendPkt, 0, sizeof(SendPkt));
    memcpy(SendPkt, tmp, strlen(tmp));
    rt = send(Sockfd, SendPkt, strlen(SendPkt), 0);
    if(rt <= 0) {
        printf("select_Send:send failure[%d] {%d:%s}\n", rt, errno, strerror(errno));
        close(Sockfd);
        exit(1);
    }
    else
        printf("Send OK [%s]\n", SendPkt);

    /* 수신 */
    memset(R_Buff, 0, sizeof(R_Buff));
    rt = Select_Receive_Cli(Sockfd, R_Buff);
    if(rt == 0) {
        if(errno == ECONNRESET)
            printf("recv : socket rest\n");
        else
            printf("recv : length:receive fail[%s] {%d:%s}\n", rt, errno, strerror(errno));

        close(Sockfd);
        exit(1);
    }
    else if(rt == -1) {
        printf("recv : socket disconnected[%d] {%d:%s}\n", rt, errno, strerror(errno));
        close(Sockfd);
        exit(1);
    }
    printf("Recv OK [%s]\n", R_Buff);

    /* 수신데이터 체크 */
    if(memcmp(&R_Buff[4], "LIOK", 4) != 0) {
        printf("TR CODE Error [%50.50s]\n", R_Buff);
        close(Sockfd);
        exit(1);
    }
    if(memcmp(&R_Buff[8], "0000", 4) != 0) {
        printf("Response CODE Error [%50.50s]\n", R_Buff);
        close(Sockfd);
        exit(1);
    }

    /* Set Seq && Date */
    memset(tmp, 0, sizeof(tmp));
    memcpy(tmp, &R_Buff[20], 8);
    INT_SEQ = atoi(tmp);
    memset(Date, 0, sizeof(Date));
    memcpy(Date, &R_Buff[12], 8);

    /* 여기서는 Poll처리는 하지않고 "Q수신"시 송신하고 응답받아서 ErrorCode 체크하고 반복한다. */
    /* Send Data */
    /* Make Q */
    int     msqid;
    key_t   new_key;
    new_key = 0x33000003;   /* <= 사용하실 Q주소로 변경해서 쓰세요*/

    msqid = MakeQueue(new_key);
    if(msqid < 0) {
        printf("msgget fail msqid[%d]\n", msqid);
        close(Sockfd);
        exit(1);
    }
    else
        printf("msgget OK msqid[%d]\n", msqid);

    while(1) {
        /* ******* */
        /* Q수신 대기 */
        printf("Rcv Wait!!! INT_SEQ[%d]\n", INT_SEQ);
        memset(R_Buff, 0, sizeof(R_Buff));
        rt = ReceiveQueue(msqid, 0L, &R_Buff);
        /* ******** */
        printf("Rcv Q [%s] [%d]\n", R_Buff, strlen(R_Buff));

        /* ******** */
        /* TCP Send */
        /* Q로 수신받아서 보낼 데이터 정합성 체크하고, 주문채번여기서 하던지 전단계에서 하던지 꼭 채번하시고 */
        memset(tmp, 0, sizeof(tmp));
        memset(SendPkt, 0, sizeof(SendPkt));
        memset(SendPkt, 0x20, 550);
        /* 송신할 Packet Size는 500으로 고정임, 앞에 사이즈정보는 총사이즈 - 4 = 496 */
        sprintf(tmp, "0546DATA0000%8.8s%08d                      ", Date, INT_SEQ+1);
        memcpy(SendPkt, tmp, strlen(tmp));
        memcpy(&SendPkt[50], R_Buff, strlen(R_Buff));
        printf("Befor SendPkt [%s][%d]\n", SendPkt, strlen(SendPkt));
        rt = send(Sockfd, SendPkt, strlen(SendPkt), 0);
        if(rt <= 0) {
            /* 송신오류발생시 Client에 오류난 사실을 알려야하며 그 로직을 만들어서 처리해야한다. */
            printf("select_Send:send failure[%d] {%d:%s}\n", rt, errno, strerror(errno));
            close(Sockfd);
            exit(1);
        }
        /* ********* */
        printf("SendPkt [%s]\n", SendPkt);

        /* ******** */
        /* TCP Recv */
        FD_ZERO(&read_set);
        FD_SET(Sockfd, &read_set);

        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        rt = select(Sockfd+1, NULL, &read_set, NULL, &timeout);
        if(rt < 0) {
            printf("Select_Send:select failure[%d] {%d:%s}\n", rt, errno, strerror(errno));
            close(Sockfd);
            exit(1);
        }

        if(!FD_ISSET(Sockfd, &read_set)) {
            printf("Select_Send:FD_ISSET {%d:%s}\n", errno, strerror(errno));
            close(Sockfd);
            exit(1);
        }

        memset(R_Buff, 0, sizeof(R_Buff));
        rt = Select_Receive_Cli(Sockfd, R_Buff);
        if(rt == 0) {
            if(errno == ECONNRESET)
                printf("recv : socket rest\n");
            else
                printf("recv : length:receive fail[%d] {%d:%s}\n", rt, errno, strerror(errno));

            close(Sockfd);
            exit(1);
        }
        else if(rt == -1) {
            printf("recv : socket disconnected[%d] {%d:%s}\n", rt, errno, strerror(errno));
            close(Sockfd);
            exit(1);
        }
        printf("Recv Data [%s]\n", R_Buff);

        if((memcmp(&R_Buff[4], "DAOK", 4) != 0) ||
                (memcmp(&R_Buff[8], "0000", 4) != 0)) {
            printf("Data Recv Response CODE Error [%50.50s]\n", R_Buff);
            close(Sockfd);
            exit(1);
        }
        /* ************** */
        if(memcmp(&R_Buff[4], "DAOK", 4) == 0)
            INT_SEQ++;
    }

}

int MakeQueue(KEY)
size_t KEY;
{
    char buff[MAXSIZE];
    Msgbuf msgbuf;
    int rtv, QID;

    QID = msgget(KEY, PERM|IPC_CREAT);
    if(QID == -1) return FALSE;

    return QID;
}

/*************************************************************
    Function        : . Receive message from queue
    Parameters IN   : . int QID;
                      . long owner;
                      . char *data;
    Parameters OUT  : .
    Return Code     : . void
    Comment         : . sleep for microsec
*************************************************************/
int ReceiveQueue(QID, owner, data)
int QID;
long owner;
char *data;
{
    int rtrn;
    Msgbuf msgbuf;

    msgbuf.mtype = (size_t) owner;
    rtrn = msgrcv(QID, &msgbuf, MAXSIZE, msgbuf.mtype, 0);
    if(rtrn > 0) {
        memcpy(data, msgbuf.mtext, rtrn);
        return rtrn;
    }
    else {
        return QUEUE_NOT_EXIST;
    }
}

/*************************************************************
    Function        : . select and receive a packet
    Parameters IN   : . p_sfd       : socket file descriptor
                      . p_recv      : data buffer
    Parameters OUT  : .
    Return Code     : . int (0:success, -1:failure, >0:packet length)
*************************************************************/
/*----------------------------------------------------------*/
int Select_Receive_Cli(int p_sfd, char *p_recv)
/*----------------------------------------------------------*/
{
    int             rt, pkt_len, rev_len;
    fd_set          read_set;
    struct timeval  timeout;

    FD_ZERO(&read_set);
    FD_SET(p_sfd, &read_set);

    timeout.tv_sec = 30;
    timeout.tv_usec = 0;

    rt = select(p_sfd+1, &read_set, NULL, NULL, &timeout);

    if(rt < 0) {
        printf("Select_Receive:select fail[%d] {%d:%s}\n", rt, errno, strerror(errno));
        return (-1);
    }

    if(!FD_ISSET(p_sfd, &read_set)) {
        printf("Select_Receive:select timeout\n");
        return (0);
    }

    rt = Recvn(p_sfd, p_recv, 4);
    if(rt < 0) {
        if(errno == ECONNRESET)
            printf("Select_Receive:socket reset\n");
        else
            printf("Select_Receive:length:receive fail[%d] {%d:%s}\n",
                    rt, errno, strerror(errno));

        return (-1);
    }
    else if(rt == 0) {
        printf("Select_Receive:socket disconnected[%d]", rt);
        return (0);
    }

    pkt_len = AtoIf(p_recv, 4);
    if(pkt_len < TCP_HEAD_LEN - 4)
        printf("Select_Receive:invalid Length[%d]", pkt_len);

    rt = Recvn(p_sfd, p_recv+4, pkt_len);
    if(rt < 0) {
        printf("Select_Receive:data:receive fail[%d] {%d:%s}",
                rt, errno, strerror(errno));
        return (-1);
    }

    return (pkt_len);
}   /* End of Select_Receive_Cli () */

/****************************************************************************
    Function        : . receive a message from a socket
    Parameters IN   : . p_sfd   : file descriptor associated with the socket
                      . p_len   : data size_t
    Parameters OUT  : . p_buf   : data buffer
    Return Code     : . int (numver of bytes received: success, -1: failure)
****************************************************************************/
/*----------------------------------------------------------*/
int Recvn(int p_sfd, char *p_buf, int p_len)
/*----------------------------------------------------------*/
{
    int     nleft, nrecv;

    nleft = p_len;

    while(nleft > 0) {
        nrecv = recv(p_sfd, p_buf, nleft, 0);
        if(nrecv < 0)
            return (nrecv);
        else if(nrecv == 0)
            break;

        nleft -= nrecv;
        p_buf += nrecv;
    }

    return (p_len - nleft);
}   /* End of Recvn () */

/*----------------------------------------------------------*/
int AtoIf(char *p_ascii, int p_len)
/*----------------------------------------------------------*/
{
    int     i, j, jj;

    j   = 0;
    jj  = 0;

    for(i = 0; i < p_len; i++) {
        for(j = 0; j < 10; j++) {
            if(*(p_ascii+i) == ('0' + j))
                break;
        }

        if(j < 10)
            jj = (jj * 10) + j;
    }

    return (jj);
}   /* End of AtoIf () */
