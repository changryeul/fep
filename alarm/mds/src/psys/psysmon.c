/******************************************************************************/
/*  Components  : pisysmon.c                                                  */
/*  Description : system manager process                                      */
/*  Rev. History: Ver   Date    Description                                   */
/*        ----  ------- ------------------------------------------------------*/
/*        1.0   2006-07 Initial version                                       */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <libgen.h>
#include "protab.h"

#if defined(SUNOS)
#define BIN_PS		"/usr/bin/sparcv9/ps"
#else
#define BIN_PS		"/bin/ps"
#endif
#define PS_OPTS		"-ef"
#define CONF_FILE	"PROCTBL"

void    ign_exit_job(int);
void    stop_system(int);
void    chld_handler(int);
int     killall_proc();
int	check_pbase(), check_ptime();
int	l_syslog(char *, const char *, ...);

static  struct  chcktab {                   /* check table              */
    int chck;                               /* valid mark               */
    int (*chker)();                         /* checker process          */
    char    desc[32];                       /* description              */
} chcktab[] = {
    {  1, check_pbase, "BASE 프로세스"  },
    {  2, check_ptime, "TIMELY 프로세스"    },
    {  0, NULL,        "EOT"        }
};

static  struct  tm now;                     /* current time             */
struct  bootab bootab;                      /* boot process table       */
struct  protab protab[MAX_PT][M_PROC];      /* daemon process table     */
int x_process[MAX_PT] = {0, 0};             /* modified flag            */
char    file_path[128];                     /* config file path         */
time_t  file_mtim;                          /* modified time stamp      */

char    svc_grp[16];                        /* service group name       */
struct  tm  today;                          /* today 일자               */
char    log_m[128];                         /* log message buffer       */
char    myname[16];                         /* process name             */

/******************************************************************************/
/* NAME : main()                                                              */
/* DESC : MAIN PROCEDURE                                                      */
/******************************************************************************/
main(argc, argv)
int argc;
char    *argv[];
{
    struct   stat  statbuf;
    time_t   clock;
    struct   tm  *ctm;
    int  retc;
    int  ii, jj;
    char    *whoami;

    whoami = basename(argv[0]);
    strcpy(myname, whoami);

    /* Usage : syscom service-group(default is 'cfg') */
    if (argc < 2)
        strcpy(svc_grp, "cfg");
    else
        strcpy(svc_grp, argv[1]);
    signal(SIGINT,  stop_system);
    signal(SIGTERM, stop_system);
    signal(SIGQUIT, stop_system);

    /* current time */
    clock = time(0);
    ctm   = localtime(&clock);
    memcpy(&today, ctm, sizeof(struct tm));

    l_syslog(myname, "SYSTEM MANAGER START-UP [%s.%s]", CONF_FILE, svc_grp);

    memset(&bootab, 0x00, sizeof(struct bootab));
    memset(&protab, 0x00, sizeof(struct protab) * (MAX_PT * M_PROC));

    /* get the process list */
    sprintf(file_path,"%s/%s.%s", ETC_DIR, CONF_FILE, svc_grp);
    printf("%s\n", file_path);
	if (get_environment() < 0 || get_boot_job() < 0)
    {
        sleep(3);
        exit(1);
    }
    for (ii = 0; chcktab[ii].chck != 0; ii++)
        (*chcktab[ii].chker)();

    killall_proc();

    /* execute the boot process */
    if (exec_boot_job() < 0)
    {
        sleep(3);
        exit(1);
    }
    memcpy(&now, ctm, sizeof(struct tm));

    signal(SIGCHLD, chld_handler);
    sighold(SIGCHLD);
    do_exec(1);
    l_syslog(myname, "ON-SCHEDULE OF PROCESSES");

    for (;;)
    {
        clock = time(0);
        ctm   = localtime(&clock);
        if (today.tm_mday != ctm->tm_mday)
            clear_chck_board();

        memcpy(&now, ctm, sizeof(struct tm));

        retc = stat(file_path, &statbuf);
        if (retc == 0 && file_mtim != statbuf.st_mtime)
        {
            file_mtim = statbuf.st_mtime;
            for (ii = 0; chcktab[ii].chck > 0; ii++)
                (*chcktab[ii].chker)();
        }
        check_protab();

        clean_zombie();
        do_exec(0);
        sleep(3);
    }

    killall_proc();

    return (0);
}

