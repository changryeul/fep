/*------------------------------------------------------------------------
#   Module  : check environment values
#   File    : check_env.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"

/*------------------------------------------------------------------------
    Function Prototypes
------------------------------------------------------------------------*/
void    Check_Environment(void);

/*************************************************************************
    Function        : . check environment values
    Parameters IN   : .
    Parameters OUT  : .
    Return Code     : . void
*************************************************************************/
/*----------------------------------------------------------------------*/
void    Check_Environment(void)
/*----------------------------------------------------------------------*/
{
    int     rt;

    rt = Get_Environment();

    if (rt < OK) {
        rt *= -1;

        switch (rt) {
            case    FEP_LOG:
                Log(PRO_FATAL, "invalid _FEP_LOG");
                break;
            case    FEP_DAT:
                Log(PRO_FATAL, "invalid _FEP_DAT");
                break;
            case    FEP_BIN:
                Log(PRO_FATAL, "invalid _FEP_BIN");
                break;
            case    FEP_TMP:
                Log(PRO_FATAL, "invalid _FEP_TMP");
                break;
            case    FEP_CFG:
                Log(PRO_FATAL, "invalid _FEP_CFG");
                break;
            case    FEP_SHL:
                Log(PRO_FATAL, "invalid _FEP_SHL");
                break;
            case    FEP_FIFO:
                Log(PRO_FATAL, "invalid _FEP_FIFO");
                break;
            default:
                Log(PRO_FATAL, "cannot get environment values[%d]", rt);
                break;
        }
        exit(FAIL);
    }

    return;
}   /* End of Check_Environment ()  */

/*************************************************************************
    End of Program (check_env.c)
*************************************************************************/
