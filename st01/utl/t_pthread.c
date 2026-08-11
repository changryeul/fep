#include    <stdio.h>
#include    <signal.h>
#include    <stdlib.h>
#include    <string.h>
#include    <strings.h>
#include    <errno.h>
#include    <time.h>
#include    <fcntl.h>
#include    <unistd.h>
#include    <memory.h>
#include    <stdarg.h>
#include    <time.h>
#include    <poll.h>
#include    <pthread.h>

char        *Get_MicroTime(char *);
void        *Thread_Read();
void        *Thread_Send();

pthread_t   Read_t;
pthread_t   Send_t;

pthread_mutex_t Mutex;
pthread_cond_t  Cond;

int     W_cnt, R_cnt;
char    Time[20];

int main(int argc, char *argv[]) {
    printf("Start\n");

    pthread_mutex_init(&Mutex, NULL);
    pthread_cond_init(&Cond, NULL);

    W_cnt = R_cnt = 0;

    pthread_create(&Send_t, NULL, Thread_Send, NULL);
    pthread_create(&Read_t, NULL, Thread_Read, NULL);

    pthread_join(Send_t, NULL);
    pthread_join(Read_t, NULL);

    printf("All End!!!\n");
}

void *Thread_Read() {
    int     i;

    for (i = 0; i < 10; i++) {
        Get_MicroTime(Time);
        printf("Send Cond_Signal Time = [%s]\n", Time);
        pthread_cond_signal(&Cond);

        W_cnt++;
        sleep(1);
    }
    printf("Read End!!!\n");
}

void *Thread_Send() {
    int     k, f = -1;

    pthread_mutex_lock(&Mutex);

    /*
        Get_MicroTime (Time);
        printf ("Before SEND Time = [%s]\n", Time);
    */

    for (k = 0; k < 10; k++) {
        f = pthread_cond_wait(&Cond, &Mutex);

        Get_MicroTime(Time);
        printf("After SEND Time = [%s]\n", Time);
        printf("f = [%d]\n", f);
        if (f > -1) {
            pthread_mutex_unlock(&Mutex);
            R_cnt++;
            printf("k[%d] W_cnt[%d] R_cnt[%d]\n", k, W_cnt, R_cnt);
        }
    }
    printf("Send End\n");
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
