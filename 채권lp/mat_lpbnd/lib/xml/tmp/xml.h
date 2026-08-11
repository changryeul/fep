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
	XML_Parser	parser;
	char		*data;			/* xml data buffer */
	int			len;			/* data buffer size */
}	XML_DATA;

#endif /* _XML_H_ */

/***** Module : xml.c *****/
XML_DATA*   Xml_Open();
int         Xml_AddName( XML_DATA *dp, const char *name, int sz);
int         Xml_AddValue( XML_DATA *dp, const char *value, int sz);
void        Xml_Start( void *userData, const XML_Char *name, const XML_Char **atts);
void        Xml_End( void *userData, const XML_Char *name);
void        Xml_Value( void *userData, const XML_Char *val, int len);
int         Xml_Close( XML_DATA *dp);
int         Xml_Print( XML_DATA *dp);

/***** Module : xml.c *****/
XML_DATA*   Xml_Open( const char *incode);
int         Xml_AddName( XML_DATA *dp, const char *name, int sz);
int         Xml_AddValue( XML_DATA *dp, const char *value, int sz);
void        Xml_Start( void *userData, const XML_Char *name, const XML_Char **atts);
void        Xml_End( void *userData, const XML_Char *name);
void        Xml_Value( void *userData, const XML_Char *val, int len);
int         Xml_Close( XML_DATA *dp);
int         Xml_Print( XML_DATA *dp);
int         Xml_Print2( XML_DATA *dp);
int         Xml_Proc( XML_DATA *dp, char *data, int sz, int f);
int         Xml_FindKey( XML_DATA *dp, char *dest, char *key);
int         XML_FindKey( XML_DATA *dp, char *key, char *dest);

