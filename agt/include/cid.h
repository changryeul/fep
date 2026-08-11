#ifndef CID_H
#define CID_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "protocol.h"
          
#define CID_TTL_SEC    10
#define CID_TABLE_SIZE 4096


void  cid_table_init(void);
int   cid_add( char *cid, int client_fd );
int   cid_find( char *cid );
int   cid_find_fd( char *cid );
void  cid_remove( char *cid );
void  cid_timeout_sweep( void);
int   cid_pop_expired( char *cid, int *fd_out);
int   cid_add_with_ttl( char *cid, int client_fd, int ttl_sec);
void  cid_generate( char *cid_out);
int   cid_get_fd( char *cid);
 


#endif
