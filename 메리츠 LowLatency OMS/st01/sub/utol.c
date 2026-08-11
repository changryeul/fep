/*------------------------------------------------------------------------
#	Module	: convert uppercase to lowercase 
#	File	: utol.c
------------------------------------------------------------------------*/

/*************************************************************************
	Function		: . convert uppercase to lowercase
	Parameters IN	: . p_utolbuf	: string to convert
					  . p_len		: data length
	Parameters OUT	: . p_utolbuf	: string converted
	Return Code		: . char * (string converted)
*************************************************************************/
/*----------------------------------------------------------------------*/
char	*UtoL (char *p_utolbuf, int p_len)
/*----------------------------------------------------------------------*/
{
	int		i;

	for (i = 0; i < p_len; i ++) 
		*(p_utolbuf+i) = (char)tolower (*(unsigned char *)(p_utolbuf+i));

	return (p_utolbuf);
}	/* End of UtoL ()	*/

/*************************************************************************
	End of Program (utol.c)
*************************************************************************/
