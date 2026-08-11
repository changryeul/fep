#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "dll.h"
#include "cfg.h"
#include "win.h"

#ifndef MAP_H
#define	MAP_H	1

#define	MAP_MIN(x,y)				((x>y)?y:x)
#define	MAP_MAX(x,y)				((x>y)?x:y)

#define MAP_TYPE_MENU			0
#define	MAP_TYPE_SELECT			1				/* menu select */
#define MAP_TYPE_EDIT			2
#define MAP_TYPE_MAP			3				/* DLL title에 저장 1=map, 0-field */
#define	MAP_TYPE_COMPARE		4
#define	MAP_TYPE_POINTER		5				/* pointer type ex) *(int *)field->data */
#define	MAP_TYPE_TYPE			6				/* 0:char 1:int 2:float */

#define	MAP_TYPE_INT			'1'
#define	MAP_TYPE_FLOAT			'2'
#define	MAP_TYPE_LONG			'3'
#define	MAP_TYPE_LONG_LONG		'4'
#define	MAP_TYPE_DOUBLE			'5'

#define	MAP_ATTR_FORE			0
#define	MAP_ATTR_BACK			1
#define	MAP_ATTR_BRIGHT			2
#define	MAP_ATTR_REVERSE		3
#define	MAP_ATTR_BOLD			4
#define	MAP_ATTR_ITALIC			5
#define	MAP_ATTR_UNDERLINE		6
#define	MAP_ATTR_CONCOAL		7
#define	MAP_ATTR_DELETE			8
#define	MAP_ATTR_BLINK			9

#define MAP_ATTR_BLACK              0x30
#define MAP_ATTR_RED                0x31
#define MAP_ATTR_GREEN              0x32
#define MAP_ATTR_YELLOW             0x33
#define MAP_ATTR_BLUE               0x34
#define MAP_ATTR_MAGENTA            0x35
#define MAP_ATTR_CYAN               0x36
#define MAP_ATTR_WHITE              0x37
#define MAP_ATTR_NONE               0x38

#define	MAP_EDIT_ATTR			"70010000000"
#define MAP_MENU_ATTR			"70010000000"
#define MAP_NONE_ATTR			"00000000000"

#define	MAP_EDIT_MODE_INIT			"\1\0\0\0\0\0\0\0"	/* 일반 field - default */
#define	MAP_EDIT_MODE_EDIT			"\0\1\0\0\0\0\0\0"	/* 편집기 */
#define MAP_EDIT_MODE_INSERT		0				/* 0-insert, 1-update */
#define MAP_EDIT_MODE_POS			1				/* 0-first, 1-prev field edit pos */

typedef struct _map_type_t_
{
	char		menu;
	char		input;
	char		map;							/* only display at Map_DisplayMap */
	char		compare;
	char		pointer;						/* pointer type ex) *(int *)field->data */
	char		type;							/* 0:char 1:int 2:float */
}	MAP_TYPE_T;

typedef struct _map_
{
	char		*f_name;

	int			x;
	int			y;
	int			sz_x;
	int			sz_y;

	int			base_x;
	int			base_y;

	int			line_s;
	int			line_e;
	int			line_n;

	WIN			*win;

	struct _map_field_	*select;			/* select menu field pointer */

	char		**scr;
	int			s_cnt;

	DLL			*title;
	DLL			*field;
	DLL			*menu;

	int			timeout;					/* select timeout - micro second */
}	MAP;

typedef struct _map_field_
{
	int				id;
	char			*name;
	char			attr[ 32];
	char			type[ 32];
	int				x;
	int				y;
	int				vsz;
	int				sz;
	int				set;					/* data가 allocation 한 값이 아닌 user pointer 일때 1 - Map_SetDataPtr 에서 assign */
	char			*data;
	char			*format;
	int				update;
	int				(*func)( MAP *map, struct _map_field_ *field);
	void			*ptr;					/* func 에서 data에 변환할 data의 pointer - Map_SetField 에서 assign */
	void			*uptr[ 8];				/* user pointer - Map_SetUserPtr 에서 setting */
	char			buf[ 256];				/* user data field */
}	MAP_FIELD;

