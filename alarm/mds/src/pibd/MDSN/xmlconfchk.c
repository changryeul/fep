#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "mds2.h"

static 	int words2xmltag_t(struct xmltag *);
int getxmlcfg_t(const char *xmlpath, struct xmltag *xmltag);
static  int strip(char *);
static void getargv_t(struct xmltag *xml, char *name, int *pval);
static void getargs_t(struct xmltag *xml, char *name, char *pstr);
static void getargx_t(struct xmltag *xml, char *name, int *v, int *n, int max);

static	char	args_b[32][512+1];
static	int	args_n, args_l;

//
// getxmlcfg()
//
int getxmlcfg_t(const char *xmlpath, struct xmltag *xmlt)
{
	char	line_b[1024];
	FILE	*cFile;
	int	dodo = 0, where = 0, minus = 0;
	int	tags_n = 0;
	struct	xmltag	xmlchk;
	int	ii;

	cFile = fopen(xmlpath, "r");
	if (cFile == NULL)
		return(0);
	
	while ((fgets(line_b, sizeof(line_b), cFile)) != NULL)
	{
		for (ii = 0; ii < (int)strlen(line_b); ii++)
		{
			if (!(line_b[ii] & 0x80) && line_b[ii] < ' ')
				line_b[ii] = ' ';
			
			switch (where)
			{
			case 0:
				if (line_b[ii] == '<')
				{
					where = 1;
					args_n = 0;
					args_l = 0;
				}
				break;
			case 1: // <?
				switch (line_b[ii])
				{
				case ' ':	      break;
				case '!': where = 20; break;
				default:
					if (args_l < 512)
					{
						args_b[args_n][args_l++] = line_b[ii];
						args_b[args_n][args_l]   = '\0';
					}
					where = 2;
					break;
				}
				break;
			case 2: // <x?
				switch (line_b[ii])
				{
				case ' ': 
					if (args_l > 0)
					{
						args_n++; args_l = 0;
					}
					break;
				case '\'':
				case '"' :
					if (args_l < 512)
					{
						args_b[args_n][args_l++] = line_b[ii];
						args_b[args_n][args_l] = '\0';
					}
					where = 4;
					break;
				case '=':
					if (args_l > 0)
					{
						args_n++; args_l = 0;
					}
					strncpy(args_b[args_n], "=", sizeof(args_b[args_n])-1);
					args_b[args_n][1] = '\0';
					args_n++; args_l = 0;
					break;
				case '/':
					if (args_l > 0)
					{
						args_n++; args_l = 0;
					}
					where = 3;
					break;
				case '>':
					where = 0;
					if (args_l > 0)
					{
						args_n++; args_l = 0;
					}
					if (args_n <= 0)
						break;

					if (dodo == 0 && strcasecmp(args_b[0], "config") == 0)
					{
						dodo = 1;
						continue;
					}
					else if (dodo == 1 && strcasecmp(args_b[0], "/config") == 0)
					{
						dodo = 0;
						continue;
					}

					if (dodo)
					{
						if (words2xmltag_t(&xmlchk) == 0)
						{

							memcpy(&xmlt[tags_n], &xmlchk, sizeof(struct xmltag));
							printf("<%s:%d> xmlt[%d].tags[%s] xmlt[%d].many[%d]\n",__func__,__LINE__,tags_n,xmlt[tags_n].tags,tags_n,xmlt[tags_n].many);
							tags_n++;
						}

					}
					break;
				default:
					if (args_l < 512)
					{
						args_b[args_n][args_l++] = line_b[ii];
						args_b[args_n][args_l] = '\0';
					}
					break;
				}
				break;

			case 3:  // /> ?
				if (line_b[ii] == '>')
				{
					where = 0;
					if (dodo && words2xmltag_t(&xmlchk) == 0)
					{
						//printf("<%s:%d> xmlchk.defs[0].name[%s] xmlchk.defs[0].vals[%s]\n", __func__, __LINE__,xmlchk.defs[0].name, xmlchk.defs[0].vals);
						//printf("<%s:%d> xmlchk.defs[1].name[%s] xmlchk.defs[1].vals[%s]\n", __func__, __LINE__,xmlchk.defs[1].name, xmlchk.defs[1].vals);
						//printf("<%s:%d> xmlchk.defs[2].name[%s] xmlchk.defs[2].vals[%s]\n", __func__, __LINE__,xmlchk.defs[2].name, xmlchk.defs[2].vals);
						//printf("<%s:%d> xmlchk.defs[3].name[%s] xmlchk.defs[3].vals[%s]\n", __func__, __LINE__,xmlchk.defs[3].name, xmlchk.defs[3].vals);
						//printf("<%s:%d> xmlchk.defs[4].name[%s] xmlchk.defs[4].vals[%s]\n", __func__, __LINE__,xmlchk.defs[4].name, xmlchk.defs[4].vals);
						printf("copy -> xmlt[%d] addr=%p xmlchk=%p\n", tags_n, &xmlt[tags_n], &xmlchk);	
						memcpy(&xmlt[tags_n], &xmlchk, sizeof(struct xmltag));
						printf("<%s:%d> xmlt[%d].tags[%s] xmlt[%d].many[%d]\n", __func__, __LINE__,
							tags_n, xmlt[tags_n].tags, tags_n, xmlt[tags_n].many );
						printf("<%s:%d> xmlt[%d].defs[0].name[%s] xmlt[%d].defs[0].vals[%s]\n", __func__, __LINE__,
							tags_n, xmlt[tags_n].defs[0].name, tags_n, xmlt[tags_n].defs[0].vals);
						printf("<%s:%d> xmlt[%d].defs[1].name[%s] xmlt[%d].defs[1].vals[%s]\n", __func__, __LINE__,
						    tags_n, xmlt[tags_n].defs[1].name, tags_n, xmlt[tags_n].defs[1].vals);
						printf("<%s:%d> xmlt[%d].defs[2].name[%s] xmlt[%d].defs[2].vals[%s]\n", __func__, __LINE__,
						    tags_n, xmlt[tags_n].defs[2].name, tags_n, xmlt[tags_n].defs[2].vals);
						printf("<%s:%d> xmlt[%d].defs[3].name[%s] xmlt[%d].defs[3].vals[%s]\n", __func__, __LINE__,
							tags_n, xmlt[tags_n].defs[3].name, tags_n, xmlt[tags_n].defs[3].vals);
						printf("<%s:%d> xmlt[%d].defs[4].name[%s] xmlt[%d].defs[4].vals[%s]\n", __func__, __LINE__,
						    tags_n, xmlt[tags_n].defs[4].name, tags_n, xmlt[tags_n].defs[4].vals);
						xmlt[tags_n].eotf = 1;
						tags_n++;
					}
				}
				break;

			case 4: // '?
				if (line_b[ii] == '\'' || line_b[ii] == '"')
				{
					where = 2;
				}
				else if (args_l < 512)
				{
					args_b[args_n][args_l++] = line_b[ii];
					args_b[args_n][args_l] = '\0';
				}
				break;

			case 20: // <!
				if (line_b[ii] == '-')
					where = 21;
				else
					where = 90;
				break;

			case 21: // <!-
				if (line_b[ii] == '-')
				{
					where = 30;
					minus = 0;
				}
				else
					where = 90;
				break;
	
			case 30: // wait -->
				if (line_b[ii] == '-')
				{
					minus++;
				}
				else if (line_b[ii] == '>' && minus >= 2)
				{
					where = 0;
					minus = 0;
				}
				else
				{
					minus = 0;
				}
				break;

			case 90: // error wait >
			default:
				if (line_b[ii] == '>')
					where = 0;
				break;
			}
		}
	}

	fclose(cFile);
	return(tags_n);
}

