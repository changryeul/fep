#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: change UDP IP address and/or port of a process
#	File	: px_setudp.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_fepp.h"

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	int		dk, pk, sk;
	u_long	ul;
	char	proc_name[12], ip_addr[16], port[8], flag[4], bumun[4], buf[128];
	char	id[4];
	FILE	*fp;

	if (argc != 1 && argc != 5)
	{
		printf ("==========================================================\n");
		printf ("[change UDP IP address and/or port of a process]\n");
		printf ("Usage: %s <process name> <ID> <IP address> <port>\n",
			argv[0]);
		printf ("  e.g. 1) %s\n", argv[0]);
		printf ("       2) %s pa_7101_ur 1 0.0.0.0 5571\n", argv[0]);
		printf ("==========================================================\n");
		exit (0);
	}

	memset (proc_name, 0, sizeof (proc_name));
	memset (id, 0, sizeof (id));
	memset (ip_addr, 0, sizeof (ip_addr));
	memset (port, 0, sizeof (port));
	memset (flag, 0, sizeof (flag));

	if (argc == 5)
	{
		memcpy (proc_name, argv[1], strlen (argv[1]));
		memcpy (id, argv[2], strlen (argv[2]));
		memcpy (ip_addr, argv[3], strlen (argv[3]));
		memcpy (port, argv[4], strlen (argv[4]));
		flag[0] = '3';
	}
	else
	{
		printf ("\033[1mprocess name (e.g. pa_7101_ur) ?\033[0m ");
		fflush (stdout);
        fgets (proc_name, sizeof(proc_name), stdin);
		if (proc_name[0] == '\0')
			exit (FAIL);

		printf ("\033[1mID (1 ~ 20) ?\033[0m ");
		fflush (stdout);
        fgets (id, sizeof(id), stdin);
		if (id[0] == '\0')
			exit (FAIL);

		printf ("\033[1mchange what (1:IP address 2:port 3:all) ?\033[0m ");
		fflush (stdout);
        fgets (flag, sizeof(flag), stdin);
		if (flag[0] == '\0')
			exit (FAIL);

		if (flag[0] < '1' || flag[0] > '3')
		{
			printf ("check <change option>[%s] (1:IP address 2:port 3:all)\n",
				flag);
			exit (FAIL);
		}

		if (flag[0] == '1' || flag[0] == '3')
		{
			printf ("\033[1mIP address (e.g. 172.30.220.135) ?\033[0m ");
			fflush (stdout);
			fgets (ip_addr, sizeof(ip_addr), stdin);
			if (ip_addr[0] == '\0')
				exit (FAIL);
		}

		if (flag[0] == '2' || flag[0] == '3')
		{
			printf ("\033[1mport (e.g. 30001) ?\033[0m ");
			fflush (stdout);
			fgets (port, sizeof(port), stdin);
			if (port[0] == '\0')
				exit (FAIL);
		}
	}

	if (strlen (proc_name) != 10)
	{
		printf ("check <process name>[%s]\n", proc_name);
		exit (FAIL);
	}

	sk = AtoIf (id, 1) - 1;

	Init_Mana (argc, argv);

	sprintf (bumun, "%-2.2s", proc_name);
	LtoU (bumun, 2);
	dk = proc_name[1] - 'a';

	if (INFO(dk).process_id[0] == 0)
	{
		printf ("%s daemon not registered !!!\n", bumun);
		exit (FAIL);
	}

	for (pk = 0; pk < DAEMON(dk).p_count; pk ++)
	{
		if (memcmp (PROC(dk,pk).process_id, proc_name, 10) == 0)
		{
			printf ("before [%s: %d.%d.%d.%d, %d]\n",
				PROC(dk,pk).process_id, UDP_IP1(dk,pk,sk), UDP_IP2(dk,pk,sk),
				UDP_IP3(dk,pk,sk), UDP_IP4(dk,pk,sk), UDP_PORT(dk,pk,sk));

			if (flag[0] == '1' || flag[0] == '3')
			{
				ul = inet_addr (ip_addr);
				if (ul == -1)
				{
					printf ("malformed IP address[%s]\n", ip_addr);
					exit (0);
				}
				else
					memcpy (&UDP_IP1(dk,pk,sk), &ul, sizeof (ul));
			}

			if (flag[0] == '2' || flag[0] == '3')
				UDP_PORT(dk,pk,sk) = atoi (port);

			printf ("after  [%s: %d.%d.%d.%d, %d]\n",
				PROC(dk,pk).process_id, UDP_IP1(dk,pk,sk), UDP_IP2(dk,pk,sk),
				UDP_IP3(dk,pk,sk), UDP_IP4(dk,pk,sk), UDP_PORT(dk,pk,sk));
			break;
		}

		if (pk == DAEMON(dk).p_count - 1)
		{
			printf ("process not registered[%s]\n", proc_name);
			exit (0);
		}
	}

#if defined __hpux
	fp = popen ("who -mR", "r");
#elif defined sun || _AIX || __linux
	fp = popen ("who -m", "r");
#endif
	fgets (buf, sizeof (buf), fp);
	pclose (fp);
	buf[strlen(buf)-1] = '\0';
	Log (USR_OK, "[%s: %s, %s, %s, %s]", buf, proc_name, id, ip_addr, port);

	exit (OK);
}	/* End of main ()	*/

/*************************************************************************
	End of Program (px_setudp.c)
*************************************************************************/
