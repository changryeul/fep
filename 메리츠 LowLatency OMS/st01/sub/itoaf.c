/*------------------------------------------------------------------------
#	Module	: convert integer to string
#	File	: itoaf.c
------------------------------------------------------------------------*/

/*************************************************************************
	Function		: . convert integer to string
	Parameters IN	: . p_int	: integer
					  . p_len	: length of the string
	Parameters OUT	: . p_ascii	: string converted
	Return Code		: . char * (string converted)
*************************************************************************/
#include        "fep_sub.h"

/*----------------------------------------------------------------------*/
char	*ItoAf (int p_int, char *p_ascii, int p_len)
/*----------------------------------------------------------------------*/
{
	memset (p_ascii, '0', p_len);

	for (p_len --; p_len >= 0; p_len --)
	{
		*(p_ascii+p_len) = p_int % 10 + '0';
		p_int /= 10;
	}

	return (p_ascii);
}	/* End of ItoAf ()	*/

/*************************************************************************
	End of Program (itoaf.c)
*************************************************************************/