/******************************************************************************/
/* NAME : get_environment()                                                   */
/* DESC : get the environment of the service group                            */
/******************************************************************************/
get_environment()
{
    FILE    *pro_F;
    char    line_b[256];
    char    envr_b[128], desc_b[128], *envbuf;
    int ii, jj, ll;
    int section_f = 0;

    pro_F = fopen(file_path, "r");
    if (pro_F == NULL)
    {
        sprintf(log_m, "FILE OPEN 오류 [%s] errno=(%d)",
            file_path, errno);
        l_syslog(myname, log_m);
        return(-1);
    }

    while ((fgets(line_b, sizeof(line_b), pro_F)) == line_b)
    {
        ll = strlen(line_b);
        if (ll <= 4)
            continue;

        for (ii = ll-1; ii >= 0; ii--)
        {
            if (line_b[ii] != '\n' && line_b[ii] != '\r')
                break;
            line_b[ii] = '\0';
            ll--;
        }

        envr_b[0] = '\0';
        desc_b[0] = '\0';

        for (ii = ll-1; ii >= 0; ii--)
        {
            if (line_b[ii] == '#' || line_b[ii] == '*')
            {
                line_b[ii] = '\0';
                break;
            }
        }

        sscanf(line_b, "%s %s", envr_b, desc_b);
        if (strlen(envr_b) == 0 || envr_b[0] == '#' || envr_b[0] == '*')
            continue;

        /* search the section */
        if (envr_b[0] == '[')
        {
            if (section_f)          /* search OK */
                break;
            if (envr_b[strlen(envr_b)-1] == ']')
                envr_b[strlen(envr_b)-1] = '\0';
            if (strcmp(SECT_ENVS, &envr_b[1]) != 0)
                continue;

            section_f = 1;
            continue;
        }
        if (!section_f)
            continue;

        if (strchr(envr_b, '=') == NULL)
            continue;

        envbuf = malloc(strlen(envr_b)+1);
        strcpy(envbuf, envr_b);
        putenv(envbuf);
    }
    fclose(pro_F);
    return (0);
}

/******************************************************************************/
/* NAME : get_boot_job()                                                      */
/* DESC : get the boot process list                                           */
/******************************************************************************/
get_boot_job()
{
    FILE    *pro_F;
    char    line_b[256];
    char    args_b[M_ARGC][128], proc_b[64], desc_b[128];
    int ii, jj, ll;
    int section_f = 0;

    pro_F = fopen(file_path, "r");
    if (pro_F == NULL)
    {
        sprintf(log_m, "FILE OPEN 오류 [%s] errno=(%d)",
            file_path, errno);
        l_syslog(myname, log_m);
        return(-1);
    }

    while ((fgets(line_b, sizeof(line_b), pro_F)) == line_b)
    {
        ll = strlen(line_b);
        if (ll <= 4)
            continue;

        for (ii = ll-1; ii >= 0; ii--)
        {
            if (line_b[ii] != '\n' && line_b[ii] != '\r')
                break;
            line_b[ii] = '\0';
            ll--;
        }

        for (ii = 0; ii < M_ARGC; ii++)
            args_b[ii][0] = '\0';
        desc_b[0] = '\0';

        for (ii = ll-1; ii >= 0; ii--)
        {
            if (line_b[ii] == '#' || line_b[ii] == '*')
            {
                memcpy(desc_b, &line_b[ii+1], L_DESC-1);
                l_ltrim(desc_b);
                line_b[ii] = '\0';
                break;
            }
        }

        sscanf(line_b, "%s %s %s %s %s %s %s %s",
            args_b[0], args_b[1], args_b[2], args_b[3], 
			args_b[4], args_b[5], args_b[6], args_b[7]);
        if (strlen(args_b[0]) == 0 ||
            args_b[0][0] == '#' || args_b[0][0] == '*')
            continue;

        strcpy(proc_b, args_b[0]);      /* process name */
        /* search the section */
        if (proc_b[0] == '[')
        {
            if (section_f)          /* search OK */
                break;
            if (proc_b[strlen(proc_b)-1] == ']')
                proc_b[strlen(proc_b)-1] = '\0';
            if (strcmp(SECT_BOOT, &proc_b[1]) != 0)
                continue;

            section_f = 1;
            continue;
        }
        if (!section_f)
            continue;

        /* argument list */
        for (ii = 1; ii < M_ARGC; ii++)
        {
            if (strlen(args_b[ii]) <= 0)
                break;
            ll = strlen(proc_b);
            sprintf(&proc_b[ll], " %s", args_b[ii]);
        }

        for (ii = 0; ii < M_BOOT; ii++)
        {
            if (strlen(bootab.rec[ii].proc) == 0)
                continue;
            if (strcmp(bootab.rec[ii].proc, proc_b) != 0)
                continue;

            memset(bootab.rec[ii].desc, 0x00, L_DESC);
            memcpy(bootab.rec[ii].desc, desc_b, L_DESC-1);
            break;
        }

        if (ii >= M_BOOT)
        {
            strcpy(bootab.rec[bootab.nrec].proc, proc_b);
            memcpy(bootab.rec[bootab.nrec].desc, desc_b, L_DESC-1);
            bootab.nrec++;
        }
    }
    fclose(pro_F);
    return (0);
}

