/*------------------------------------------------------------------------
#	Module	: save read sequence
#	File	: seq_save.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
int		Seq_Save (char *, int, int);
int		Dshm_Seq_Save (char *, int, int, int);

/*************************************************************************
	Function		: . save file read sequence
	Parameters IN	: . p_file	: file name 
					  . dk		: daemon key
					  . fk		: FILEM position
	Parameters OUT	: .
	Return Code		: . int (0: success, -1: failure)
	Global Data		: . char *_FEP_DAT
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Seq_Save (char *p_file, int dk, int fk)
/*----------------------------------------------------------------------*/
{
	int				i, rt;
	char			f_name[100], buf[512], bumun[4];
	FILE			*fp;
	struct flock	lock;

	sprintf (bumun, "%-2.2s", p_file);
	sprintf (f_name, "%s/%s/00000000/%s_seq",
		_FEP_DAT, LtoU (bumun, 2), p_file);
	memset (buf, 0, sizeof (buf));

	fp = fopen (f_name, "r+");

	if (fp == NULL) 
	{
		Log (SAM_FATAL, "Seq_Save:fopen failure[%s] {%d:%s}",
			f_name, SYS_NO, SYS_STR);
		return (NOTOK);
	}
	
	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = 0;
		lock.l_start = 0L;
		lock.l_len = 0L;

		rt = fcntl (fileno (fp), F_SETLKW, &lock);

		if (rt == -1) 
		{
			if (SYS_NO == EINTR)
				continue;

			fclose (fp);
			Log (SYS_FATAL, "Seq_Save:cannot lock (fcntl)[%s] {%d:%s}",
				f_name, SYS_NO, SYS_STR);
			return (-1);
		}
	} while (rt == -1);

	for (i = 0; i < 9; i ++)
		sprintf (&buf[8*i], "%08d", FILEM(dk,fk).r_cnt[i]);

	fseek (fp, 0L, SEEK_SET);

	rt = fwrite (buf, strlen (buf), 1, fp);

	fflush (fp);

	lock.l_type = F_UNLCK;
	fcntl (fileno (fp), F_SETLK, &lock);
	fclose (fp);

	return (OK);
}	/* End of Seq_Save ()	*/

/*************************************************************************
	Function		: . save data SHM read sequence
	Parameters IN	: . p_file	: data SHM base name 
					  . dk		: daemon key
					  . fk		: DSHM position
					  . ck		: read count key (0 ~ 8)
	Parameters OUT	: .
	Return Code		: . int (0: success, -1: failure)
	Global Data		: . char *_FEP_DAT
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Dshm_Seq_Save (char *p_file, int dk, int fk, int ck)
/*----------------------------------------------------------------------*/
{
	int				i, rt;
	char			f_name[100], buf[512], bumun[4];
	FILE			*fp;
	struct flock	lock;

	sprintf (bumun, "%-2.2s", p_file);
	sprintf (f_name, "%s/%s/00000000/%s_dseq",
		_FEP_DAT, LtoU (bumun, 2), p_file);
	memset (buf, 0, sizeof (buf));

	fp = fopen (f_name, "r+");

	if (fp == NULL) 
	{
		Log (SAM_FATAL, "Dshm_Seq_Save:fopen fail[%s] {%d:%s}",
			f_name, SYS_NO, SYS_STR);
		return (NOTOK);
	}

	do {
		lock.l_type = F_WRLCK;
		lock.l_whence = SEEK_SET;
		lock.l_start = 8L * ck;
		lock.l_len = 8L;

		rt = fcntl (fileno (fp), F_SETLKW, &lock);

		if (rt == -1) 
		{
			if (SYS_NO == EINTR)
				continue;

			fclose (fp);
			Log (SYS_FATAL, "Dshm_Seq_Save:cannot lock (fcntl)[%s] {%d:%s}",
				f_name, SYS_NO, SYS_STR);
			return (-1);
		}
	} while (rt == -1);

	fseek (fp, 8L * ck, SEEK_SET);

	if (ck == 9)
		sprintf (buf, "%08d", DSHM(dk,fk).sm_r_cnt);
	else
		sprintf (buf, "%08d", DSHM(dk,fk).r_cnt[ck]);

	rt = fwrite (buf, strlen (buf), 1, fp);

	fflush (fp);

	lock.l_type = F_UNLCK;
	fcntl (fileno (fp), F_SETLK, &lock);
	fclose (fp);

	return (OK);
}	/* End of Dshm_Seq_Save ()	*/

/*************************************************************************
	End of Program (seq_save.c)
*************************************************************************/
