/*------------------------------------------------------------------------
#	Module	: main routine of pz_memory_mp
#	File	: pz_memory_proc.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"daemon.h"

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	Main_Process (void);
void	Check_Work_Dir (void);
int		Chk_Digit (char *, int);
void	Shm_Conf_Process (int);

/*************************************************************************
	Function		: . main routine
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Main_Process (void)
/*----------------------------------------------------------------------*/
{
	/* executed by super daemon (fepp_mp) or user (manually)	*/
	if (D_K + 'A' == 'Z' || D_K == -1)
	{
		/* initialize daemon SHM (INFO)	*/
		Sub_SHM_Creat ();

		/* read daemon.ini and set the daemon SHM (INFO)	*/
		Daemon_Config_Read (0);
	}
	/* executed by sub daemons (daemon_mp)	*/
	else
	{
		/* attach daemon SHM (INFO)	*/
		Sub_SHM ();
	}

	/* executed by user or sub daemons	*/
	if (D_K + 'A' != 'Z')
	{
		/* initialize sub SHM and set daemon SHM (INFO, DAEMON)	*/
		Mem_SHM_Creat ();

		/* initialize sise data SHM	*/
		if (D_K != -1)
		{
			if (INFO(D_K).sisetr_count > 0)
				Sise_SHM_Creat ();
		}

		/* create and backup the work directory	*/
		Check_Work_Dir ();

		if (D_K == -1)
			Shm_Conf_Process (0);				/* set sub SHM - all	*/
		else
			Shm_Conf_Process (1);				/* set sub SHM - one	*/
	}

	return;
}	/* End of Main_Process ()	*/

/*************************************************************************
	Function		: . create and backup the work directory
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Check_Work_Dir (void)
/*----------------------------------------------------------------------*/
{
	int		i, rt;
	char	file_name[128], sub[4], Sub[4], data[10], buf[256], w_date[12];
	char    file_name2[128], file_name3[128], file_name4[128], file_name5[128];
	char    file_name6[128], file_name7[128], file_name8[128], file_name9[128];
	char    file_name10[128];
	FILE	*fp = NULL;

	for (i = 0; i < SHM_MAX_SUB; i++)
	{
		if (INFO(i).date_flag == 9)
			continue;

		if ((D_K == -1 || D_K == i) && INFO(i).process_count != 0)
		{
			sprintf (sub, "%c%c", _Exe_Name[0], i + 'a');
			memcpy (Sub, sub, 2);
			LtoU (Sub, 2);
			memset (w_date, 0, sizeof (w_date));

			if (D_K == -1)
				memcpy (w_date, INFO(i).date, 8);
			else
			{
				if (INFO(i).process_count != 0)
					memcpy (w_date, DAEMON(i).date, 8);
				else
					memset (w_date, ' ', 8);
			}

			sprintf (file_name, "%s/%cZ/00000000", _FEP_DAT, _System_Name[0]);

			rt = stat (file_name, &_F_Info);

			if (rt == -1)
			{
				mkdir (file_name, 0777);
				chmod (file_name, 0777);
				Log (SYS_OK, "work date directory (%s) created", file_name);
			}

			sprintf (file_name2, "%s/%.2s/00000000", _FEP_DAT, Sub);

			rt = stat (file_name2, &_F_Info);

			if (rt == -1)
			{
				mkdir (file_name2, 0777);
				chmod (file_name2, 0777);
				Log (SYS_OK, "data directory (%s) created", file_name2);
				sprintf (file_name3, "%s/%cZ/00000000/%.2s_date",
					_FEP_DAT, _System_Name[0], sub);
				memset (buf, 0, sizeof (buf));

				fp = fopen (file_name3, "w");

				if (fp == NULL)
				{
					Log (SAM_FATAL, "cannot open file1[%s] {%d:%s}",
						file_name3, SYS_NO, SYS_STR);
					exit (FAIL);
				}

				fwrite (w_date, 8, 1, fp);
				fflush (fp);
				fclose (fp); fp = NULL;
			}
			else
			{
				memset (buf, 0, sizeof (buf));
				sprintf (file_name4, "%s/%cZ/00000000/%.2s_date",
					_FEP_DAT, _System_Name[0], sub);

				rt = stat (file_name4, &_F_Info);

				if (rt == -1)
				{
					fp = fopen (file_name4, "w");

					if (fp == NULL)
					{
						Log (SAM_FATAL, "cannot open file2[%s] {%d:%s}",
							file_name4, SYS_NO, SYS_STR);
						exit (FAIL);
					}

					fwrite (w_date, 8, 1, fp);
					fflush (fp);
					fclose (fp); fp = NULL ;
				}

				sprintf (file_name10, "%s/%cZ/00000000/%.2s_date",
					_FEP_DAT, _System_Name[0], sub);
				if (fp == NULL)
				{
					fp = fopen (file_name10, "r+");

					if (fp == NULL)
					{
						Log (SAM_FATAL, "cannot open file[%s] {%d:%s}",
							file_name10, SYS_NO, SYS_STR);
						exit (FAIL);
					}
				}

				fgets (data, 10, fp);

				if (memcmp (data, w_date, 8) != 0)
				{
					if (Chk_Digit (data, 8) == OK)
					{
						/* move data files	*/
						sprintf (buf, "mv %s/%.2s/00000000 %s/%.2s/%.8s",
							_FEP_DAT, Sub, _FEP_DAT, Sub, data);

						rt = system (buf);
#if defined __hpux || sun || _AIX
						if (rt < 0)
						{
							Log (SYS_FATAL, "system call failure [%s] 01 rt[%d]", buf, rt);
							fclose (fp); fp = NULL ;
							exit (FAIL);
						}
#endif

						/* move work date file	*/
						sprintf (file_name5, "%s/%cZ/%.8s",
							_FEP_DAT, _System_Name[0], data);

						rt = stat (file_name5, &_F_Info);

						if (rt == -1)
						{
							mkdir (file_name5, 0777);
							chmod (file_name5, 0777);
						}

						sprintf (buf,
							"mv %s/%cZ/00000000/%.2s_date %s/%cZ/%.8s",
							_FEP_DAT, _System_Name[0], sub, _FEP_DAT,
							_System_Name[0], data);

						rt = system (buf);
#if defined __hpux || sun || _AIX
						if (rt < 0)
						{
							Log (SYS_FATAL, "system call failure[%s] 02 rt[%d]", buf, rt);
							fclose (fp); fp = NULL ;
							exit (FAIL);
						}
#endif
					}
					else
					{
						fwrite (w_date, 8, 1, fp);
						fflush (fp);
						fclose (fp); fp = NULL ;

						if (D_K == i)
							break;
						else
							continue;
					}

					fclose (fp);
					sleep (2);
					sprintf (file_name6, "%s/%.2s/00000000", _FEP_DAT, Sub);
					mkdir (file_name6, 0777);
					chmod (file_name6, 0777);
					sprintf (file_name7, "%s/%cZ/00000000/%.2s_date",
						_FEP_DAT, _System_Name[0], sub);
					memset (buf, 0, sizeof (buf));

					fp = fopen (file_name7, "w");

					if (fp == NULL)
					{
						Log (SAM_FATAL, "cannot open file4 [%s]", file_name7);
						exit (FAIL);
					}

					fwrite (w_date, 8, 1, fp);
					fflush (fp);
					fclose (fp); fp = NULL ;

					/* move log files	*/
					sprintf (file_name8, "%s/%.2s/00000000", _FEP_LOG, Sub);

					rt = stat (file_name8, &_F_Info);

					if (rt != -1)
					{
						if (Chk_Digit (data, 8) == OK)
						{
							sprintf (buf, "mv %s/%.2s/00000000 %s/%.2s/%.8s",
								_FEP_LOG, Sub, _FEP_LOG, Sub, data);

							rt = system (buf);

#if defined __hpux || sun || _AIX
							if (rt < 0)
							{
								Log (SYS_FATAL, "system call failure[%s] 03 rt[%d]", buf, rt);
								exit (FAIL);
							}
							else
								Log (USR_OK, "system call OK 03");
#endif
						}
						else
						{
							Log (USR_FATAL, "%.2s:invalid work date (%.8s)",
								Sub, data);
							exit (FAIL);
						}

						sleep (2);
					}

					sprintf (file_name9, "%s/%.2s/00000000", _FEP_LOG, Sub);

					rt = stat (file_name9, &_F_Info);

					if (rt == -1)
					{
						mkdir (file_name9, 0777);
						chmod (file_name9, 0777);
						Log (SYS_OK, "log directory (%s) created", file_name9);
					}
				}
				else
				{
					fclose (fp);
					fp = NULL ;
				}
			}

			if (D_K == i)
				break;
		}
	}

	return;
}	/* End of Check_Work_Dir ()	*/

