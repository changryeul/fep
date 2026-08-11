#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : change the data size of data SHM
#   File    : px_setdsize.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    char    data_name[20], data_size[8], sub[4], buf[128];
    int     dk, fk;
    FILE    *fp;

    if (argc != 1 && argc != 3) {
        printf("==========================================================\n");
        printf("[change the data size of data SHM]\n\n");
        printf("Usage: %s <data name> <size>\n\n", argv[0]);
        printf("  e.g. %s %cc_1101_ts 210\n", argv[0], argv[0][0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    memset(data_name, 0, sizeof (data_name));
    memset(data_size, 0, sizeof (data_size));

    if (argc == 3) {
        memcpy(data_name, argv[1], strlen(argv[1]));
        memcpy(data_size, argv[2], strlen(argv[2]));
    }
    else {
        printf("\033[1mdata SHM name(e.g. jc_1101_ts) ?\033[0m ");
        fflush(stdout);
        fgets(data_name, sizeof(data_name), stdin);
        if (data_name[0] == '\0')
            exit(FAIL);

        printf("\033[1mrecord size ?\033[0m ");
        fflush(stdout);
        fgets(data_size, sizeof(data_size), stdin);
        if (data_size[0] == '\0')
            exit(FAIL);
    }

    if (strlen(data_name) != 10) {
        printf("ERROR:data SHM name[%s]\n", data_name);
        exit(FAIL);
    }

    /* initialize global variables and attach to daemon SHM (INFO)  */
    Init_Mana(argc, argv);

    sprintf(sub, "%-2.2s", data_name);
    LtoU(sub, 2);
    dk = sub[1] - 'A';

    if (INFO(dk).process_id[0] == 0) {
        printf("%s daemon not registered !!!\n", sub);
        exit(FAIL);
    }

    for (fk = 0; fk < DAEMON(dk).d_count; fk ++) {
        if (memcmp(DSHM(dk,fk).data_name, data_name, 10) == 0) {
            printf("data_size changed [%s: %d ->",
                    data_name, DSHM(dk,fk).data_size);

            DSHM(dk,fk).data_size = atoi(data_size);

            printf(" %d]\n", DSHM(dk,fk).data_size);
            break;
        }

        if (fk == DAEMON(dk).d_count - 1) {
            printf("ERROR:data SHM not registered[%s]\n", data_name);
            exit(FAIL);
        }
    }

#if defined __hpux
    fp = popen("who -mR", "r");
#elif defined(sun) || defined(_AIX) || defined(__linux)
    fp = popen("who -m", "r");
#endif
    fgets(buf, sizeof (buf), fp);
    pclose(fp);
    buf[strlen(buf)-1] = '\0';

    Log(USR_OK, "[%s: %s, %s]", buf, data_name, data_size);

    exit(OK);
}   /* End of main ()   */

/*************************************************************************
    End of Program (px_setdsize.c)
*************************************************************************/
