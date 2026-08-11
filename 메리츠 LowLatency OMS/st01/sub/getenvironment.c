/*------------------------------------------------------------------------
#	Module	: get environment values
#	File	: getenvironment.c
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#include	"fep_sub.h"

/*------------------------------------------------------------------------
	Function Prototypes
------------------------------------------------------------------------*/
int		Get_Environment (void);

/*************************************************************************
	Function		: . get environment values and set global variables
	Parameters IN	: .
	Parameters OUT	: .
	Return Code		: . int
						success	: 0
						failure	: -1 --> _FEP_LOG
								  -2 --> _FEP_DAT
								  -3 --> _FEP_BIN
								  -4 --> _FEP_TMP
								  -5 --> _FEP_CFG
								  -6 --> _FEP_SHL
								  -7 --> _FEP_FIFO
	Global Data		: . char *_FEP_LOG
					  . char *_FEP_DAT
					  . char *_FEP_BIN
					  . char *_FEP_TMP
					  . char *_FEP_CFG
					  . char *_FEP_SHL
					  . char *_FEP_FIFO
*************************************************************************/
/*----------------------------------------------------------------------*/
int		Get_Environment (void)
/*----------------------------------------------------------------------*/
{
	char	buf[256];

	/* LOG home path	*/
	if ((_FEP_LOG = (char *)getenv ("_P_LOG")) == NULL)
		return (-1);

	/* DAT home path	*/
	if ((_FEP_DAT = (char *)getenv ("_P_DAT")) == NULL)
		return (-2);

	/* bin home path	*/
	if ((_FEP_BIN = (char *)getenv ("_P_BIN")) == NULL)
		return (-3);

	/* tmp home path	*/
	if ((_FEP_TMP = (char *)getenv ("_FEP_TMP")) == NULL)
		return (-4);

	/* cfg home path	*/
	if ((_FEP_CFG = (char *)getenv ("_P_CFG")) == NULL)
		return (-5);

	/* shl home path	*/
	if ((_FEP_SHL = (char *)getenv ("_P_SHL")) == NULL)
		return (-6);

	/* FIFO home path	*/
	if ((_FEP_FIFO = (char *)getenv ("_P_FIFO")) == NULL)
		return (-7);

	return (0);
}	/* End of Get_Environment ()	*/

/*************************************************************************
	End of Program (getenvironment.c)
*************************************************************************/