/******************************************************************************/
/* NAME : check_protab()                                                      */
/* DESC : Check process table's modification.                                 */
/******************************************************************************/
check_protab()
{
    int tidx;
    int ii, jj;

    for (tidx = 0; tidx < MAX_PT; tidx++)
    {
        if (x_process[tidx] == 0)
            continue;
        for (ii = 0; ii < M_PROC; ii++)
        {
            switch (protab[tidx][ii].cnfg)
            {
            case _RUN_:
                break;
            case _DEL_:
                if (protab[tidx][ii].xpid > 1)
                    kill(protab[tidx][ii].xpid, SIGTERM);
                protab[tidx][ii].xpid = 0;
                break;
            case _BAR_:
                if (protab[tidx][ii].xpid > 1)
                    kill(protab[tidx][ii].xpid, SIGTERM);
                protab[tidx][ii].xpid = 0;
                break;
            case _MOD_: /* modified */
                if (protab[tidx][ii].xpid > 1)
                    kill(protab[tidx][ii].xpid, SIGTERM);
                protab[tidx][ii].xpid = 0;
                protab[tidx][ii].cnfg = _RUN_;
                break;
            default:
                if (protab[tidx][ii].xpid > 1)
                    kill(protab[tidx][ii].xpid, SIGTERM);
                protab[tidx][ii].xpid = 0;
                protab[tidx][ii].cnfg = _DEL_;
                break;
            }
        }
        x_process[tidx] = 0;
    }
    return(0);
}

