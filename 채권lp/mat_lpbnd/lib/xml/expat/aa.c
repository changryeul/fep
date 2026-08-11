#pragma warning(disable:4996)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"

#define XML_STATIC      /* for static linking   */
char *_T( char *data);

#if defined (UNICODE)
#  define XML_UNICODE_WCHAR_T
#endif

#include "expat.h"

#if defined (UNICODE)
#  pragma comment(lib, "libexpatMTw.lib")
#else
#  pragma comment(lib, "libexpatMT.lib")
#endif

typedef struct NameBook
{
    int m_nSince;
    char m_szName[0xFF];
} NameBook;

static void XMLCALL StartElement(void *pUserData, const char *name, const char **atts)
{
    int i = 0;
    const char *pszAttrName = NULL;
    const char *pszAttrValue = NULL;
    NameBook *pNameBook = (NameBook *)pUserData;

    for (i = 0; atts[i]; i += 2) 
    {
        pszAttrName = atts[i];
        pszAttrValue = atts[i+1];

        if ( strcmp(pszAttrName, "since") == 0 && pszAttrValue )
            pNameBook->m_nSince = atoi(pszAttrValue);
    }
}

static void XMLCALL VisitData(void *pUserData, const char *s, int len)
{
    NameBook *pNameBook = (NameBook *)pUserData;
    
    if ( pNameBook && s && len > 0 )
    {
        memcpy(pNameBook->m_szName, s, len);
        pNameBook->m_szName[len] = 0;
    }
}

static void XMLCALL EndElement(void *pUserData, const char *name)
{
    NameBook *pNameBook = (NameBook *)pUserData;
}

int sample(int argc, char **argv)
{
    int bEof = 1;
    const char *pszXMLDoc = _T("<NameBook since=\"1997\"><name>John</name></NameBook>");
    XML_Parser parser = NULL;
    NameBook nameBook;
	char	data[ 65535];

	fgets( data, 65535, stdin);

    memset(&nameBook, 0x00, sizeof(NameBook));
    parser = XML_ParserCreate(NULL);

    XML_SetUserData(parser, (void *)&nameBook);
    XML_SetElementHandler(parser, StartElement, EndElement);
    XML_SetCharacterDataHandler(parser, VisitData);

    if (XML_Parse(parser, data, strlen(data), bEof) == XML_STATUS_ERROR) 
    {
        LogDbg(_T("%s at line %u\n"), XML_ErrorString(XML_GetErrorCode(parser)), 
                                        XML_GetCurrentLineNumber(parser));
        return 1;
    }

    LogDbg(_T("NameBook Since %d, Name : '%s'\n"), nameBook.m_nSince, nameBook.m_szName);
    XML_ParserFree(parser);

    return 0;
}


char *_T( char *data)
{
	int			sz;
	static char *p;

	sz = strlen( data);
	p = malloc( sz +1);
	memcpy( p, data, sz);
	p[sz] = 0;

	return p;
}