/*************************************************************************
	Function		: . test for any decimal-digit character
	Parameters IN	: . str	: string
					  . len	: length of the string
	Parameters OUT	: .
	Return Code		: . 0 (OK: digit only), -1 (NOTOK)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Chk_Digit (char *str, int len)
/*----------------------------------------------------------------------*/
{
	int		i;

	for (i = 0; i < len; i++)
	{
		if (!isdigit (str[i]))
			return (NOTOK);
	}

	return (OK);
}	/* End of Chk_Digit ()	*/

/*************************************************************************
	Function		: . read configuration files and load SHM
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void	Shm_Conf_Process (int flag)
/*----------------------------------------------------------------------*/
{
	/* read file.ini and set the file SHM (FILEM)	*/
	File_Config_Read (flag);

	/* read dshm.ini and set the data SHM (DSHM)	*/
	Dshm_Config_Read (flag);

#if defined ISAM_INCL
	/* read cisam.ini and set the file SHM (CISAM)	*/
	Cisam_Config_Read (flag);
#endif

	/* read tcp1.ini and set the TCP/IP 1 SHM (TCP1)	*/
	Tcp1_Config_Read (flag);

	/* read tcp2.ini and set the TCP/IP 2 SHM (TCP2)	*/
	Tcp2_Config_Read (flag);

	/* read udpip.ini and set the udpip SHM (UDPIP)	*/
	Udpip_Config_Read (flag);

	/* read proc.ini and set the process SHM (PROC)	*/
	Proc_Config_Read (flag);

	/* read sisetr.ini and set the sise tr SHM (SISETR) */
/* 202201
	SiseTr_Config_Read (flag);
*/

	/* read accno.ini and set the sise tr SHM (SISETR) */
/* 202201
	Accno_Config_Read (flag);
*/

	return;
}	/* End of Shm_Conf_Process ()	*/

/*************************************************************************
	End of Program (pz_memory_proc.c)
*************************************************************************/
