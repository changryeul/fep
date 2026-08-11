#define		_GLOBAL
/*------------------------------------------------------------------------
#	Module	: report the shared memory information
#	File	: px_chkshm.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
void	Struct_Size (void);
void    Daemon_Info (int);
void    SiseTR_Info (int);
void    Process_Info (int, int);

/*----------------------------------------------------------------------*/
int		main (int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
	int		dk, pk;
	char	d_time[16];

	Get_DateTime (d_time);
	printf ("[%s: %.4s/%.2s/%.2s %.2s:%.2s:%.2s]\n",
		argv[0], d_time, d_time+4, d_time+6, d_time+8, d_time+10, d_time+12);

	Struct_Size ();

	/* initialize global variables and attach to daemon SHM	*/
	Init_Mana (argc, argv);

	puts ("\n<Daemon Information>");
	for (dk = 0; dk < Process_Count; dk ++)
		Daemon_Info (dk);

	puts ("\n<Process Information>");
	for (dk = 0; dk < Process_Count; dk ++)
	{
		if (INFO(dk).process_no == 0)
		{
			if (INFO(dk).process_status != 0)
				printf ("%c%c processes not run [start time=%s, end time=%s]\n",
					_System_Name[0], dk + 'A', INFO(dk).start_time,
					INFO(dk).end_time);
			continue;
		}

		for (pk = 0; pk < DAEMON(dk).p_count; pk ++)
			Process_Info (dk, pk);
	}

	puts ("\n<SiseTR Information>");
	for (dk = 0; dk < Process_Count; dk ++)
		SiseTR_Info (dk);

	exit (OK);
}	/* End of main ()	*/

/*----------------------------------------------------------------------*/
void	Struct_Size (void)
/*----------------------------------------------------------------------*/
{
	puts ("\n<SHM Size>");
	printf (" SHM_MEMORY      = %d", sizeof (SHM_MEMORY));
	printf ("\n ALL_DAEMON_INFO = %d", sizeof (ALL_DAEMON_INFO));
	printf ("\n SUB_DAEMON_INFO = %d", sizeof (SUB_DAEMON_INFO));
	printf ("\n PROCESS_INFO    = %d", sizeof (PROCESS_INFO));
	printf ("\n FILE_INFO       = %d", sizeof (FILE_INFO));
	printf ("\n DSHM_INFO       = %d", sizeof (DSHM_INFO));
#if defined ISAM_INCL
	printf ("\n CISAM_INFO      = %d", sizeof (CISAM_INFO));
#endif
	printf ("\n LINE_INFO       = %d", sizeof (LINE_INFO));
	printf ("\n TCP1_INFO       = %d", sizeof (TCP1_INFO));
	printf ("\n TCP2_INFO       = %d", sizeof (TCP2_INFO));
	printf ("\n UDPIP_INFO      = %d", sizeof (UDPIP_INFO));
	printf ("\n SISETR_INFO     = %d", sizeof (SISETR_INFO));
	printf ("\n ACCNO_INFO      = %d", sizeof (ACCNO_INFO));

	return;
}	/* End of Struct_Size ()	*/

