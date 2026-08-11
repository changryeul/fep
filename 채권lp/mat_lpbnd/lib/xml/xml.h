#include <stdio.h>
#include <expat.h>

#ifndef _XML_H_
#define _XML_H_	1

#define XML_MAX_DEPTH		256
#define XML_MAX_SIZE		65535

typedef struct _xml_stack_
{
	char	*key;				/* xml key */
	int		depth;				/* xml depth */
	char	*name;				/* xml name */
	int		nsz;				/* xml name size */
	char	*value;				/* xml value */
	int		vsz;				/* xml value size */
}	XML_MEMBER;

typedef struct _xml_data_
{
	int			cnt;			/* member count */
	XML_MEMBER	**member;		/* member pointer */
	int			depth;
	char		**key;			/* key name */

	char		t1[ 8192];
	XML_Parser	parser;
	char		t2[ 8192];

	char		*data;			/* xml data buffer */
	char		*remain;		/* xml end & remain data */
	int			end;			/* parser end flag */
	int			len;			/* xml data buffer size */
	int			remain_len;		/* remain data size */
	int			line;			/* end point at line */
	int			column;			/* end point at column */
	int			pos;			/* end position */
}	XML_DATA;

#endif /* _XML_H_ */

/***** Module : xml.c *****/
XML_DATA*   Xml_Open( const char *incode);
int         Xml_AddName( XML_DATA *dp, const char *name, int sz);
int         Xml_AddValue( XML_DATA *dp, const char *value, int sz);
int         Xml_AddTrimValue( XML_DATA *dp, const char *value, int sz);
void        Xml_Start( void *userData, const XML_Char *name, const XML_Char **atts);
void        Xml_End( void *userData, const XML_Char *name);
void        Xml_Value( void *userData, const XML_Char *val, int len);
int         Xml_Close( XML_DATA *dp);
int         Xml_Print( XML_DATA *dp);
int         Xml_Print2( XML_DATA *dp);
int         Xml_Proc( XML_DATA *dp, char *data, int sz, int f);
int         Xml_StopProc( XML_DATA *dp);
int         Xml_FindKey( XML_DATA *dp, char *dest, char *key);
int         XML_FindKey( XML_DATA *dp, char *key, char *dest);
int         Xml_GetRemainLen( XML_DATA *dp);
int         Xml_GetRemain( XML_DATA *dp, char *data, int len);
int         Xml_GetData( XML_DATA *dp, char *data, int sz);

