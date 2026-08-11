#ifndef	_CONFIG_H
#define	_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define	MAX_DEFINITION	16

#define	C_INT		1
#define	C_STRING	2
#define	C_CHCK		9

struct	xmltag {
	char	tags[32];
	int	many;
	struct	{
		char	name[32];
		char	vals[80];
	} defs[MAX_DEFINITION];
	int	eotf;
};

int		getxmlcfg(const char *xmlpath, struct xmltag *);
void 	getargv(struct xmltag *, char *, int *);
void 	getargs(struct xmltag *, char *, char *);
void 	getargx(struct xmltag *xml, char *name, int *v, int *n, int max);

#ifdef __cplusplus
}
#endif

#endif
