/*------------------------------------------------------------------------
#	Module	: check korean character
#	File	: chk_kor.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"

/*************************************************************************
	Function		: . check korean/korean half, alpahnumeric
	Prameters IN	: . str	: string to check
					  . idx	: check position
	Parameters OUT	: .
	Return Code		: . int
							-1:korean start
							1:korean end, space (0x20) or uppercase (A ~ Z)
							0:alpahnumeric (0 ~ 9, a ~ z)
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Chk_Korean (char *str, int idx)
/*----------------------------------------------------------------------*/
{
	int		i, flag;
	char	ch;
	
	i = flag = 0;

	if (idx < 0) 
		return (flag);

	ch = *(str+idx);

	if (!(ch & 0x80))
		return (flag);

	while (i <= idx)
	{ 
		if (*str & 0x80)
			flag ^= 0x01;
		str ++;
		i ++;
	}

	if (flag)
		return (1);
	else
		return (-1);
}	/* Chk_Korean ()	*/

/*************************************************************************
	End of Program (chk_kor.c)
*************************************************************************/
