#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : change the read/write count of a file or data SHM
#   File    : px_setatstop.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    char    pname[20], w_buf[KRX_DATA_BUFF_SIZE], tmp[128];
    int     pk, rt;

    if (argc != 2) {
        printf("==========================================================\n");
        printf("[Set Auto Process Stop(500300)] argc[%d]\n\n", argc);
        printf("Usage: %s <process name>\n\n", argv[0]);
        printf("  e.g. %s pa_50101mp\n", argv[0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    memset(pname, 0, sizeof(pname));
    memcpy(pname, argv[1], strlen(argv[1]));
    /* 찾는로직도 8201TR과 동일하게 처리 */
    pk = (AtoIf(&pname[4], 2) - 1) * 10;
    pk =  AtoIf(&pname[6], 2) + 3 + pk;

    if (pk < 0+3 || pk > MAX_AUTO_PROC+3) {
        printf("==========================================================\n");
        printf("[Set Auto Process Stop(500300)] argc[%d]\n\n", argc);
        printf("Usage: %s <process name>\n\n", argv[0]);
        printf("  e.g. %s pa_50101mp\n", argv[0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    printf("START\n");

    /* initialize global variables and attach to daemon SHM (INFO) */
    Init_Mana(argc, argv);

    memset(tmp,    0,      sizeof(tmp));
    sprintf(tmp, "0000000100000001AUTOSTOP0000                                          50030000000000%5.5sA", &pname[3]);  // 어느 Client에서 온지 모르니 양쪽 다 보낸다.
    memset(w_buf,  0,      sizeof(w_buf));
    memset(w_buf,  0x20,   70+2048);
    memcpy(w_buf, tmp, strlen(tmp));
    w_buf[70+2048] = '\n';
    rt = DSHM_WT(pk*10, w_buf, 1);

    if (rt < 0)
        printf("Stop(500300) Write Error pname[%s]\n", pname);
    else
        printf("Stop(500300) Write OK pname[%s]\n", pname);

    exit(OK);
}   /* End of main ()   */

/*************************************************************************
    End of Program (px_setatstop.c)
*************************************************************************/
