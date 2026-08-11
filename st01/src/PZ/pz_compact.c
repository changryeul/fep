#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : unlink the expired data and log files
#   File    : pz_compact.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    <dirent.h>
#include    "fep_sub.h"

extern void     Sub_SHM(void);

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define     DEF_DAYS    "7"                 /* default expiration days  */
#define     DAY_SEC     86400L              /* 24 * 60 * 60 (sec)       */

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
int     Days;                                       /* expiration days  */
char    ProcDate[10], Path[256];
int     Dk = -1;

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    Check_Argument(int argc, char *argv[]);
static  void    Main_Process(void);
void    Log_Unlink(int);
void    Dat_Unlink(int);
void    Unlink_Proc(char *);

/*************************************************************************
    Function        : . main
    Parameters IN   : . argc    : number of arguments
                      . argv[0] : execution name
                      . argv[1] : expiration days
                      . argv[2] : sub system name
    Parameters OUT  : .
    Return Code     : . 0 (OK)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     main(int argc, char **argv)
/*----------------------------------------------------------------------*/
{
    _System_Name[0] = argv[0][0];
    LtoU(_System_Name, 1);
    sprintf(_Exe_Name, "%s", argv[0]);
    sprintf(_SubSystem_Name, "%-2.2s", argv[0]);
    sprintf(_Process_Name, "%s", argv[0]);

    /* check environment values */
    Check_Environment();

    /* check arguments  */
    Check_Argument(argc,argv);

    /* main routine */
    Main_Process();

    exit(OK);
}   /* End of main ()   */

/*************************************************************************
    Function        : . arguments check
    Parameters IN   : . argc    : arguments count
                      . argv[0] : execution name
                      . argv[1] : expiration days
                      . argv[2] : sub system name
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Check_Argument(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    if (argc == 1) {
        Days = atoi(DEF_DAYS);
        return;
    }

    if (argc > 3) {
        puts("==========================================================");
        printf("ERROR: too many arguments [%d]\n", argc);
        printf("USAGE: %s <expiration days(0 ~ %s)> <sub system name(a ~ z)>\n", argv[0], DEF_DAYS);
        printf("  e.g. 1) %s\n", argv[0]);
        printf("       2) %s %s\n", argv[0], DEF_DAYS);
        printf("       3) %s %s b\n", argv[0], DEF_DAYS);
        puts("==========================================================");
        exit(FAIL);
    }

    if (_SubSystem_Name[1] != 'z') {
        Log(USR_FATAL, "invalid sub name [%s]", _SubSystem_Name);
        exit(FAIL);
    }

    Days = atoi(argv[1]);
    if (Days < 0) {
        Log(USR_ERROR, "invalid expiration days [%d]", Days);
        puts("==========================================================");
        printf("ERROR: invalid expiration days [%d]\n", Days);
        printf("Usage: %s <expiration days(0 ~ %s)> <sub system name(a ~ z)>\n", argv[0], DEF_DAYS);
        printf("  e.g. 1) %s\n", argv[0]);
        printf("       2) %s %s\n", argv[0], DEF_DAYS);
        printf("       3) %s %s b\n", argv[0], DEF_DAYS);
        puts("==========================================================");
        exit(FAIL);
    }

    if (argc == 3)
        Dk = argv[2][0] - 'a';

    return;
}   /* End of Check_Argument () */

/*************************************************************************
    Function        : . main routine
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Main_Process(void)
/*----------------------------------------------------------------------*/
{
    int         i;
    char        dt[20], d_time[16];
    time_t      t;
    struct tm   tm, *tp;

    /* attach daemon SHM (INFO) */
    Sub_SHM();

    for (i = 0; i < SHM_MAX_SUB; i++) {
        if ((Dk == -1 && (i + 'A' == 'X' || i + 'A' == 'Y' ||
                i + 'A' == 'Z' || INFO(i).process_count != 0)) || Dk == i) {
            if (i + 'A' >= 'W' && i + 'A' <= 'Z')
                time(&t);
            else {
                Get_DateTime(d_time);

                memset(dt, 0, sizeof (dt));
                sprintf(dt, "%.4s-%.2s-%.2s %.2s:%.2s:%.2s",
                        INFO(i).date, INFO(i).date+4, INFO(i).date+6,
                        d_time+8, d_time+10, d_time+12);
                strptime(dt, "%Y-%m-%d %H:%M:%S", &tm);
                t = mktime(&tm);
            }

            t -= (DAY_SEC * Days);
            tp = localtime(&t);
            strftime(ProcDate, 10, "%Y%m%d", tp);
            Log(USR_OK, "expiration date [%c%c,%s]",
                    _System_Name[0], i + 'A', ProcDate);

            Log_Unlink(i);

            if (Dk + 'A' == 'Z') {
                Log_Unlink('W' - 'A');
                Log_Unlink('X' - 'A');
                Log_Unlink('Y' - 'A');
            }

            Dat_Unlink(i);
        }
    }

    return;
}   /* End of Main_Process ()   */

/*************************************************************************
    Function        : . clear the expired log
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Log_Unlink(int dk)
/*----------------------------------------------------------------------*/
{
    sprintf(Path, "%s/%c%c", _FEP_LOG, _System_Name[0], dk + 'A');
    Log(USR_OK, "unlinking ... [%s]", Path);
    Unlink_Proc("LOG");

    return;
}   /* End of Log_Unlink () */

/*************************************************************************
    Function        : . clear the expired data
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Dat_Unlink(int dk)
/*----------------------------------------------------------------------*/
{
    sprintf(Path, "%s/%c%c", _FEP_DAT, _System_Name[0], dk + 'A');
    Log(USR_OK, "unlinking... [%s]", Path);
    Unlink_Proc("DAT");

    return;
}   /* End of Dat_Unlink () */

/************************************************************************
    Function        : . unlink the expired directories and files
    Parameters IN   : . p_buf   : work flag
    Parameters OUT  : .
    Return Code     : . void
************************************************************************/
/*---------------------------------------------------------------------*/
void    Unlink_Proc(char *p_buf)
/*---------------------------------------------------------------------*/
{
    char            tmp[1024];
    DIR             *dirfd;
    struct dirent   *dir;

    if ((dirfd = opendir(Path)) == NULL)
        return;

    while ((dir = readdir(dirfd)) != NULL) {
        if (memcmp(dir->d_name, ".", 1) == 0 ||
                memcmp(dir->d_name, "..", 2) == 0 ||
                memcmp(dir->d_name, "00000000", 8) == 0)
        continue;

        if (strlen(dir->d_name) == 8 && memcmp(dir->d_name, ProcDate, 8) <= 0) {
            memset(tmp, 0, sizeof(tmp));
            sprintf(tmp, "rm -rf %s/%s", Path, dir->d_name);
            system(tmp);
            Log(USR_OK, "%s/%s unlinked", Path, dir->d_name);
        }
    }
    closedir(dirfd);

    return;
}   /* End of Unlink_Proc ()    */

/*************************************************************************
    End of Program (pz_compact.c)
*************************************************************************/
