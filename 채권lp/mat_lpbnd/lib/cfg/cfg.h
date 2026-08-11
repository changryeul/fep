#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "dll.h"

#ifndef	CFG_H
#define	CFG_H

#define		CFG_FILE_BUF_SZ		8192
#define		CFG_WORD_BUF_SZ		65535	/* 20220914 통신 테스트를 위해서 512에서 변경 - input 임시 buffer 이므로 커도 상관 없음 */
										/* input이 끝나면 malloc한 pointer로 copy */
#define		CFG_MAX_STACK		512

#define		CFG_TYPE_VALUE		1
#define		CFG_TYPE_LIST		2
#define		CFG_TYPE_DLL		10

#define		CFG_MAX(x,y)		((x>y)?x:y)

typedef struct _cfg_element_
{
	char	*name;
	void	*value;
	int		type;
}	CFG_MEMBER;

typedef struct _cfg_
{
	char		*f_name;
	FILE		*fp;
	char		rec[ CFG_FILE_BUF_SZ];
	int			lin;
	int			col;


	char		word[ CFG_WORD_BUF_SZ];
	int			w_pos;

	char		**stack;
	int			s_cnt;

	CFG_MEMBER	*member;

	char		**key;
	int			key_cnt;
	CFG_MEMBER	*cur_mp;

}	CFG;

#endif /* CFG_H */

/***** Module : cfg.c *****/
CFG*        Cfg_Open( char *f_name);
int         Cfg_Close( CFG *cfg);
int         Cfg_MemberClose( CFG *cfg, CFG_MEMBER *ptr);
int         Cfg_GetLine( CFG *cfg, const char *call);
int         Cfg_GetFile( CFG *cfg, const char *call);
int         Cfg_PutFile( CFG *cfg, int c);
int         Cfg_PutChar( CFG *cfg, int c, const char *call);
int         Cfg_Push( CFG *cfg, const char *call);
char*       Cfg_Pop( CFG *cfg, const char *call);
int         Cfg_Load( CFG *cfg);
int         Cfg_Start( CFG *cfg);
int         Cfg_GetComment( CFG *cfg);
int         Cfg_GetName( CFG *cfg, CFG_MEMBER *arg_ptr);
int         Cfg_GetListName( CFG *cfg, CFG_MEMBER *arg_ptr);
int         Cfg_GetSection( CFG *cfg, CFG_MEMBER *arg_ptr);
int         Cfg_GetWinSection( CFG *cfg, CFG_MEMBER *arg_ptr);
int         Cfg_GetWinName( CFG *cfg, CFG_MEMBER *member);
int         Cfg_GetValue( CFG *cfg, CFG_MEMBER *member);
int         Cfg_GetString( CFG *cfg, int ch);
int         Cfg_GetChar( CFG *cfg);
int         Cfg_GetHexChar( CFG *cfg);
int         Cfg_GetBitChar( CFG *cfg);
int         Cfg_GetOctChar( CFG *cfg);
int         Cfg_GetDecChar( CFG *cfg);
int         Cfg_GetEnv( CFG *cfg);
void*       Cfg_GetFirstPtr( CFG *cfg);
void*       Cfg_GetNextPtr( CFG *cfg);
void*       Cfg_GetPtr( CFG *cfg, char *name);
void*       Cfg_GetListPtr( CFG *cfg);
int         Cfg_GetInt( CFG *cfg, char *name);
int         Cfg_PutEnv( CFG *cfg);
int         Cfg_Get( CFG *cfg, char *name, char *rec, int r_sz);
int         Cfg_Set( CFG *cfg, char *name);
int         Cfg_SetClear( CFG *cfg);
int         Cfg_SetMakeKeyword( CFG *cfg, char *name);
char*       Cfg_GetEnvPtr( CFG *cfg, char *name);
void*       Cfg_GetFirstNamePtr( CFG *cfg, char *name);
void*       Cfg_GetNextNamePtr( CFG *cfg, char *name);
CFG_MEMBER* Cfg_GetFirstMemberPtr( CFG *cfg, char *name);
CFG_MEMBER* Cfg_GetNextMemberPtr( CFG *cfg, char *name);
void*       Cfg_GetCurrNamePtr( CFG *cfg, char *name);
void*       Cfg_GetFirstValuePtr( CFG *cfg, char *name);
void*       Cfg_GetNextValuePtr( CFG *cfg, char *name);
void*       Cfg_GetCurrValuePtr( CFG *cfg, char *name);
CFG_MEMBER* Cfg_FindName( CFG *cfg, CFG_MEMBER *dp, char *name);
int         Cfg_Print( CFG *cfg);
int         Cfg_PrintMember( CFG *cfg, CFG_MEMBER *member, const char *name);

