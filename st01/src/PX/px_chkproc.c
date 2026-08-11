#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : check process info (PROC)
#   File    : px_chkproc.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
char    Type[4];

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    Process_Info(int, int, int);

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    int     dk, pk;
    char    sub[4];

    if (argc != 2 && argc != 3) {
        printf("==========================================================\n");
        printf("[check process info(PROC)]\n\n");
        printf("Usage: %s <sub name> <type>\n\n", argv[0]);
        printf("  e.g. 1) %s ja\n", argv[0]);
        printf("       2) %s ja ts\n", argv[0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    /* initialize global variables and attach to daemon SHM (INFO)  */
    Init_Mana(argc, argv);

    memset(sub, 0, sizeof (sub));
    memcpy(sub, argv[1], strlen(argv[1]));
    LtoU(sub, 2);
    dk = sub[1] - 'A';

    if (argc == 3) {
        memset(Type, 0, sizeof (Type));
        memcpy(Type, argv[2], strlen(argv[2]));
    }

    if (INFO(dk).process_id[0] == 0) {
        printf("%s daemon not registered !!!\n", sub);
        exit(FAIL);
    }

    puts("\033[1m* D:data_cnt P:process_status PNO:process_no TYP:process_type TOUT:timeout\033[0m");
    puts("\033[1m  S:start_status\033[0m");
    printf("\033[4m[%s, %s, %s ~ %s]\033[0m\n", sub, DAEMON(dk).process_info,
            DAEMON(dk).start_time, DAEMON(dk).end_time);
    printf("\033[7mID         D P_K P S        PNO STM  ETM  TYP TOUT Info                          \033[0m\n");

    for (pk = 0; pk < DAEMON(dk).p_count; pk ++)
        Process_Info(dk, pk, argc);

    exit(OK);
}   /* End of main ()   */

/*----------------------------------------------------------------------*/
void    Process_Info(int i, int j, int argnum)
/*----------------------------------------------------------------------*/
{
    char    info[44];

    if (argnum == 3 &&
            memcmp(PROC(i,j).process_id+8, Type, strlen(Type)) != 0)
    return;

    memset(info, 0, sizeof (info));
    memcpy(info, PROC(i,j).process_info, sizeof (PROC(i,j).process_info));

    /* UTF-8: pad incomplete multi-byte char at boundary */
    {
        int p = 29;
        while (p > 0 && Chk_Korean(info, p) == -1) p--;
        while (p < 29) info[p++] = ' ';
    }

    if (PROC(i,j).process_id[8] == 'p')
        printf("\033[32m%10.10s\033[0m", PROC(i,j).process_id);
    else if (PROC(i,j).process_id[8] == 't')
        printf("\033[33m%10.10s\033[0m", PROC(i,j).process_id);
    else if (PROC(i,j).process_id[8] == 'q')
        printf("\033[35m%10.10s\033[0m", PROC(i,j).process_id);
    else if (PROC(i,j).process_id[8] == 'b')
        printf("\033[34m%10.10s\033[0m", PROC(i,j).process_id);
    else if (PROC(i,j).process_id[8] == 'd')
        printf("\033[36m%10.10s\033[0m", PROC(i,j).process_id);
    else
        printf("%10.10s", PROC(i,j).process_id);

    if (PROC(i,j).process_no == 0)
        printf("\033[1m %d %3d %d %d %10d %4.4s %4.4s %-3.3s %4d %-29.29s\033[0m",
                DATA_CNT(i,j), j, PROC(i,j).process_status,
                PROC(i,j).start_status, PROC(i,j).process_no, PROC(i,j).start_time,
                PROC(i,j).end_time, PROC(i,j).process_type, PROC(i,j).timeout, info);
    else
        printf(" %d %3d %d %d %10d %4.4s %4.4s %-3.3s %4d %-29.29s",
                DATA_CNT(i,j), j, PROC(i,j).process_status,
                PROC(i,j).start_status, PROC(i,j).process_no,
                PROC(i,j).start_time, PROC(i,j).end_time,
                PROC(i,j).process_type, PROC(i,j).timeout, info);

    printf("\n");

    return;
}   /* End of Process_Info ()   */

/*************************************************************************
    End of Program (px_chkproc.c)
*************************************************************************/
