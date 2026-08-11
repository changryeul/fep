#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : change server IP address of TCP2 process
#   File    : px_sett2ip.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    int     dk, pk, l;
    struct in_addr  tmp_ia;
    char    buf[128], ip[16], line[4], proc_id[12], sub[4];
    FILE    *fp;

    if (argc != 1 && argc != 4) {
        printf("==========================================================\n");
        printf("[change server IP address of TCP2 process]\n\n");
        printf("Usage: %s <process ID> <line(P|B|A)> <IP address>\n\n",
                argv[0]);
        printf("       (line = P:primary B:backup A:all)\n");
        printf("  e.g. %s %cc_1101_ts P 200.100.3.191\n",
                argv[0], argv[0][0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    memset(proc_id, 0, sizeof (proc_id));
    memset(line, 0, sizeof (line));
    memset(ip, 0, sizeof (ip));

    if (argc == 4) {
        memcpy(proc_id, argv[1], strlen(argv[1]));
        memcpy(line, argv[2], strlen(argv[2]));
        memcpy(ip, argv[3], strlen(argv[3]));
    }
    else {
        printf("\033[1mprocess ID(e.g. %cc_1101_ts) ?\033[0m ", argv[0][0]);
        fflush(stdout);
        fgets(proc_id, sizeof(proc_id), stdin);
        if (proc_id[0] == '\0')
            exit(FAIL);

        printf("\033[1mline flag(P|B|A) ?\033[0m ");
        fflush(stdout);
        fgets(line, sizeof(line), stdin);
        if (line[0] == '\0')
            exit(FAIL);

        printf("\033[1mserver IP address(e.g. 200.100.3.191) ?\033[0m ");
        fflush(stdout);
        fgets(ip, sizeof(ip), stdin);
        if (ip[0] == '\0')
            exit(FAIL);
    }

    if (strlen(proc_id) != 10) {
        printf("ERROR:process ID[%s]\n", proc_id);
        exit(FAIL);
    }

    LtoU(line, 1);
    if (line[0] != 'P' && line[0] != 'B' && line[0] != 'A') {
        printf("ERROR:line flag[%s]\n", line);
        exit(FAIL);
    }

    /* initialize global variables and attach to daemon SHM (INFO)  */
    Init_Mana(argc, argv);

    sprintf(sub, "%-2.2s", proc_id);
    LtoU(sub, 2);
    dk = sub[1] - 'A';

    if (INFO(dk).process_id[0] == 0) {
        printf("%s daemon not registered !!!\n", sub);
        exit(FAIL);
    }

    if (inet_pton(AF_INET, ip, &tmp_ia) != 1) {
        puts("malformed IP address");
        exit(FAIL);
    }

    if (line[0] == 'P')
        l = 0;
    else if (line[0] == 'B')
        l = 1;

    for (pk = 0; pk < DAEMON(dk).p_count; pk ++) {
        if (memcmp(PROC(dk,pk).process_id, proc_id, 10) == 0) {
            if (PROC(dk,pk).type != TY_TRS2) {
                puts("TCP2 process only !!!");
                exit(FAIL);
            }

            if (line[0] == 'A') {
                printf("ip changed [%s: P=%d.%d.%d.%d, B=%d.%d.%d.%d ->",
                        PROC(dk,pk).process_id, TCP2_IP1(dk,pk,0),
                        TCP2_IP2(dk,pk,0), TCP2_IP3(dk,pk,0), TCP2_IP4(dk,pk,0),
                        TCP2_IP1(dk,pk,1), TCP2_IP2(dk,pk,1), TCP2_IP3(dk,pk,1),
                        TCP2_IP4(dk,pk,1));

                memcpy(&TCP2_IP1(dk,pk,0), &tmp_ia, sizeof (tmp_ia));
                memcpy(&TCP2_IP1(dk,pk,1), &tmp_ia, sizeof (tmp_ia));

                printf(" P=%d.%d.%d.%d, B=%d.%d.%d.%d]\n",
                        TCP2_IP1(dk,pk,0), TCP2_IP2(dk,pk,0), TCP2_IP3(dk,pk,0),
                        TCP2_IP4(dk,pk,0), TCP2_IP1(dk,pk,1), TCP2_IP2(dk,pk,1),
                        TCP2_IP3(dk,pk,1), TCP2_IP4(dk,pk,1));
            }
            else {
                printf("ip changed [%s: %c=%d.%d.%d.%d",
                        PROC(dk,pk).process_id, line[0], TCP2_IP1(dk,pk,l),
                        TCP2_IP2(dk,pk,l), TCP2_IP3(dk,pk,l), TCP2_IP4(dk,pk,l));

                memcpy(&TCP2_IP1(dk,pk,l), &tmp_ia, sizeof (tmp_ia));

                printf(" %c=%d.%d.%d.%d]\n",
                        line[0], TCP2_IP1(dk,pk,l), TCP2_IP2(dk,pk,l),
                        TCP2_IP3(dk,pk,l), TCP2_IP4(dk,pk,l));
            }

            break;
        }

        if (pk == DAEMON(dk).p_count - 1) {
            printf("process not registered[%s]\n", proc_id);
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

    Log(USR_OK, "[%s: %s, %s, %s]", buf, proc_id, line, ip);

    exit(OK);
}   /* End of main ()   */

/*************************************************************************
    End of Program (px_sett2ip.c)
*************************************************************************/