/******************************************************************************/
/* NAME : check_pbase()                                                       */
/* DESC : check base process table                                            */
/******************************************************************************/
check_pbase()
{
    FILE    *pro_F;
    char    line_b[256];
    char    args_b[M_ARGC][128], proc_b[64], desc_b[128];
    int ii, jj, ll;
    int section_f = 0;

    x_process[T_BAS] = 1;
    for (ii = 0; ii < M_PROC; ii++)
        protab[T_BAS][ii].cnfg = _DEL_;

    pro_F = fopen(file_path, "r");
    if (pro_F == NULL)
        return(0);

    while ((fgets(line_b, sizeof(line_b), pro_F)) == line_b)
    {
        ll = strlen(line_b);
        if (ll <= 4)
            continue;

        for (ii = ll-1; ii >= 0; ii--)
        {
            if (line_b[ii] != '\n' && line_b[ii] != '\r')
                break;
            line_b[ii] = '\0';
            ll--;
        }

        for (ii = 0; ii < M_ARGC; ii++)
            args_b[ii][0] = '\0';
        desc_b[0] = '\0';

        for (ii = ll-1; ii >= 0; ii--)
        {
            if (line_b[ii] == '#' || line_b[ii] == '*')
            {
                memcpy(desc_b, &line_b[ii+1], L_DESC-1);
                l_ltrim(desc_b);
                line_b[ii] = '\0';
                break;
            }
        }

        sscanf(line_b, "%s %s %s %s %s %s %s %s",
            args_b[0], args_b[1], args_b[2], args_b[3], 
			args_b[4], args_b[5], args_b[6], args_b[7]);
        if (strlen(args_b[0]) == 0 ||
            args_b[0][0] == '#' || args_b[0][0] == '*')
            continue;

        strcpy(proc_b, args_b[0]);      /* process name */
        /* search the section */
        if (proc_b[0] == '[')
        {
            if (section_f)          /* search OK */
                break;
            if (proc_b[strlen(proc_b)-1] == ']')
                proc_b[strlen(proc_b)-1] = '\0';
            if (strcmp(SECT_BASE, &proc_b[1]) != 0)
                continue;

            section_f = 1;
            continue;
        }
        if (!section_f)
            continue;

        /* argument list */
        for (ii = 1; ii < M_ARGC; ii++)
        {
            if (strlen(args_b[ii]) <= 0)
                break;
            ll = strlen(proc_b);
            sprintf(&proc_b[ll], " %s", args_b[ii]);
        }

        jj = -1;
        for (ii = 0; ii < M_PROC; ii++)
        {
            if (strlen(protab[T_BAS][ii].proc) == 0)
            {
                if (jj == -1)
                    jj = ii;
                continue;
            }
            if (strcmp(protab[T_BAS][ii].proc, proc_b) != 0)
                continue;
            memset(protab[T_BAS][ii].desc, 0x00, L_DESC);
            memcpy(protab[T_BAS][ii].desc, desc_b, L_DESC-1);
            protab[T_BAS][ii].cnfg = _RUN_;
            break;
        }
        if (ii >= M_PROC && jj != -1)
        {
            memset(&protab[T_BAS][jj], 0x00, sizeof(struct protab));
            strcpy(protab[T_BAS][jj].proc, proc_b);
            memcpy(protab[T_BAS][jj].desc, desc_b, L_DESC-1);
            protab[T_BAS][jj].cnfg = _RUN_;
            protab[T_BAS][jj].type = T_BAS;
        }
    }
    fclose(pro_F);
    return (0);
}