/*----------------------------------------------------------------------*/
void	Daemon_Info (int i)
/*----------------------------------------------------------------------*/
{
	int		j;

	if (INFO(i).process_id[0] == 0)
		return;

	/* daemon SHM (INFO)	*/
	printf ("%c%c   INFO[%d,%s,%s,%s,%d,%d,%d]\n", _System_Name[0], i + 'A',
		INFO(i).process_no, INFO(i).process_id, INFO(i).start_time,
		INFO(i).end_time, INFO(i).system_status, INFO(i).process_status,
		INFO(i).check_status);
	printf ("         [%s,%s,%s]\n", INFO(i).start_FIFO_name,
		INFO(i).exit_FIFO_name, INFO(i).daemon_FIFO_name);
#if defined ISAM_INCL
	printf ("        P[%d] F[%d] DSHM[%d] CI[%d] T1[%d] T2[%d] U[%d] SI[%d] AC[%d]\n",
		INFO(i).process_count, INFO(i).file_count, INFO(i).dshm_count,
		INFO(i).cisam_count, INFO(i).tcp1_count, INFO(i).tcp2_count,
		INFO(i).udpip_count, INFO(i).sisetr_count, INFO(i).accno_count);
#else
	printf ("        P[%d] F[%d] DSHM[%d] T1[%d] T2[%d] T2[%d] U[%d] SI[%d] AC[%d]\n",
		INFO(i).process_count, INFO(i).file_count, INFO(i).dshm_count,
		INFO(i).tcp1_count, INFO(i).tcp2_count,
		INFO(i).udpip_count, INFO(i).sisetr_count, INFO(i).accno_count);
#endif
	printf ("         [%d,%s,%d]\n",
		INFO(i).data_count, INFO(i).date, INFO(i).date_flag);

	/* sub daemon SHM (DAEMON)	*/
	if (INFO(i).process_no != 0)
	{
		printf ("%c%c DAEMON[%d,%s,%s]\n", _System_Name[0], i + 'A',
			DAEMON(i).process_no, DAEMON(i).process_path, DAEMON(i).process_id);
		printf ("         [%s,%s:%d,%d]\n",
			DAEMON(i).start_time, DAEMON(i).end_time,
			DAEMON(i).system_status, DAEMON(i).process_status);
		printf ("         [%s,%s,%s:%d,%d,%d]\n",
			DAEMON(i).start_FIFO_name, DAEMON(i).exit_FIFO_name,
			DAEMON(i).daemon_FIFO_name, SFIFD(i), EFIFD(i), DFIFD(i));
#if defined ISAM_INCL
		printf ("        P[%3d,%3d,%7d]  F[%3d,%3d,%7d]  DSHM[%3d,%3d,%7d] CI[%3d,%3d,%7d]\n",
			DAEMON(i).process_count, DAEMON(i).p_count, DAEMON(i).Shmsize,
			DAEMON(i).file_count, DAEMON(i).f_count, DAEMON(i).FShmsize,
			DAEMON(i).dshm_count, DAEMON(i).d_count, DAEMON(i).DShmsize,
			DAEMON(i).cisam_count, DAEMON(i).c_count, DAEMON(i).CShmsize);
#else
		printf ("        P[%3d,%3d,%7d]  F[%3d,%3d,%7d]  DSHM[%3d,%3d,%7d]\n",
			DAEMON(i).process_count, DAEMON(i).p_count, DAEMON(i).Shmsize,
			DAEMON(i).file_count, DAEMON(i).f_count, DAEMON(i).FShmsize,
			DAEMON(i).dshm_count, DAEMON(i).d_count, DAEMON(i).DShmsize);
#endif
		printf ("       T1[%3d,%3d,%7d] T2[%3d,%3d,%7d]\n",
			DAEMON(i).tcp1_count, DAEMON(i).t1_count, DAEMON(i).T1Shmsize,
			DAEMON(i).tcp2_count, DAEMON(i).t2_count, DAEMON(i).T2Shmsize);
		printf ("        U[%3d,%3d,%7d] SI[%3d,%3d,%7d] AC[%3d,%3d,%7d]\n",
			DAEMON(i).udpip_count, DAEMON(i).u_count, DAEMON(i).UShmsize,
			DAEMON(i).sisetr_count, DAEMON(i).s_count, DAEMON(i).SShmsize,
			DAEMON(i).accno_count, DAEMON(i).a_count, DAEMON(i).AShmsize);
		printf ("         [%s,%d,%s,%d]\n",
			DAEMON(i).process_info, DAEMON(i).data_count,
			DAEMON(i).date, DAEMON(i).date_flag);
	}

	return;

}	/* End of Daemon_Info ()	*/

