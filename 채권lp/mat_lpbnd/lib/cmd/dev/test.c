#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "task.h"

main( int argc, char *argv[])
{
	int		rtn = 1;

	TaskInit();

	while( rtn >= 0)
	{
		rtn = Cmd_Main( Cmd);
	}

	return 1;
}

