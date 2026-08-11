/* This is simple demonstration of how to use expat. This program
   reads an XML document from standard input and writes a line with
   the name of each element to standard output indenting child
   elements by one tab stop more than their parent element.
   It must be used with Expat compiled for UTF-8 output.
*/
#include <stdio.h>
#include "log.h"
#include "expat.h"

#if defined(__amigaos__) && defined(__USE_INLINE__)
#include <proto/expat.h>
#endif

#ifdef XML_LARGE_SIZE
#if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#define XML_FMT_INT_MOD "I64"
#else
#define XML_FMT_INT_MOD "ll"
#endif
#else
#define XML_FMT_INT_MOD "l"
#endif

typedef struct _name_book_
{
	int		depth;
	char	name[ 0xff];
}	NAME_BOOK;


static void XMLCALL startElement(void *userData, const char *name, const char **atts)
{
  int 		i;
  int 		*depthPtr = (int *)userData;
  NAME_BOOK	*np = ( NAME_BOOK *)userData;
  char		*anp;
  char		*avp;

	LogDel( "startElement(void *userData=[%s], const char *name=[%s], const char **atts=[%p])", userData, name, atts);

	for( i = 0; i < *depthPtr; i++)
	{
		putchar('\t');
	}

	LogDel( "np->depth     = [%d]", np->depth);
	LogDel( "np->name      = [%s]", np->name);
	LogDump( atts, 100, "stts[0]=[%p]", atts[ 0]);
	for( i = 0; atts[i]; i +=2)
	{
		anp = atts[ i];
		avp = atts[ i+1];

		LogDel( "anp=[%s] avp=[%s]", anp, avp);


		LogDel( "atts [%s]=[%s]", atts[i], atts[i+1]);
	}

  puts(name);
  *depthPtr += 1;
}

static void XMLCALL endElement(void *userData, const char *name)
{
  int *depthPtr = (int *)userData;
	LogDel( "endElement(void *userData=[%s], const char *name=[%s])", userData, name);
  *depthPtr -= 1;
}


int sample(int argc, char *argv[])
{
  char 			buf[BUFSIZ];
  XML_Parser 	parser = XML_ParserCreate(NULL);
  int 			done, len;
  int 			depth = 0;
  FILE			*fp;


  XML_SetUserData(parser, &depth);
  XML_SetElementHandler(parser, startElement, endElement);

  fp = fopen( "main.dat", "r");

  do 
  {
    len = (int)fread(buf, 1, sizeof(buf), fp);
    done = len < sizeof(buf);
    if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) 
	{
      fprintf(stderr,
              "%s at line %" XML_FMT_INT_MOD "u\n",
              XML_ErrorString(XML_GetErrorCode(parser)),
              XML_GetCurrentLineNumber(parser));
      return 1;
    }

  } while( len);

  XML_ParserFree(parser);

  return 0;
}