static int words2xmltag_t(struct xmltag *xmlt)
{
	int	flag, ndef;
	int	ii;

	if (args_n <= 0)
		return(-1);

	memset(xmlt, 0, sizeof(struct xmltag));

	// Copy tag name
	strncpy(xmlt->tags, args_b[0], sizeof(xmlt->tags)-1);
	xmlt->tags[sizeof(xmlt->tags)-1] = '\0';

	// Detect closing tag
	if (xmlt->tags[0] == '/')
		xmlt->eotf = 2;
	else
		xmlt->eotf = 0;

	for (ii = 1, flag = 0, ndef = 0; ii < args_n && ndef < MAX_DEFINITION; ii++)
	{
		strip(args_b[ii]);
	
		switch (flag)
		{
		case 0:
			if (strcmp(args_b[ii], "=") == 0)
				return(-1);
			
			//printf("<%s:%d> args_b[%d][%s]\n", __func__, __LINE__, ii, args_b[ii]);	
			strncpy(xmlt->defs[ndef].name, args_b[ii], sizeof(xmlt->defs[ndef].name)-1);
			xmlt->defs[ndef].name[sizeof(xmlt->defs[ndef].name)-1] = '\0';
			//printf("<%s:%d> xmlt->defs[%d].name[%s] \n", __func__, __LINE__, ndef, xmlt->defs[ndef].name);	
			flag = 1;
			break;
		
		case 1: 
			if (strcmp(args_b[ii], "=") == 0)
			{
				flag = 2;
			}
			else
			{
				xmlt->defs[ndef].vals[0] = '\0';
				ndef++;
				//printf("<%s:%d> args_b[%d][%s]\n", __func__, __LINE__, ii, args_b[ii]);	
				strncpy(xmlt->defs[ndef].name, args_b[ii], sizeof(xmlt->defs[ndef].name)-1);
				xmlt->defs[ndef].name[sizeof(xmlt->defs[ndef].name)-1] = '\0';
				flag = 1;
			}
			break;

		case 2: 
			if (strcmp(args_b[ii], "=") == 0)
				return(-1);

			//printf("<%s:%d> args_b[%d][%s]\n", __func__, __LINE__, ii, args_b[ii]);	
			strncpy(xmlt->defs[ndef].vals, args_b[ii], sizeof(xmlt->defs[ndef].vals)-1);
			xmlt->defs[ndef].vals[sizeof(xmlt->defs[ndef].vals)-1] = '\0';
			//printf("<%s:%d> xmlt->defs[%d].name[%s] \n", __func__, __LINE__, ndef, xmlt->defs[ndef].name);	
			ndef++;
			flag = 0;
			break;
		}
	}

	if (flag == 1 && ndef < MAX_DEFINITION)
	{
		xmlt->defs[ndef].vals[0] = '\0';
		ndef++;
	}

	xmlt->many = ndef;
	return(0);
}

