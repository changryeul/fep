#include "cid.h"
#include "handlers.h"
#include "log.h"
#include "netutil.h"
#include <unistd.h>
#include <stdio.h>

typedef struct
{
   char cid[11];
   int  fd;
   time_t expire_at ;
}CidEntry;

static CidEntry g_tbl[CID_TABLE_SIZE];

static int is_empty( CidEntry *e )
{
    return ( e->fd <= 0  || e->cid[0] == '\0');

}

static void clear_entry ( CidEntry *e)
{
   e->cid[0]= '\0';
   e->fd    =  0;
   e->expire_at = 0;
}


static int find_index_by_cid ( char *cid)
{
    int i=0;
    for (i=0;i<CID_TABLE_SIZE;i++)
	{
        if ( !is_empty( &g_tbl[i]) && strncmp(g_tbl[i].cid, cid, 10) == 0)
        {
             return i;
        }
    }
    return -1;
}

static int find_index_by_fd( int fd)
{
    
    int i=0;
    for (i=0;i<CID_TABLE_SIZE;i++)
	{
        if ( !is_empty( &g_tbl[i]) && g_tbl[i].fd == fd)
        {
             return i;
        }
    }
    return -1;
}

int cid_pop_expired( char *cid, int *fd_out)
{

    if(!cid || !fd_out) return -1;
    time_t now = time(NULL);
	int i;
    for (i=0; i<CID_TABLE_SIZE; i++)
	{
        if ( !is_empty( &g_tbl[i]) && 
		     g_tbl[i].expire_at > 0 && g_tbl[i].expire_at  <= now )
		{
//log_info( " *cid pop ... 1");
		    strncpy( cid, g_tbl[i].cid, 10); cid[10] = 0x00; 
			*fd_out = g_tbl[i].fd;
            clear_entry( &g_tbl[i]);
//log_info( " *cid pop ... 2");
			return 1;
        }
    }
	return 0;
}

void cid_table_init(void)
{
    memset(g_tbl,0, sizeof(g_tbl));
}

/* 등록 (꽉 차면 가장 오래된 항목 제거 후 재사용) */
int  cid_add(char * cid, int cfd)
{

    if( !cid || cfd <=0) return -1;

    time_t now = time(NULL);

    int idx = find_index_by_cid(cid);
	if( idx >= 0)
	{
	    g_tbl[idx].fd =cfd;
		g_tbl[idx].expire_at = now+CID_TTL_SEC;
		return 0;
    }

    int i; 
    for (i=0; i<CID_TABLE_SIZE; ++i)
	{
        if ( is_empty(&g_tbl[i]))
		{
		    strncpy(g_tbl[i].cid , cid, 10);
			g_tbl[i].cid[10] = '\0';
			g_tbl[i].fd = cfd;
			g_tbl[i].expire_at = now + CID_TTL_SEC;
			return 0;
         }
     }

    for (i=0; i<CID_TABLE_SIZE; ++i)
	{
        if ( !is_empty(&g_tbl[i]) 
		   && g_tbl[i].expire_at > 0 
		   && g_tbl[i].expire_at <= now )
		{
		    clear_entry(&g_tbl[i]);
		    strncpy(g_tbl[i].cid , cid, 10);
			g_tbl[i].cid[10] = '\0';

			g_tbl[i].fd = cfd;
			g_tbl[i].expire_at = now + CID_TTL_SEC;
			return 0;
         }
     }

     int victim = -1;
	 time_t oldest = now;
	 for( i = 0; i<CID_TABLE_SIZE; ++i)
	 {
	     if(victim < 0 || g_tbl[i].expire_at <oldest)
		 {
		     victim = i;
			 oldest= g_tbl[i].expire_at;
         }
      }

	  if( victim >= 0)
	  {
	        clear_entry(&g_tbl[victim]);

		    strncpy(g_tbl[i].cid , cid, 10);
			g_tbl[i].cid[10] = '\0';
			g_tbl[i].fd = cfd;
			g_tbl[i].expire_at = now + CID_TTL_SEC;
			return 0;
	  }
	  return -1;

}