/******************************************************************************/
/* NAME : check_ptime()                                                       */
/* DESC : check timely process table                                          */
/******************************************************************************/
check_ptime()
{
    FILE    *pro_F;
    char    line_b[256];
    char    args_b[M_ARGC][128], proc_b[64], desc_b[128];
    char    tmpbuf[8][8], *hour_b, *min_b, wday_b[8];
    int ii, jj, kk, ll;
    int tmhh, tmmm;
    char    section_f = 0;
    char    fr_wk[4], to_wk[4], range_f;

    x_process[T_TIM] = 1;
    for (ii = 0; ii < M_PROC; ii++)
        protab[T_TIM][ii].cnfg = _DEL_;

    pro_F = fopen(file_path, "r");
    if (pro_F == NULL)
        return(0);

    while ((fgets(line_b, sizeof(line_b), pro_F)) == line_b)
    {
        ll = strlen(line_b);
        if (ll <= 4)
            continue;

        for (ii = ll-1; ii >= 0; ii--)
        {
            if (line_b[ii] != '\n' && line_b[ii] != '\r')
                break;
            line_b[ii] = '\0';
            ll--;
        }

        for (ii = 0; ii < M_ARGC; ii++)
            args_b[ii][0] = '\0';
        desc_b[0] = '\0';

        for (ii = ll-1; ii >= 0; ii--)
        {
            if (line_b[ii] == '#' || line_b[ii] == '*')
            {
                memcpy(desc_b, &line_b[ii+1], L_DESC-1);
                l_ltrim(desc_b);
                line_b[ii] = '\0';
                break;
            }
        }

        sscanf(line_b, "%s %s %s", args_b[0], args_b[1], args_b[2]);
        if (strlen(args_b[0]) == 0 ||
            args_b[0][0] == '#' || args_b[0][0] == '*')
            continue;

        strcpy(proc_b, args_b[0]);      /* process name */
        /* search the section */
        if (proc_b[0] == '[')
        {
            if (section_f)          /* search OK */
                break;
            if (proc_b[strlen(proc_b)-1] == ']')
                proc_b[strlen(proc_b)-1] = '\0';
            if (strcmp(SECT_TIME, &proc_b[1]) != 0)
                continue;

            section_f = 1;
            continue;
        }
        if (!section_f)
            continue;

        /* execution hour and minute */
        if (strlen(args_b[1]) == 0 || args_b[1][0] == '#')
        {
            /* warning */
            l_syslog(myname, "Time is omitted [%s]", args_b[0]);
            continue;
        }

        memset(tmpbuf, 0x00, sizeof(tmpbuf));
        hour_b = tmpbuf[0];
        min_b  = tmpbuf[1];
        for (ii = jj = kk = 0; ii < strlen(args_b[1]); ii++)
        {
            if (kk >= 2 || jj >= 3)
                break;
            switch (args_b[1][ii])
            {
            case ':' :
                kk++; jj = 0;
                break;
            default :
                if (!isdigit(args_b[1][ii]) &&
                    args_b[1][ii] != '*')
                    break;
                tmpbuf[kk][jj++] = args_b[1][ii];
                break;
            }
        }
        if (*hour_b == '*')
            strcpy(hour_b, "-1");       /* 매분 (-1) */
        if (*min_b == '*')
            strcpy(min_b, "-1");        /* 매시 (-1) */
        tmhh = atoi(hour_b);
        tmmm = atoi(min_b);

        /* execution week */
        range_f = 0;
        memset(wday_b, 0x00, sizeof(wday_b));
        if (strlen(args_b[2]) == 0 ||
            args_b[2][0] == '#' || args_b[2][0] == '*')
            wday_b[7] = 1;          /* default 매일 */
        else
        {
            memset(tmpbuf, 0x00, sizeof(tmpbuf));
            memset(fr_wk, 0x00, sizeof(fr_wk));
            memset(to_wk, 0x00, sizeof(to_wk));
            for (ii = jj = kk = 0; ii < strlen(args_b[2]); ii++)
            {
                if (kk >= 7)
                    break;
                switch (args_b[2][ii])
                {
                case ',':
                    kk++; jj = 0;
                    break;
                case '-':
                    range_f = 1;
                    strcpy(fr_wk, tmpbuf[kk]);
                    kk++; jj = 0;
                    break;
                default :
                    if (!isdigit(args_b[2][ii]))
                        break;
                    tmpbuf[kk][jj] = args_b[2][ii];
                    if (range_f)
                        to_wk[jj] = args_b[2][ii];
                    jj++;
                    break;
                }
            }
            if (range_f)
            {
                memset(tmpbuf, 0x00, sizeof(tmpbuf));
                for (ii = atoi(fr_wk); ii <= atoi(to_wk); ii++)
                    tmpbuf[ii][0] = ii + '0';
            }
            for (ii = 0; ii < 7; ii++)
            {
                if (tmpbuf[ii][0] >= '0' &&
                    tmpbuf[ii][0] <= '6')
                    wday_b[tmpbuf[ii][0] - '0'] = 1;
            }
        }

        jj = -1;
        for (ii = 0; ii < M_PROC; ii++)
        {
            if (strlen(protab[T_TIM][ii].proc) == 0)
            {
                if (jj == -1)
                    jj = ii;
                continue;
            }
            if (strcmp(protab[T_TIM][ii].proc, proc_b) ||
                protab[T_TIM][ii].tmhh != tmhh ||
                protab[T_TIM][ii].tmmm != tmmm ||
                memcmp(protab[T_TIM][ii].wday, wday_b, 8))
                continue;
            memset(protab[T_TIM][ii].desc, 0x00, L_DESC);
            protab[T_TIM][ii].tmhh = tmhh;
            protab[T_TIM][ii].tmmm = tmmm;
            memcpy(protab[T_TIM][ii].wday, wday_b, 8);
            memcpy(protab[T_TIM][ii].desc, desc_b, L_DESC-1);
            protab[T_TIM][ii].cnfg = _RUN_;
            break;
        }
        if (ii >= M_PROC && jj != -1)
        {
            memset(&protab[T_TIM][jj], 0x00, sizeof(struct protab));
            strcpy(protab[T_TIM][jj].proc, proc_b);
            protab[T_TIM][jj].tmhh = tmhh;
            protab[T_TIM][jj].tmmm = tmmm;
            memcpy(protab[T_TIM][jj].wday, wday_b, 8);
            memcpy(protab[T_TIM][jj].desc, desc_b, L_DESC-1);
            protab[T_TIM][jj].cnfg = _RUN_;
            protab[T_TIM][jj].type = T_TIM;
        }
    }
    fclose(pro_F);
    return (0);
}