static int strip(char *str)
{
	char	tmpb[512];
	int	ii, jj;

	for (ii = 0, jj = 0; ii < (int)strlen(str); ii++)
	{
		if (str[ii] == '"' || str[ii] == '\'')
			continue;
		if (jj < (int)sizeof(tmpb) - 1)
			tmpb[jj++] = str[ii];
	}
	tmpb[jj] = '\0';
	strncpy(str, tmpb, 512 - 1);
	str[sizeof(tmpb)-1] = '\0';
	return(0);
}

/**
 * for parsing xml configuration string
 */
static void getargv_t(struct xmltag *xml, char *name, int *pval)
{
	int	ii;
	*pval = 0;

	for (ii = 0; ii < xml->many; ii++)
	{
		if (strcasecmp(xml->defs[ii].name, name) == 0)
		{
			*pval = atoi(xml->defs[ii].vals);
			return;
		}
	}
}

static void getargs_t(struct xmltag *xml, char *name, char *pstr)
{
	int	ii;
	*pstr = '\0';

	for (ii = 0; ii < xml->many; ii++)
	{
		if (strcasecmp(xml->defs[ii].name, name) == 0)
		{
			strncpy(pstr, xml->defs[ii].vals, 256);
			return;
		}
	}
}

static void getargx_t(struct xmltag *xml, char *name, int *v, int *n, int max)
{
	char	argstr[128];
	char	v_args[16][16];
	char	*lptr;
	int	ii, nn;

	*v = 0;	
	if (n != NULL)
		*n = 0;
	for (ii = 0; ii < xml->many; ii++)
	{
		if (strcasecmp(xml->defs[ii].name, name) == 0)
			break;
	}
	if (ii >= xml->many)
		return;
		
	strncpy(argstr, xml->defs[ii].vals, sizeof(argstr)-1);
	argstr[sizeof(argstr)-1] = '\0';

	while ((lptr = strstr(argstr, ";")) != NULL)
		*lptr = ' ';
	while ((lptr = strstr(argstr, ",")) != NULL)
		*lptr = ' ';

	for (ii = 0; ii < 16; ii++)
		v_args[ii][0] = '\0';

	sscanf(argstr, "%15s %15s %15s %15s %15s %15s %15s %15s %15s %15s %15s %15s %15s %15s %15s",
		v_args[0], v_args[1], v_args[2], v_args[3], v_args[4],
		v_args[5], v_args[6], v_args[7], v_args[8], v_args[9],
		v_args[10],v_args[11],v_args[12],v_args[13],v_args[14]);

	for (ii = 0, nn = 0; ii < 15 && nn < max && strlen(v_args[ii]) > 0; ii++)
		v[nn++] = atoi(v_args[ii]);
	if (n != NULL)
		*n = nn;
}

