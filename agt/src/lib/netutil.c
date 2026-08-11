#define _POSIX_C_SOURCE 200809L
#include "netutil.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <poll.h>

static int wait_io(int fd, int events, int timeout_ms)
{
    struct pollfd p = { .fd=fd, .events = events };
    int r = poll( &p,1,timeout_ms);
    if( r == 0) {
       errno = ETIMEDOUT;
       return -ETIMEDOUT;
    }
    if( r < 0 ) return -1;
    if( p.revents & (POLLERR |POLLHUP |POLLNVAL))
    {
        errno = ECONNRESET;
        return -1;
    }
    return 0;
}



int set_nonblock(int fd){
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl < 0) return -1;
    if (fcntl(fd, F_SETFL, fl | O_NONBLOCK) < 0) return -1;
    return 0;
}

int mk_server(int port, int backlog)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) 
	    return -1;
    int yes=1; 
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in a={0};
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.sin_port = htons(port);

    if (bind(fd, (struct sockaddr*)&a, sizeof(a))<0) 
	    return -1;
    if (listen(fd, backlog)<0) 
	    return -1;

    return fd;
}

int tcp_connect(const char *host_ip, int port)
{

    struct sockaddr_in a={0};

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    a.sin_family = AF_INET;
    a.sin_port = htons(port);
    if (inet_pton(AF_INET, host_ip, &a.sin_addr) != 1)
	{ 
    	close(fd); 
		return -1; 
	}
    if (connect(fd, (struct sockaddr*)&a, sizeof(a))<0)
	{ 
		close(fd); 
		return -1; 
	}
    return fd;
}

int readn_timed( int fd, void *buf, size_t n, int timeout_ms)
{
    size_t off = 0;
    char *p = (char*)buf;

    while (off < n)
    {

        int rc = wait_io (fd, POLLIN, timeout_ms);
        if( rc < 0) return rc;

        ssize_t r = read(fd, p + off, n - off);

        if (r >  0) { off += (size_t)r; continue; }
        if (r == 0)
        {
             errno = 0;
             return -2;
        }
        if (rc == -1 && (errno == EAGAIN ||  errno == EWOULDBLOCK || errno == EINTR )) continue;
        return -1;

     }
     return 0;
}

int sendn_timed( int fd, void *buf, size_t n, int timeout_ms)
{
    const char *p = (char *)buf;
    size_t off = 0;

    while ( off < n )
    {
        int rc = wait_io ( fd, POLLOUT,timeout_ms );
        if( rc < 0 ) return rc;

        ssize_t w = write( fd, p+ off, n-off);
        if( w> 0)
        {
           off += (size_t)w;
           continue;
        }
        if (w  == -1 && (errno == EAGAIN ||  errno == EWOULDBLOCK || errno == EINTR )) continue;

        return -1;
    }
    return 0;
}