/******************************************************************************/
/* NAME : do_exec()                                                           */
/* DESC : execution all configured process                                    */
/******************************************************************************/
do_exec(flag)
int flag;
{
    time_t  clock;
    pid_t   xpid;
    int ii, jj;
    char    proc_b[64];

    for (ii = 0; ii < MAX_PT; ii++)
    {
        for (jj = 0; jj < M_PROC; jj++)
        {
            switch (protab[ii][jj].cnfg)
            {
            case _RUN_:
                if (strlen(protab[ii][jj].proc) <= 0)
                    continue;
                if (protab[ii][jj].xpid > 0)
                    continue;
                break;
            default:
                continue;
            }

            /* check the timely process */
            if (protab[ii][jj].type == T_TIM)
            {
                if (chk_time_proc(&protab[ii][jj]) == 0)
                    continue;

                /* kill the old process */
                proc_b[0] = '\0';
                sscanf(protab[ii][jj].proc, "%s", proc_b);
                xpid = l_getpid(proc_b, NULL, NULL);
                if (xpid > 100)
                    kill(xpid, SIGKILL);
            }

            clock = time(0);
            if (protab[ii][jj].extm > clock)
                continue;
            clock -= protab[ii][jj].extm;
            if (clock <= 5 && protab[ii][jj].excc > 3)
            {
                protab[ii][jj].excc = 0;
                protab[ii][jj].extm = clock + 10;
                sprintf(log_m, "'%s' requires to be checked",
                    protab[ii][jj].proc);
                l_syslog(myname, log_m);

                continue;
            }

            xpid = exec_proc(protab[ii][jj].proc);
            if (xpid < 0)
                continue;

            protab[ii][jj].xpid = xpid;
            protab[ii][jj].excc++;
            protab[ii][jj].extm = time(0);
        }
    }
}

/*****************************************************************************/
/* NAME : chk_time_proc()                                                    */
/* DESC : 특정 시간대별 PROCESS 실행                                         */
/*****************************************************************************/
chk_time_proc(timepro)
struct  protab  *timepro;
{
    int check_id;

    if (!timepro->wday[7])      /* 특정 요일에 실행 */
    {
        if (!timepro->wday[now.tm_wday])
            return(0);
    }

    if (timepro->tmhh < 0 && timepro->tmmm < 0)
        return(0);
    if (timepro->tmhh < 0)      /* 매시간 지정 분에 실행 */
    {
        if (now.tm_min != timepro->tmmm)
            return(0);
        check_id = (now.tm_hour*60) + now.tm_min;
    }
    else if (timepro->tmmm < 0) /* 지정 시간에 */
    {
        if (now.tm_hour != timepro->tmhh)
            return(0);
        check_id = (timepro->tmhh * 60);
    }
    else
    {
        if (now.tm_hour != timepro->tmhh ||
            now.tm_min  != timepro->tmmm)
            return(0);
        check_id = (now.tm_hour * 60) + now.tm_min;
    }

    /* if already processed, skip it */
    if (timepro->chck[check_id] != 0)
        return(0);

    timepro->chck[check_id] = 1;
    return(1);
}

