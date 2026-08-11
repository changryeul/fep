#include "mds2.h"

static 	int words2xmltag(struct xmltag *);
static  int strip(char *);

static	char	args_b[32][512+1];
static	int	args_n, args_l;

int getxmlcfg(const char *xmlpath, struct xmltag *xmltag);
static int words2xmltag(struct xmltag *xmltag);
static int strip(char *str);
void getargv(struct xmltag *xml, char *name, int *pval);
void getargs(struct xmltag *xml, char *name, char *pstr);
void getargx(struct xmltag *xml, char *name, int *v, int *n, int max);

//
// getxmlcfg()
//
int getxmlcfg(const char *xmlpath, struct xmltag *xmltag)
{
	char	line_b[1024];
	FILE	*cFile;
	int	dodo, where, minus = 0;
	int	tags_n = 0;
	struct	xmltag	xmlchk, parent;
	int	ii;

	cFile = fopen(xmlpath, "r");
	if (cFile == NULL)
		return(0);
	tags_n = 0;
	where = 0;
	dodo = 0;
	while ((fgets(line_b, sizeof(line_b), cFile)) == line_b)
	{
		for (ii = 0; ii < strlen(line_b); ii++)
		{
			if (!(line_b[ii] & 0x80) && line_b[ii] < ' ')
				line_b[ii] = ' ';
			switch (where)
			{
			case 0:
				if (line_b[ii] == '<')
					where = 1;
				args_n = 0;
				args_l = 0;
				break;
			case 1: // <?
				switch (line_b[ii])
				{
				case ' ':	      break;
				case '!': where = 20; break;
				default:  
					args_b[args_n][args_l++] = line_b[ii];
					args_b[args_n][args_l]   = '\0';
					where = 2;
					break;
				}
				break;
			case 2: // <x?
				switch (line_b[ii])
				{
				case ' ': 
					if (args_l <= 0)
						break;
					args_n++; args_l = 0;
					break;
				case '\'':
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
					args_b[args_n][args_l++] = line_b[ii];
					args_b[args_n][args_l] = '\0';
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
						args_n++; 
						args_l = 0;
					}
					if (args_n <= 0)
						break;
					switch (dodo)
					{
					case 0:
						if (strcasecmp(args_b[0], "config") == 0)
							dodo = 1;
						continue;
					case 1: 
						if (strcasecmp(args_b[0], "/config") == 0)
						{
							dodo = 0;
							continue;
						}
						break;
					}

					if (words2xmltag(&xmlchk) != 0)
						break;
					memcpy(&xmltag[tags_n], &xmlchk, sizeof(struct xmltag));
					memcpy(&parent, &xmlchk, sizeof(struct xmltag));
					tags_n++;
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
				where = 0;
				if (!dodo)
					break;
				if (line_b[ii] != '>')
					break;
				if (words2xmltag(&xmlchk) != 0)
					break;
				memcpy(&xmltag[tags_n], &xmlchk, sizeof(struct xmltag));
				xmltag[tags_n].eotf = 1;
				tags_n++;
				break;
			case 4: // '?
				switch (line_b[ii])
				{
				case '\'':
					where = 2;
				default:
					if (args_l < 512)
					{
						args_b[args_n][args_l++] = line_b[ii];
						args_b[args_n][args_l] = '\0';
					}
					break;
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
				switch (line_b[ii])
				{
				case '-': minus++; break;
				case '>': if (minus >= 2)
					  {
						where = 0;
						minus = 0;
						break;
					  }
				default:  minus = 0;
					  break;
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

static int words2xmltag(struct xmltag *xmltag)
{
	int	flag, ndef;
	int	ii;

	if (args_n <= 0)
		return(-1);
	memset(xmltag, 0, sizeof(struct xmltag));
	strcpy(xmltag->tags, args_b[0]);
	if (args_b[0][0] == '/')
		xmltag->eotf = 2;
	for (ii = 1, flag = 0, ndef = 0; ii < args_n && ndef < MAX_DEFINITION; ii++)
	{
		strip(args_b[ii]);
		switch (flag)
		{
		case 0:
			if (strcmp(args_b[ii], "=") == 0)
				return(-1);
			flag = 1;
			strcpy(xmltag->defs[ndef].name, args_b[ii]);
			break;
		case 1: if (strcmp(args_b[ii], "=") == 0)
			{
				flag = 2;
				break;
			}
			ndef++;
			strcpy(xmltag->defs[ndef].name, args_b[ii]);
			flag = 0;
			break;
		case 2: if (strcmp(args_b[ii], "=") == 0)
				return(-1);
			strcpy(xmltag->defs[ndef].vals, args_b[ii]);
//printf("%s : %d[%s/%s]\n", __func__, ndef, xmltag->defs[ndef].name, xmltag->defs[ndef].vals);			
			ndef++;
			flag = 0;
			break;
		}
	}
	xmltag->many = ndef;
	return(0);
}

static int strip(char *str)
{
	char	tmpb[512];
	int	ii, jj;

	for (ii = 0, jj = 0; ii < strlen(str); ii++)
	{
		if (str[ii] == '"' || str[ii] == '\'')
			continue;
		tmpb[jj++] = str[ii];
	}
	tmpb[jj] = '\0';
	strcpy(str, tmpb);
	return(0);
}

/**
 * for parsing xml configuration string
 */
void getargv(struct xmltag *xml, char *name, int *pval)
{
	int	ii;

	*pval = 0;
	for (ii = 0; ii < xml->many; ii++)
	{
		if (strcasecmp(xml->defs[ii].name, name) == 0)
		{
			xml->defs[ii].name[0] = '\0';
			*pval = atoi(xml->defs[ii].vals);
			return;
		}
	}
}

void getargs(struct xmltag *xml, char *name, char *pstr)
{
	int	ii;

	*pstr = '\0';
	for (ii = 0; ii < xml->many; ii++)
	{
		if (strcasecmp(xml->defs[ii].name, name) == 0)
		{
			xml->defs[ii].name[0] = '\0';
			strcpy(pstr, xml->defs[ii].vals);
			return;
		}
	}
}

void getargx(struct xmltag *xml, char *name, int *v, int *n, int max)
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
		
	xml->defs[ii].name[0] = '\0';
	strcpy(argstr, xml->defs[ii].vals);
	while ((lptr = strstr(argstr, ";")) != NULL)
		*lptr = ' ';
	while ((lptr = strstr(argstr, ",")) != NULL)
		*lptr = ' ';
	for (ii = 0; ii < 16; ii++)
		v_args[ii][0] = '\0';
	sscanf(argstr, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
		v_args[0], v_args[1], v_args[2], v_args[3], v_args[4],
		v_args[5], v_args[6], v_args[7], v_args[8], v_args[9],
		v_args[10],v_args[11],v_args[12],v_args[13],v_args[14]);
	for (ii = 0, nn = 0; ii < 15 && nn < max && strlen(v_args[ii]) > 0; ii++)
		v[nn++] = atoi(v_args[ii]);
	if (n != NULL)
		*n = nn;
}
