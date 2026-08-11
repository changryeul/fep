#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>
#include <arpa/inet.h>

#include "protocol.h"
#include "log.h"


int recv_packet_fd_timed(int fd, CommHdr *ch_in, DataHdr *dh_in, void *body, int *body_len, int max_body, int timeout_ms )
{
	int      blen=0; 
    CommHdr *ch = ch_in;
	DataHdr *dh = dh_in;

#if 0
    if( !ch_in || !dh_in || !body || !body_len || max_body < 0)
	{
	    errno = EINVAL ;
		LOG_INFO ( " 1.. ");
		return -1;
    }
#endif

	int rc = readn_timed(fd, ch, sizeof(CommHdr ), timeout_ms) ;
	if( rc == -2) // eof
	{
	    LOG_INFO( "eof...."); // 재연결 필요할 것 같은데... 
		return -2;
		//return 0;
    }
    if ( rc < 0)  // error /timeout
	{
	    LOG_INFO( "error or timeout ");
	    return rc;
    }

	char lenbuf[5];
	memcpy( lenbuf, ch->Length, 4); lenbuf[4] = 0x00;
    
	int total =  atoi(lenbuf) +4;

    if( total < sizeof( CommHdr ))
	{
	   errno = EPROTO;
	   return -1;
	}

    if( total == sizeof(CommHdr))
	{
        LOG_INFO( "RD [%.50s](%d)", (void *)ch, sizeof(CommHdr) );
	    memset( dh, 0, sizeof(DataHdr));
		*body_len = 0;
		return 0;
	}

	if( total< sizeof(CommHdr) + sizeof(DataHdr))
	{
	   errno = EPROTO;
	   return -1;
	}

	rc = readn_timed(fd, dh, sizeof(DataHdr ), timeout_ms) ;
    if ( rc < 0)
	{
	    LOG_ERR("[PROTO] data head read error rc (%d) ", rc );
	    return rc;
    }

    int remain = total - sizeof(CommHdr) - sizeof(DataHdr);

	if( remain < 0)  remain = 0;

    int to_read = remain;
	if( to_read > 0)
	{
        rc = readn_timed(fd, body, to_read, timeout_ms );
        if( rc < 0)
	    {
	        LOG_WARN("[PROTO] read body timeout or failed  fd=%d read=%d ", fd ,blen);
	        return rc ;
        }
    }
	*body_len = to_read;

    LOG_INFO( "RD [%s%.*s](%d)", (void*)dh , to_read, body,total );
    return 0;
}
//	LOG_INFO("recv ...total len=%d = %d %d %d ",  total, 50, 50, remain );

#if 0
    log_debug(" in  function com head ptr = %p ", (void *)ch );
    log_debug(" in  function dat head ptr = %p ", (void *)dh );
    log_debug(" in  function dat data ptr = %p ", body );
#endif



int send_packet_fd_timed(int fd,  CommHdr *ch_in, DataHdr *dh_in, void *body, int body_len, int timeout_ms )
{
    int rc ;

    if ( !ch_in || !dh_in )
	{
	    errno = EINVAL;
		return -1;
    }

    CommHdr ch = *ch_in;
	DataHdr dh = *dh_in;

    int total = 50 + strlen(dh_in) + body_len ;

	//LOG_INFO("send ...total len=%d = %d %d %d ",  total, 50, strlen(dh_in), body_len );
    char lenbuf[5];

    snprintf( lenbuf, sizeof(lenbuf), "%04d", total - 4); lenbuf[4] = 0x00;
    memcpy( ch.Length , lenbuf, 4);

    char buf[4096];

	memcpy( buf , &ch, sizeof(CommHdr));
	if( total > 50)
	{
	    memcpy( buf + sizeof(CommHdr), &dh, sizeof(DataHdr));
	}

	if( body_len > 0 && body)
	{
	    memcpy( buf + sizeof(CommHdr) + sizeof(DataHdr) , body, body_len );
    }

    // LOG_INFO("send total len = (%d) DataLength [%.4s] ", total, lenbuf );
    rc =  sendn_timed (fd, buf, total, timeout_ms );
	if( rc < 0)
	{
	    LOG_ERR("[SEND] send 에러 ....rc=%d fd=%d", rc, fd );
		return rc;
	}

    LOG_INFO( "WR [%.*s](%d)",  total,buf,total );

	return rc;
}