/*----------------------------------------------------------------------*/
void    Process_Info (int i, int j)
/*----------------------------------------------------------------------*/
{
	int		k, l;

	printf ("%c%c   PROC[%d,%s,%s,%d]\n", _System_Name[0], i + 'A',
		PROC(i,j).process_no, PROC(i,j).process_path,
		PROC(i,j).process_id, PROC(i,j).process_status);
	printf ("         [%s:%d,%d,%d:%d,%d,%d:%s,%s]\n",
		PROC(i,j).process_info, IFIFD(i,j,0), IFIFD(i,j,1), IFIFD(i,j,2),
		PROC(i,j).start_status, PROC(i,j).timeout, PROC(i,j).delay,
		PROC(i,j).start_time, PROC(i,j).end_time);
	printf ("         [%s,%8d,%d]\n",
		PROC(i,j).process_type, PROC(i,j).if_seq, PROC(i,j).type);

	if (PROC(i,j).type == 0)
		printf ("        0[]\n");
	else if (PROC(i,j).type == TY_TRS1) 
	{
		if (PROC(i,j).l.t1.line_gubun)
		{
			printf ("       T1[%d,%d,%d:%d,%d.%d.%d.%d]\n",
				PROC(i,j).l.t1.line_gubun, PROC(i,j).l.t1.port_type,
				PROC(i,j).l.t1.network_status, TCP1_PORT(i,j), TCP1_IP(i,j,0),
				TCP1_IP(i,j,1), TCP1_IP(i,j,2), TCP1_IP(i,j,3));
			if (PROC(i,j).l.t1.port_type == 0)
			{
				for (l = 0; l < 5; l ++)
				{
					if (TCP1_SPORT(i,j,l))
					{
						printf ("    T1_S%d[%d, %d, %d]\n",
							l+1, TCP1_SPORT(i,j,l), TCP1_S_ST(i,j,l),
							TCP1_S_CT(i,j,l));
					}
				}
			}
			else 
			{
				printf ("    T1_S%d[%d, %d, %d]\n",
					PROC(i,j).l.t1.port_type,
					TCP1_SPORT(i,j,PROC(i,j).l.t1.port_type-1),
					TCP1_S_ST(i,j,PROC(i,j).l.t1.port_type-1),
					TCP1_S_CT(i,j,PROC(i,j).l.t1.port_type-1));
			}

			if (PROC(i,j).l.t1.port_type != 0 && PROC(i,j).process_status != 1)
				printf ("   CLIENT[%s:%d]\n", PROC(i,j).ip, PROC(i,j).port);
		}
	}
	else if (PROC(i,j).type == TY_TRS2)
	{
		printf ("       T2[%d:%d,%d]\n",
			PROC(i,j).l.t2.line_gubun,
			PROC(i,j).l.t2.l[0], PROC(i,j).l.t2.l[1]);

		for (l = 0; l < 2; l ++)
		{
			if (PROC(i,j).l.t2.l[l])
			{
				printf ("    T2_L%d[%d:%d,%d.%d.%d.%d:%d,%d,%d:%s]\n", l + 1,
					TCP2_ID(i,j,l), TCP2_PORT(i,j,l), TCP2_IP1(i,j,l),
					TCP2_IP2(i,j,l), TCP2_IP3(i,j,l), TCP2_IP4(i,j,l),
					TCP2_LSTAT(i,j,l), TCP2_PSTAT(i,j,l), TCP2_NSTAT(i,j,l),
					TCP2_INFO(i,j,l));
			}
		}
	}

	printf ("        DSHM[%d,%d,%d:%d,%d,%d,%d,%d,%d,%d,%d,%d]\n",
		PROC(i,j).in_d[0], PROC(i,j).in_d[1], PROC(i,j).in_d[2],
		PROC(i,j).out_d[0], PROC(i,j).out_d[1], PROC(i,j).out_d[2],
		PROC(i,j).out_d[3], PROC(i,j).out_d[4], PROC(i,j).out_d[5],
		PROC(i,j).out_d[6], PROC(i,j).out_d[7], PROC(i,j).out_f[8]);

	printf ("        F[%d,%d,%d:%d,%d,%d,%d,%d,%d,%d,%d,%d:%s]\n",
		PROC(i,j).in_f[0], PROC(i,j).in_f[1], PROC(i,j).in_f[2],
		PROC(i,j).out_f[0], PROC(i,j).out_f[1], PROC(i,j).out_f[2],
		PROC(i,j).out_f[3], PROC(i,j).out_f[4], PROC(i,j).out_f[5],
		PROC(i,j).out_f[6], PROC(i,j).out_f[7], PROC(i,j).out_f[8],
		PROC(i,j).date);

	for (k = 0; k < 3; k ++)
	{
		if (PROC(i,j).in_d[k])
			printf ("      ID%d[%s,%s:%d:%d,%d,%d,%d,%d,%d,%d,%d,%d:%d,%d,%d]\n",
				k + 1, IDN(i,j,k), FFN(i,j,k), IDW(i,j,k,0), IDR(i,j,k,0),
				IDR(i,j,k,1), IDR(i,j,k,2), IDR(i,j,k,3), IDR(i,j,k,4),
				IDR(i,j,k,5), IDR(i,j,k,6), IDR(i,j,k,7), IDR(i,j,k,8),
				IDS(i,j,k), IDC(i,j,k), IDM(i,j,k));
	}

	for (k = 0; k < 3; k ++)
	{
		if (PROC(i,j).in_f[k])
			printf ("      IF%d[%s,%s:%d:%d,%d,%d,%d,%d,%d,%d,%d,%d:%d,%d]\n",
				k + 1, IFN(i,j,k), FFN(i,j,k), IFW(i,j,k,0), IFR(i,j,k,0),
				IFR(i,j,k,1), IFR(i,j,k,2), IFR(i,j,k,3), IFR(i,j,k,4),
				IFR(i,j,k,5), IFR(i,j,k,6), IFR(i,j,k,7), IFR(i,j,k,8),
				IFS(i,j,k), IFC(i,j,k));
	}

#if defined ISAM_INCL
	for (k = 0; k < 3; k ++)
	{
		if (PROC(i,j).in_c[k])
			printf ("      IC%d[%s:%d,%d]\n",
				k + 1, ICN(i,j,k), ICK(i,j,k), ICS(i,j,k));
	}

	for (k = 0; k < 3; k ++)
	{
		if (PROC(i,j).out_c[k])
			printf ("      OC%d[%s:%d,%d]\n",
				k + 1, OCN(i,j,k), OCK(i,j,k), OCS(i,j,k));
	}
#endif

	for (k = 0; k < 99; k ++)
	{
		if (PROC(i,j).out_d[k] == 0)
			break;
		else
			printf ("      OD%d[%s:%d:%d,%d,%d,%d,%d,%d,%d,%d,%d:%d,%d,%d]\n",
				k + 1, ODN(i,j,k), ODW(i,j,k,0), ODR(i,j,k,0), ODR(i,j,k,1),
				ODR(i,j,k,2), ODR(i,j,k,3), ODR(i,j,k,4), ODR(i,j,k,5),
				ODR(i,j,k,6), ODR(i,j,k,7), ODR(i,j,k,8), ODS(i,j,k),
				ODC(i,j,k), ODM(i,j,k));
	}

	for (k = 0; k < 99; k ++)
	{
		if (PROC(i,j).out_f[k] == 0)
			break;
		else
			printf ("      OF%d[%s:%d:%d,%d,%d,%d,%d,%d,%d,%d,%d:%d,%d]\n",
				k + 1, OFN(i,j,k), OFW(i,j,k,0), OFR(i,j,k,0), OFR(i,j,k,1),
				OFR(i,j,k,2), OFR(i,j,k,3), OFR(i,j,k,4), OFR(i,j,k,5),
				OFR(i,j,k,6), OFR(i,j,k,7), OFR(i,j,k,8), OFS(i,j,k),
				OFC(i,j,k));
	}

	printf ("         [%s,%-6.6s,%-6.6s:%-4.4s,%-6.6s:%d,%d,0x%08x]\n",
		PROC(i,j).date, PROC(i,j).tr_s_tm, PROC(i,j).tr_e_tm,
		PROC(i,j).error_cd, PROC(i,j).error_tm, PROC(i,j).data_flag,
		PROC(i,j).data_cnt, PROC(i,j).data);

	return;

}	/* End of Process_Info ()	*/

/*----------------------------------------------------------------------*/
void    SiseTR_Info (int i)
/*----------------------------------------------------------------------*/
{
	int     j;

#if 0
	if (INFO(i).process_id[0] == 0)
		return;

	if (INFO(i).process_no != 0)
	{
#endif
		for (j = 0; j < INFO(i).sisetr_count; j ++)
		{
			if (SISETR(i,j).tr[0] != '\0')
			{
				printf ("P%c SISETR_%03d INFO [%s] [%s,%d,%d] UR:%d, DD:%d\n", 
					i + 'A', j + 1, SISETR(i,j).tr_info, SISETR(i,j).tr,
					SISETR(i,j).length, SISETR(i,j).queue, 
					SISETR(i,j).ur_count, SISETR(i,j).dd_count);
			}
			else
				break;
		}

#if 0
	}
#endif

	return;
}

/*************************************************************************
	End of Program (px_chkshm.c)
*************************************************************************/
