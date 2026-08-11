#include <stdarg.h>
#include <sys/wait.h>
#include "mymq.h"
#include "context.h"

void	mds_alert(MARKET *market, const char *msg)
{
	MyMQ	*mymq;

	mymq = mymq_open("", market->procname, "mymq", "mymq");
	if (mymq == NULL)
		return;
	mymq_broadcast(mymq, UNSOLICITED_MSG, NULL, "", "", "", "", (char *)msg, (int)strlen(msg));
	mymq_close(mymq);
}
