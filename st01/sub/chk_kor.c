/*------------------------------------------------------------------------
#   Module  : check korean character
#   File    : chk_kor.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Header Files
------------------------------------------------------------------------*/
#include    "fep_sub.h"

/*************************************************************************
    Function        : . check if truncating at idx splits a multi-byte char
    Prameters IN    : . str : string to check
                      . idx : check position (truncation boundary)
    Parameters OUT  : .
    Return Code     : . int
                            -1: idx is inside a multi-byte char (truncation unsafe)
                             0: idx is ASCII (truncation safe)
                             1: idx is start of a new multi-byte char (truncation safe before it)
    Note            : . supports both EUC-KR (2-byte) and UTF-8 (3-byte Korean)
*************************************************************************/
/*----------------------------------------------------------------------*/
int     Chk_Korean(char *str, int idx)
/*----------------------------------------------------------------------*/
{
    unsigned char ch;

    if (idx < 0)
        return (0);

    ch = (unsigned char)*(str+idx);

    /* ASCII byte — safe boundary */
    if (!(ch & 0x80))
        return (0);

    /* UTF-8 continuation byte (10xxxxxx) — inside multi-byte char */
    if ((ch & 0xC0) == 0x80)
        return (-1);

    /* UTF-8 leading byte or EUC-KR first byte — start of new char */
    return (1);
}   /* Chk_Korean ()    */

/*************************************************************************
    End of Program (chk_kor.c)
*************************************************************************/