/******************************************************************************/
/* NAME : killall_proc()                                                      */
/* DESC : Kill all remaining process                                          */
/******************************************************************************/
int killall_proc()
{
    FILE    *chk_F;
    char    *logf;
    char    command[256];
    char    line_b[256], path_b[128], chck_b[128];
    char    uid_b[16], pid_b[16], ppid_b[16], proc_b[64], dumy_b[64];
    char    *name_b, argv_b[8][64], find_ok;
    pid_t   xpid;
    int ii, jj, argv_n;

    /* kill all remaining processes */
    for (ii = 0; ii < bootab.nrec; ii++)
    {
        if ((xpid = bootab.rec[ii].xpid) <= 0)
            continue;

        kill(xpid, SIGTERM);
        bootab.rec[ii].xpid = 0;
    }
    for (ii = 0; ii < MAX_PT; ii++)
    {
        for (jj = 0; jj < M_PROC; jj++)
        {
            protab[ii][jj].excc = 0;
            if ((xpid = protab[ii][jj].xpid) <= 0)
                continue;

            kill(xpid, SIGTERM);
            protab[ii][jj].xpid = 0;
        }
    }

    sprintf(command, "Cx%d", getpid());
    logf = tempnam(P_tmpdir, command);
    if (logf == NULL)
        return (-1);

    sprintf(path_b, "%s/", BIN_DIR);
    sprintf(command, "%s %s|/bin/grep %s|/bin/grep -v grep > %s 2>&1",
        BIN_PS, PS_OPTS, path_b, logf);
    system(command);
    chk_F = fopen(logf, "r");
    if (chk_F == NULL)
        return(0);

    while ((fgets(line_b, sizeof(line_b), chk_F)) == line_b)
    {
        uid_b[0] = '\0';
        pid_b[0] = '\0';
        ppid_b[0] = '\0';
        proc_b[0] = '\0';
        for (ii = 0; ii < 8; ii++)
            argv_b[ii][0] = '\0';

        sscanf(line_b, "%s %s %s %s %s %s %s %s %s %s %s %s",
            uid_b, pid_b, ppid_b, dumy_b,
            argv_b[0], argv_b[1], argv_b[2], argv_b[3],
            argv_b[4], argv_b[5], argv_b[6], argv_b[7]);
        if (uid_b[0] <= ' ')
            continue;

        xpid = atoi(pid_b);
        if (xpid <= 10)
            continue;

        for (ii = 2; ii < 8; ii++)
        {
            if (strchr(argv_b[ii], ':'))
                break;
        }
        if (ii >= 8)
            continue;

        strcpy(proc_b, argv_b[ii+1]);
        name_b = basename(proc_b);
        if (name_b == NULL)
            continue;

        find_ok = 0;
        for (ii = 0; ii < bootab.nrec; ii++)
        {
            chck_b[0] = '\0';
            sscanf(bootab.rec[ii].proc, "%s", chck_b);
            if (strlen(chck_b) <= 0 ||
                strcmp(chck_b, name_b) != 0)
                continue;

            kill(xpid, SIGKILL);
            find_ok = 1;
            break;
        }
        if (find_ok)
            continue;

        for (ii = 0; ii < MAX_PT; ii++)
        {
            for (jj = 0; jj < M_PROC; jj++)
            {
                chck_b[0] = '\0';
                sscanf(protab[ii][jj].proc, "%s", chck_b);
                if (strlen(chck_b) <= 0 ||
                    strcmp(chck_b, name_b) != 0)
                    continue;

                kill(xpid, SIGKILL);
                break;
            }
        }
    }
    fclose(chk_F);
    unlink(logf);
    (void)free(logf);

    return (0);
}

/******************************************************************************/
/* NAME : clear_chck_board()                                                  */
/* DESC : clear the minutely checked board when the day changed.              */
/******************************************************************************/
clear_chck_board()
{
    int ii, jj;
    time_t  clock;
    struct  tm  *ctm;

    for (ii = 0; ii < MAX_PT; ii++)
    {
        for (jj = 0; jj < M_PROC; jj++)
        {
            memset(&protab[ii][jj].chck, 0x00,
                    sizeof(protab[ii][jj].chck));
            protab[ii][jj].excc = 0;
        }
    }

    clock = time(0);
    ctm = localtime(&clock);
    memcpy(&today, ctm, sizeof(struct tm));

    return(0);
}

