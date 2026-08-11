#define     _GLOBAL
/*------------------------------------------------------------------------
#   Module  : check sise TR count
#   File    : px_chktrcnt.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*------------------------------------------------------------------------
    Global Variables
------------------------------------------------------------------------*/
char    buf[KRX_DATA_BUFF_SIZE];

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    pri_1(int dk, int idx);
void    pri_2(int dk, int idx);

/*-----------------------------------------------------------------------*/
int main(int argc, char *argv[])
/*-----------------------------------------------------------------------*/
{
    char    d_time[16], line[90], hostname[12];
    int     rt, dk, idx;
    int     cnt = 0;
    int     urt = 0;
    int     mpt = 0;
    int     ust = 0;
    int     ddt = 0;

    /* Process Init */
    Init_Mana(argc, argv);

    Get_DateTime(d_time);
    gethostname(hostname, sizeof (hostname));
    printf("[%s: %s, %.4s/%.2s/%.2s %.2s:%.2s:%.2s]\n", argv[0], hostname,
            d_time, d_time+4, d_time+6, d_time+8, d_time+10, d_time+12);

    memset(line, '-', 85);

    if (argc != 2) {
        printf("----------------------------------------------------------\n");
        printf("Parameter is needed! ...\n");
        printf("Usage : %s pa\n", argv[0]);
        printf("----------------------------------------------------------\n");
        printf("< TR�� �Ǽ� ��ȸ >\n");
        printf("pa : ���ռ���, �����ɼ�, ������������, �ڽ�������\n");
        printf("----------------------------------------------------------\n");

        exit(1);
    }

    if (memcmp(argv[1], "pa", 2) == 0) {
        printf("%-60.60s\n", "######  �ü� TR�� �Ǽ� ��ȸ  ######");
        printf("[%-5.5s %7s %7s %7s %7s]", "TR", "  UR", "  DD", " MP", "US");
        printf("       [%-5.5s %7s %7s %7s %7s]\n", "TR", "  UR", "  DD", " MP", "US");

        printf("%-39.39s", line);
        printf("       %-39.39s\n", line);

        dk = 'a' - 'a';
        for (idx = 0; idx < INFO(dk).sisetr_count; idx ++) {
            if (SISETR(dk,idx).tr[0] == '\0')
                break;

            if (idx%2 == 0)
                pri_1(dk, idx);
            else
                pri_2(dk, idx);

            urt += SISETR(dk, idx).ur_count;
            ddt += SISETR(dk, idx).dd_count;
            mpt += SISETR(dk, idx).mp_count;
            ust += SISETR(dk, idx).us_count;
        }

        printf("\n");
        printf("%-86.86s\n", line);
        printf("[RECV:%d,   DD:%d,   MP:%d   US:%d]\n", urt, ddt, mpt, ust);
        printf("%-86.86s\n", line);

        exit(1);
    }
    else {
        printf("input parameter error [para 2 - %s] ...!\n", argv[1]);
        exit(1);
    }

    exit(OK);
} /* end of main */

/*-----------------------------------------------------------------------*/
void pri_1(int dk, int idx)
/*-----------------------------------------------------------------------*/
{
    printf("\n[%-5.5s %7d,%7d,%7d,%7d]",
            SISETR(dk, idx).tr,
            SISETR(dk, idx).ur_count,
            SISETR(dk, idx).dd_count,
            SISETR(dk, idx).mp_count,
            SISETR(dk, idx).us_count);
}

/*-----------------------------------------------------------------------*/
void pri_2(int dk, int idx)
/*-----------------------------------------------------------------------*/
{
    printf("       [%-5.5s %7d,%7d,%7d,%7d]",
            SISETR(dk, idx).tr,
            SISETR(dk, idx).ur_count,
            SISETR(dk, idx).dd_count,
            SISETR(dk, idx).mp_count,
            SISETR(dk, idx).us_count);
}

/**************************************************************************
    end of program(px_chktrcnt.c)
**************************************************************************/
