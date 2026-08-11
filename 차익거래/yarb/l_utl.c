#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <time.h>
#include <sys/msg.h>
#include <stdarg.h>
#include <iconv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <zlib.h>
#include "def.h"

void l_sleep(int time)
{
	struct pollfd w_fdtbl;
	if (poll(&w_fdtbl, 0, time) < 0) return;
}

#define MAX_ALLOC_RETRY_CNT 100

void  *l_alloc(int size) 
{
	int retry_cnt = 0;
	void *ptr = NULL;

malloc_retry:
	ptr = malloc(size);
	if (ptr == NULL) {
		if (retry_cnt > MAX_ALLOC_RETRY_CNT) {
			l_dbg(L_ERR, "l_alloc failed!! errno[%d] size[%d]", errno, size);

			l_sleep(100);
			exit(-1);
		}

		retry_cnt++;
		goto malloc_retry;
	}
	return(ptr);
}

void l_free(void *rbuf) 
{
	free(rbuf);
}

int l_stoi(char *in, int ilen) 
{
	int ii, out = 0;
	for (ii = 0; ii < ilen; ii++) {
		if (in[ii] < '0' || in[ii] > '9') break;
		out = (out*10) + (in[ii]-'0');
	}
	return(out);
}

void l_itos(int val, char *out, int olen) 
{
	int minus = 0, ii;

	if (val < 0) {
		minus = 1;
		val *= (-1);
	}

	for (ii = olen; ii > 0; ii--) {
		out[ii-1] = (unsigned char)(val%10)+'0';
		val /= 10;
	}

	if (minus) {
		for (ii = 0; ii < olen; ii++) {
			if (out[ii] != '0') break;
			out[ii] = ' ';
		}
		out[ii] = '-';
	}
	return;
}

int l_create_shm(int shmkey, int shmsz) 
{
	int shmid;
	shmid = shmget(shmkey, shmsz, 0666|IPC_CREAT);
	if (shmid < 0) {
		return (-1);
	}
	return(shmid);
}

int l_open_shm(int shmkey, int shmsz) 
{
	int shmid;
	shmid = shmget(shmkey, shmsz, 0);
	if (shmid < 0) {
		return (-1);
	}
	return(shmid);
}

void l_getdate(char *buf) 
{
	time_t tv;
	struct tm tp;
	tv = time(NULL);
	localtime_r(&tv, &tp);
	sprintf(buf, "%04d%02d%02d%02d%02d%02d", tp.tm_year+1900, tp.tm_mon+1, tp.tm_mday, tp.tm_hour, tp.tm_min, tp.tm_sec);
	return;
}

struct msg_buf {
	long mtype;
	char mtext[L_DATA_LEN+1];
};

int l_que_snd(qid, qdata, qdatalen, qtry_cnt)
	int qid;
	void *qdata;
	int qdatalen;
	int qtry_cnt;
{
	int flg, len, cnt;

	if (qdatalen > L_DATA_LEN) return(-2);
	flg = (qtry_cnt == 0) ? 0 : IPC_NOWAIT;
	while(1) {
		len = msgsnd(qid, qdata, (size_t)qdatalen, flg);
		if (len < 0) {
			if (qtry_cnt && (errno == EAGAIN)) {
				if (qtry_cnt > ++cnt) {
					l_sleep(1000);
					continue;
				}
				l_dbg(L_ERR, "msgsnd fail qid=%d errno=%d(%s) mtype=%ld sized=%d retry=%d/%d"
										, qid, errno, strerror(errno), *(long*)qdata, qdatalen, cnt, qtry_cnt);
				return(-1);
			} else {
				l_dbg(L_ERR, "msgsnd fail qid=%d errno=%d(%s) mtype=%ld sized=%d retry=%d/%d"
										, qid, errno, strerror(errno), *(long*)qdata, qdatalen, cnt, qtry_cnt);
				return(-1);
			}
		}
		return 0;
	}
}

int l_que_rcv(qid, qdata, qtry_cnt)
	int qid;
	void *qdata;
	int qtry_cnt;
{
	int flg, len;
	int cnt = 0;
	struct msg_buf mbuf;

	if (qdata == NULL) return(-2);
	mbuf.mtype = 1;
	flg = (qtry_cnt == 0) ? 0 : IPC_NOWAIT;

	while(1) {
		len = msgrcv(qid, &mbuf, L_DATA_LEN, 0, flg);
		if (len > 0) break;
		switch (errno) {
			case EINTR:
				return(0);
			case ENOMSG:
				if (qtry_cnt > ++cnt) {
					l_sleep(1000);
					continue;
				}
				return(-1);
			default:
				return(-1);
		}
	}
	memcpy(qdata, mbuf.mtext, len);
	return(len);
}

int l_que_create(qkey) 
	int qkey;
{
	int qid;
	qid = msgget(qkey, IPC_CREAT|IPC_EXCL|0666);
	if (qid < 0 && errno == EEXIST) {
		qid = msgget(qkey, 0666);
	}

	return(qid);
}

int l_que_open(qkey)
	int qkey;
{
	return(msgget(qkey,0666));
}

void l_remove_spaces(char *str) 
{
	int i = 0, j = 0;
	while (str[i]) {
		if (str[i] != ' ') {
			str[j++] = str[i];
		}
		i++;
	}
	str[j] = '\0';
}

void l_rtrim(char *str) 
{
	int i = 0;
	int len = strlen(str);
	int start = 0;
	int end = len;

	while ((start < end) && (str[end-1] <= ' ')) {
		end--;
	}

	if (start > end) {
		memset(str, '\0', len);
	}

	for (i = 0; i < end - start; i++) {
		str[i] = str[i + start];
	}

	for ( ; i < len; i++) {
		str[i] = '\0';
	}
}

void l_ltrim(char *str) 
{
	int i = 0;
	int len = strlen(str);
	int start = 0;
	int end = len;

	while ((start < end) && (str[end-1] <= ' ')) {
		start++;
	}

	if (start > end) {
		memset(str, '\0', len);
	}

	for (i = 0; i < end - start; i++) {
		str[i] = str[i + start];
	}

	for ( ; i < len; i++) {
		str[i] = '\0';
	}
}

void l_trim(char *str) {
	l_ltrim(str);
	l_rtrim(str);
}

