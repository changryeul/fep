/*------------------------------------------------------------------------
#	Module	: convert double to string
#	File	: dtoaf.c
------------------------------------------------------------------------*/

/*************************************************************************
	Function		: . convert double to string
	Parameters IN	: . p_double	: double data
					  . p_len		: length of the string converted
	Parameters OUT	: . p_ascii		: string converted
	Return Code		: . char * (string converted)
*************************************************************************/
#include        "fep_sub.h"

/*----------------------------------------------------------------------*/
char	*DtoAf (double p_double, char *p_ascii, int p_len)
/*----------------------------------------------------------------------*/
{
	char    fmt[10], buf[20];

	sprintf (fmt, "%%%03d.0f", p_len);
	sprintf (buf, fmt, p_double);
	memcpy (p_ascii, buf, p_len);

    return (p_ascii);
}	/* End of DtoAf ()	*/

/*************************************************************************
	End of Program (dtoaf.c)
*************************************************************************/