typedef struct _map_menu_
{
	int				type;
	int				key;				/* last input key */
	char			key_str[ 512];		/* find menu key string - 현재는 toupper( 1byte)만 비교 */
	int				key_pos;			/* key_str pos */
	MAP_FIELD		*select;
	DLL				*menu;
}	MAP_MENU;

typedef struct _map_edit_
{
	int				type;
	unsigned int	key;
	int				pos;
	int				x;
	int				y;
	int				x_sz;
	int				y_sz;
	char			*attr;
	char			mode[ 8];
	int				sz;								/* total size */
	char			buf[ 512];						/* edit data buffer */
	char			*ptr;							/* current field buffer */
	char			*rec;							/* total buffer */
	MAP_FIELD		*select;
	DLL				*edit;
}	MAP_EDIT;


extern MAP	*__Map;
#endif /* MAP_H */

/***** Module : map_edit.c *****/
MAP_EDIT*   Map_EditOpen( MAP *map, int edit_type, char *mode);             /* Editor open */
int         Map_EditClose( MAP *map, MAP_EDIT *edit);                       /* Editor close */
int         Map_EditPutData( MAP *map, MAP_EDIT *edit, char *rec, int sz);  /* put data to edit field */
int         Map_Edit( MAP *map, MAP_EDIT *edit);                            /* Edit process */
unsigned    Map_EditField( MAP *map, MAP_EDIT *edit, MAP_FIELD *field);     /* 필드이름으로 한개 필드의 data 입력 */
unsigned    Map_EditFieldString( MAP *map, MAP_EDIT *edit, MAP_FIELD *field);/* 필드이름으로 한개 필드의 data 입력 */
unsigned    Map_EditFieldNum( MAP *map, MAP_EDIT *edit, MAP_FIELD *field);  /* 필드이름으로 한개 필드의 data 입력 */
int         Map_EditNext( MAP *map, MAP_EDIT *edit);                        /* next field edit */
int         Map_EditPrev( MAP *map, MAP_EDIT *edit);                        /* prev field edit */
int         Map_EditHome( MAP *map, MAP_EDIT *edit);                        /* first field edit */
int         Map_EditEnd( MAP *map, MAP_EDIT *edit);                         /* last field edit */