int cid_find_fd( char *cid)
{

   if(!cid) return -1;

   time_t now = time(NULL);

   int idx = find_index_by_cid(cid);
   if( idx < 0) return -1;

   if( g_tbl[idx].expire_at > 0 && g_tbl[idx].expire_at <= now )
   {
       clear_entry( &g_tbl[idx]);
	   return -1;
   }

   return g_tbl[idx].fd;
}

void cid_remove( char *cid)
{
   if( !cid) return;
   int idx = find_index_by_cid( cid);

   if( idx >= 0) clear_entry(&g_tbl[idx]);
}

void cid_del_by_fd(int cfd)
{
	if( cfd <= 0) return;

    int idx = find_index_by_fd(cfd);
    if( idx >=0) clear_entry( &g_tbl[idx]);
}

void  cid_timeout_sweep(void)
{
    time_t now = time(NULL);

    int i;
    for (i=0;i<CID_TABLE_SIZE;++i)
	{
        if ( !is_empty( &g_tbl[i]) && 
		     g_tbl[i].expire_at > 0 && g_tbl[i].expire_at  <= now )
		{
            clear_entry( &g_tbl[i]);
        }
    }
}

int cid_add_with_ttl( char *cid, int client_fd, int ttl_sec)
{
    // if( cid || client_fd <= 0) return -1;

	if( ttl_sec <=0) ttl_sec = CID_TTL_SEC;

	time_t now = time(NULL);

    // log_info("cid add inext by cid [%s]",cid );
    // 1) 동일 cid 갱신
	int idx = find_index_by_cid( cid);
	if( idx >= 0) 
	{
	   g_tbl[idx].fd = client_fd;
	   g_tbl[idx].expire_at  = now + ttl_sec ;
    // log_info("cid add inext . fd[%d] [%d]",client_fd, g_tbl[idx].expire_at );
	   return 0;
    }
    // 2)  빈 슬롯
    int i; 
    for (i=0; i<CID_TABLE_SIZE; ++i)
	{
        if ( is_empty(&g_tbl[i]))
		{
		    strncpy(g_tbl[i].cid , cid, 10);
			g_tbl[i].cid[10] = '\0';
			g_tbl[i].fd = client_fd;
			g_tbl[i].expire_at = now + ttl_sec;
			return 0;
         }
     }

    // 3) 만료 슬롯 재사용
    for (i=0; i<CID_TABLE_SIZE; ++i)
	{
        if ( !is_empty(&g_tbl[i]) 
		   && g_tbl[i].expire_at > 0 
		   && g_tbl[i].expire_at <= now )
		{
		    clear_entry(&g_tbl[i]);
		    strncpy(g_tbl[i].cid , cid, 10);
			g_tbl[i].cid[10] = '\0';

			g_tbl[i].fd = client_fd;
			g_tbl[i].expire_at = now + ttl_sec;
			return 0;
         }
     }

     // 4) 희생 정책
     int victim = -1;
	 time_t oldest = now;
	 for( i = 0; i<CID_TABLE_SIZE; ++i)
	 {
	     if(victim < 0 || g_tbl[i].expire_at <oldest)
		 {
		     victim = i;
			 oldest= g_tbl[i].expire_at;
         }
      }

	  if( victim >= 0)
	  {
	        clear_entry(&g_tbl[victim]);

		    strncpy(g_tbl[i].cid , cid, 10);
			g_tbl[i].cid[10] = '\0';
			g_tbl[i].fd = client_fd;
			g_tbl[i].expire_at = now + ttl_sec;
			return 0;
       }
	   return -1;
}


int cid_get_fd( char *cid)
{
     int i;
	 for( i = 0; i<CID_TABLE_SIZE; i++)
	 {
			if( strncmp(g_tbl[i].cid , cid, 10) == 0) 
			{
			     return g_tbl[i].fd;
            }
      }
	  return -1;
}	     

static uint32_t rand_u32(void)
{
    uint32_t a = (uint32_t)rand();
	uint32_t b = (uint32_t)rand();
		    return (a << 16) ^ b;
}

void cid_generate( char *out_cid )
{
     char cidbuf[11];
     uint32_t cid = rand_u32();
     
	 snprintf( cidbuf, sizeof(cidbuf),"%010u", cid); cidbuf[10] = 0x00;
     memcpy( out_cid, cidbuf, 10);
}