/******************************************************************************/
/* NAME : exec_boot_job()                                                     */
/* DESC : 시작시 실행 process                                                 */
/******************************************************************************/
exec_boot_job()
{
    int xpid = 0; 
	int	status;
    int ii;
    char    *prog;

    signal(SIGCHLD, ign_exit_job);
    sighold(SIGCHLD);

    l_syslog(myname, "ON-BOOT JOB 시작");
    for (ii = 0; ii < bootab.nrec; ii++)
    {
        prog = basename(bootab.rec[ii].proc);
        l_syslog(myname, "ON-BOOT [%s] 실행", prog);

        bootab.rec[ii].xpid = exec_proc(bootab.rec[ii].proc);
        xpid = waitpid(-1, &status, 0);
        if (xpid < 0 || WEXITSTATUS(status) != 0)
        {
            sprintf(log_m, "ON-BOOT [%s] 이상", prog);
            break;
        }
        else
            sprintf(log_m, "ON-BOOT [%s] 정상", prog);
        l_syslog(myname, log_m);
    }
    if (xpid < 0)
    {
        l_syslog(myname, "ON-BOOT 작업중 오류. COMLOG* 파일 확인 요망");
        return(-1);
    }

    l_syslog(myname, "ON-BOOT JOB 완료");
    return(0);
}

/******************************************************************************/
/* NAME : exec_proc()                                                         */
/* DESC : fork and execute the process                                        */
/******************************************************************************/
exec_proc(char *procnm)
{
    int ii;
    int rc;
    pid_t   xpid;
    char    argv_b[M_ARGC][32], *argv_p[M_ARGC+1], prog[64];

    for (ii = 0; ii < M_ARGC; ii++)
        argv_b[ii][0] = '\0';
    sscanf(procnm, "%s %s %s %s %s %s %s %s", 
			prog, argv_b[1], argv_b[2], argv_b[3], 
			argv_b[4], argv_b[5], argv_b[6], argv_b[7]);

    if (prog[0] == '/')
        strcpy(argv_b[0], prog);
    else
	{
		if (strstr(prog, ".sh") != NULL)
			sprintf(argv_b[0], "%s/%s", SHL_DIR, prog);
		else
			sprintf(argv_b[0], "%s/%s", BIN_DIR, prog);
	}
    for (ii = 0; ii < M_ARGC; ii++)
    {
        if (strlen(argv_b[ii]) <= 0)
            break;
        argv_p[ii] = argv_b[ii];
    }
    argv_p[ii] = NULL;
    l_syslog(myname, "PROCESS EXEC [%s]", procnm);

    switch((xpid = fork()))
    {
    case -1: return(-1);
    case  0: rc = execv(argv_b[0], argv_p);
	l_syslog(myname, "[%s] rc= [%d] errno=(%d)", argv_b[0], rc, errno);
         exit(1);
    default: return(xpid);
    }
}

/******************************************************************************/
/* NAME : clean_zombie()                                                      */
/* DESC : clean zombie process                                                */
/******************************************************************************/
clean_zombie()
{
    int status;
    pid_t   xpid;
    int ii, jj;

    for (;;)
    {
        xpid = waitpid(-1, &status, WNOHANG);
        if (xpid <= 0)
            break;
        for (ii = 0; ii < MAX_PT; ii++)
        {
            for (jj = 0; jj < M_PROC; jj++)
            {
                if (protab[ii][jj].xpid == xpid)
                {
                    protab[ii][jj].xpid = 0;
                    l_syslog(myname, "PROCESS EXIT [%s]",
                        protab[ii][jj].proc);
                }
            }
        }
    }
    return (0);
}

/******************************************************************************/
/* NAME : ign_exit_job()                                                      */
/* DESC : SIGCHLD signal handler                                              */
/******************************************************************************/
void ign_exit_job(signo)
int signo;
{
    int status;
    int pid;

    pid = waitpid(-1, &status, WNOHANG);
    signal(SIGCHLD, ign_exit_job);
}

/******************************************************************************/
/* NAME : stop_system()                                                       */
/* DESC : Stop messenger system.                                              */
/******************************************************************************/
void stop_system(int signo)
{
    killall_proc();
    l_syslog(myname, "SYSTEM MANAGER STOP [%s.%s]", CONF_FILE, svc_grp);
    exit(0);
}

/******************************************************************************/
/* NAME : chld_handler()                                                      */
/* DESC : SIGCHLD signal handler                                              */
/******************************************************************************/
void chld_handler(int signo)
{
    sighold(SIGCHLD);
}