/***** Module : map.c *****/
int         MapInit();                                                      /* 맵 파용하기 위한 터미널 초기화 */
int         MapEnd();                                                       /* 터미널 세팅을 끝냄 */
MAP*        Map_Open( char *f_name);                                        /* 맵 파일을 열어 출력을위한 초기화 */
int         Map_Delete( MAP *map);                                          /* 맵 파일 */
int         Map_Close( MAP *map);                                           /* 맵 파일 */
int         Map_SetBuffer( MAP *map, char *buffer, int sz);                 /* 맵 파일 */
int         Map_GetBuffer( MAP *map);                                       /* 맵 파일 */
int         Map_DisplayMap( MAP *map);                                      /* 맵 파일 */
int         Map_DisplayField( MAP *map);                                    /* 맵 파일 */
int         Map_DisplayFieldData( MAP *map, MAP_FIELD *field);              /* 한개의 필드 출력 */
int         Map_DisplayFieldDataA( MAP *map, MAP_FIELD *field, char *attr); /* 한개의 필드를 인수의 attribute로  출력 */
int         Map_DisplayFieldDataNA( MAP *map, MAP_FIELD *field, int d_sz, char *attr);/* 한개의 필드를 인수의 attribute로  출력 */
int         Map_DisplayUpdate( MAP *map);                                   /* 맵 파일 */
int         Map_Load( MAP *map);                                            /* 맵 파일 */
int         Map_GetSize( MAP *map, char *line);                             /* 맵 파일 */
int         Map_GetScreen( MAP *map, char *line);                           /* 맵 파일 */
int         Map_GetBase( MAP *map, char *line);                             /* 맵 파일 */
int         Map_GetLine( MAP *map, char *line);                             /* 맵 파일 */
int         Map_GetField( MAP *map, char *line);                            /* 맵 파일 */
int         Map_LineField( MAP *map, MAP_FIELD *field);                     /* 맵 파일 */
int         Map_AddField( MAP *map, int id, char *name, int x, int y, int vsz, int sz, char *attr, char *type, char *data);/* 맵 파일 */
int         Map_SetDataPtr( MAP *map, char *name, void *ptr);               /* 맵 파일 */
int         Map_SetUserPtr( MAP *map, char *name, int num, void *ptr);      /* field->uptr을 setting - Map_GetUserPtr 에서 반환 */
void*       Map_GetUserPtr( MAP *map, int num, MAP_FIELD *field);           /* field->uptr을 반환 - Map_SetUserPtr 에서 set */
int         Map_PrintName( MAP *map, char *name, char *format, ...);        /* 맵 파일 */
int         Map_PrintNameA( MAP *map, char *name, char *attr, char *format, ...);/* 맵 파일 */
int         Map_SetField( MAP *map, char *name, int ( *func)( MAP *map, MAP_FIELD *field), void *ptr);/* 맵 파일 */
int         Map_PrintField( MAP *map, MAP_FIELD *field, char *format, ...); /* 맵 파일 */
int         Map_PrintFieldA( MAP *map, MAP_FIELD *field, char *attr, char *format, ...);/* 맵 파일 */
MAP_FIELD*  Map_GetFieldPtr( MAP *map, char *name);                         /* find 필드 */
int         Map_Trim( MAP *map, char *str);                                 /* 맵 파일 */
int         Map_GotoXY( MAP *map, int x, int y);                            /* 맵 파일 */
int         Map_GotoEnd( MAP *map);                                         /* 맵 파일 */
int         Map_CursorOff( MAP *map);                                       /* 맵 파일 */
int         Map_CursorOn( MAP *map);                                        /* 맵 파일 */
int         Map_PrintMap( MAP *map);                                        /* 맵 파일 */
unsigned    Map_GetKey( MAP *map, int timeout);                             /* 맵 파일 */
int         Map_Message( MAP *map, char *format, ...);                      /* 맵 파일 */
int         Map_AlarmMessage( MAP *map, char *format, ...);                 /* 맵 파일 */
int         Map_GetLineSize( MAP *map);                                     /* 맵 파일 */

/***** Module : map_menu.c *****/
MAP_MENU*   Map_MenuOpen( MAP *map, int menu_type);                         /* 메뉴 open */
int         Map_MenuScan( MAP *map, MAP_MENU *menu);                        /* 메뉴 open */
int         Map_MenuClose( MAP *map, MAP_MENU *menu);                       /* 메뉴 close */
int         Map_Menu( MAP *map, MAP_MENU *menu, int timeout);               /* 메뉴 action */
int         Map_MenuNext( MAP *map, MAP_MENU *menu);                        /* 메뉴 next select */
int         Map_MenuPrev( MAP *map, MAP_MENU *menu);                        /* 메뉴 prev select */
int         Map_MenuHome( MAP *map, MAP_MENU *menu);                        /* 메뉴 home select */
int         Map_MenuEnd( MAP *map, MAP_MENU *menu);                         /* 메뉴 end select */
int         Map_MenuRight( MAP *map, MAP_MENU *menu);                       /* 메뉴 next select */
int         Map_MenuLeft( MAP *map, MAP_MENU *menu);                        /* 메뉴 next select */
int         Map_MenuUp( MAP *map, MAP_MENU *menu);                          /* 메뉴 next select */
int         Map_MenuDown( MAP *map, MAP_MENU *menu);                        /* 메뉴 next select */
int         Map_MenuGetId( MAP *map, MAP_MENU *menu);                       /* current id return */
int         Map_MenuFind( MAP *map, MAP_MENU *menu);                        /* 메뉴 next select */
MAP_FIELD*  Map_MenuGetSelect( MAP *map, MAP_MENU *menu);                   /* 선택된 field의 pointer get */

