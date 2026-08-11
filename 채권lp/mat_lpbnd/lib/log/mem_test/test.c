#include <stdio.h>
#include "log.h"

int test_log();
int MyMsg( char *rec, int sz);

int main( int argc, char *argv[])
{
	int		rtn;
	char	*file = "./test.log";
	char	rec[ 512] = "12345678901234567890";

	LogOpen( NULL);
	LogTst( "debug", file);
	/*
	LogFile( "./test.log");
	*/
	rtn = test_log();
	printf( "log redirect to file .... name=[%s]\n", "./test.log");
	LogDump( rec, strlen( rec), "dump test %d", 1);

	LogFunc( MyMsg);
	LogUsr( "test...");
	LogDbg( "Here...");

	printf( "\n\n\n\n\n\n");
	LogFile( "./test.log");
	rtn = test_log();
	LogUsr( "test...");


	while( 1)
	{
		rtn = test_log();
		sleep( 1);
	}


	LogClose();

	return 0;
}

int test_log()
{
	LogDbg( "debug");
	LogMsg( "message");
	LogApp( "application");
	LogLib( "Library");
	LogWar( "warning");
	LogErr( "error");
	LogCri( "critical");
	LogBel( "bell");

}

int MyMsg( char *rec, int sz)
{
	printf( "MyMsg..... %s", rec);
	printf( "%.*s\n", sz, rec);

	return 1;
}

