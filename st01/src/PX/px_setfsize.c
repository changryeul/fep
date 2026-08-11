#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : change the record size of a file
#   File    : px_setfsize.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    char    file_name[20], file_size[8], sub[4], buf[128];
    int     dk, fk;
    FILE    *fp;

    if (argc != 1 && argc != 3) {
        printf("==========================================================\n");
        printf("[change the record size of a file]\n\n");
        printf("Usage: %s <file name> <size>\n\n", argv[0]);
        printf("  e.g. %s %cc_4101_ts 400\n", argv[0], argv[0][0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    memset(file_name, 0, sizeof (file_name));
    memset(file_size, 0, sizeof (file_size));

    if (argc == 3) {
        memcpy(file_name, argv[1], strlen(argv[1]));
        memcpy(file_size, argv[2], strlen(argv[2]));
    }
    else {
        printf("\033[1mfile name(e.g. jc_4101_ts) ?\033[0m ");
        fflush(stdout);
        fgets(file_name, sizeof(file_name), stdin);
        if (file_name[0] == '\0')
            exit(FAIL);

        printf("\033[1mrecord size ?\033[0m ");
        fflush(stdout);
        fgets(file_size, sizeof(file_size), stdin);
        if (file_size[0] == '\0')
            exit(FAIL);
    }

    if (strlen(file_name) != 10) {
        printf("ERROR:file name[%s]\n", file_name);
        exit(FAIL);
    }

    /* initialize global variables and attach to daemon SHM (INFO)  */
    Init_Mana(argc, argv);

    sprintf(sub, "%-2.2s", file_name);
    LtoU(sub, 2);
    dk = sub[1] - 'A';

    if (INFO(dk).process_id[0] == 0) {
        printf("%s daemon not registered !!!\n", sub);
        exit(FAIL);
    }

    for (fk = 0; fk < DAEMON(dk).f_count; fk ++) {
        if (memcmp(FILEM(dk,fk).file_name, file_name, 10) == 0) {
            printf("record_size changed [%s: %d ->",
                    file_name, FILEM(dk,fk).record_size);

            FILEM(dk,fk).record_size = atoi(file_size);

            printf(" %d]\n", FILEM(dk,fk).record_size);
            break;
        }

        if (fk == DAEMON(dk).f_count - 1) {
            printf("ERROR:file not registered[%s]\n", file_name);
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

    Log(USR_OK, "[%s: %s, %s]", buf, file_name, file_size);

    exit(OK);
}   /* End of main ()   */

/*************************************************************************
    End of Program (px_setfsize.c)
*************************************************************************/
