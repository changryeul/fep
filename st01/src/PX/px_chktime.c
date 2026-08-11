/*------------------------------------------------------------------------
#   System  : TLFEP System
#   Module  : report order transaction time
#   File    : px_chktime.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_fepp.h"

/*----------------------------------------------------------------------*/
int     main(int argc, char *argv[])
/*----------------------------------------------------------------------*/
{
    int     tmp_time, min_time, max_time;
    double  min, max;
    char    path[128], buf[512];
    FILE    *fp;

    if ((argc != 2) || (argc == 2 && argv[1][0] == '?')) {
        printf("==========================================================\n");
        printf("Usage: %s <file> \n\n", argv[0]);
        printf("  e.g. 1) %s\n", argv[0]);
        printf("       2) %s tsgap0\n", argv[0]);
        printf("==========================================================\n");
        exit(FAIL);
    }

    tmp_time = 0;
    min_time = max_time = 0;
    min = max = 0;

    sprintf(path, "%s", &argv[1][0]);

    if ((fp = fopen(path, "r")) == NULL) {
        printf("\033[5mcannot open:[%s]\033[0m\n", path);
        exit(1);
    }

    while (fgets(buf, sizeof (buf), fp) != NULL) {
        tmp_time = AtoIf(buf+64, 6);

        /***
                printf ("tmp_time: [%d]\n", tmp_time);
        ***/

        if (min_time == 0 || tmp_time < min_time)
            min_time = tmp_time;

        if (tmp_time > max_time)
            max_time = tmp_time;

        memset(buf, 0, sizeof (buf));
    }

    printf("------------------------------------------------\n");
    /***
        if (min_time > 100000)
            printf ("min: [0.%d]\n", min_time);
        else
            printf ("min: [0.0%d]\n", min_time);

        if (max_time > 100000)
            printf ("max: [0.%d]\n", max_time);
        else
            printf ("max: [0.0%d]\n", max_time);
    ***/
    min = min_time / 1000000.0;
    printf("Min [%.06f]\n", min);
    max = max_time / 1000000.0;
    printf("Max [%.06f]\n", max);

    fclose(fp);

    exit(0);
}   /* End of main ()   */

/*************************************************************************
    End of Program (px_chktime.c)
*************************************************************************/
