#ifndef NETUTIL_H
#define NETUTIL_H

#include <stddef.h>
#include <stdint.h>

int mk_server( int port, int backlog);
int tcp_connect(const char *host_ip, int port);
int set_nonblock(int fd);
int set_sock_timeout( int fd, int ms,int optname);
int readn_timed( int fd, void *buf, size_t  n, int timeout_ms);
int sendn_timed( int fd, void *buf, size_t  n, int timeout_ms);

#endif
