/***** Module : map.c *****/
int         MapInit();                                                      /* 맵 파용하기 위한 터미널 초기화 */
int         MapEnd();                                                       /* 터미널 세팅을 끝냄 */
MAP*        Map_Open( char *f_name);                                        /* 맵 파일을 열어 출력을위한 초기화 */
int         Map_Close( MAP *map);                                           /* 맵 파일 */
int         Map_SetBuffer( MAP *map, char *buffer, int sz);                 /* 맵 파일 */
int         Map_GetBuffer( MAP *map);                                       /* 맵 파일 */
int         Map_DisplayMap( MAP *map);                                      /* 맵 파일 */
int         Map_DisplayField( MAP *map);                                    /* 맵 파일 */
int         Map_DisplayFieldData( MAP *map, MAP_FIELD *field);              /* 한개의 필드 출력 */
int         Map_DisplayFieldDataA( MAP *map, MAP_FIELD *field, char *attr); /* 한개의 필드를 인수의 attribute로  출력 */
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
int         Map_MenuFind( MAP *map, MAP_MENU *menu);                        /* 메뉴 next select */

/***** Module : map_edit.c *****/
MAP_EDIT*   Map_EditOpen( MAP *map, int edit_type, char *mode);             /* Editor open */
int         Map_EditClose( MAP *map, MAP_EDIT *edit);                       /* Editor close */
int         Map_EditPutData( MAP *map, MAP_EDIT *edit, char *rec, int sz);  /* put data to edit field */
int         Map_Edit( MAP *map, MAP_EDIT *edit);                            /* Edit process */
unsigned    Map_EditField( MAP *map, MAP_EDIT *edit, MAP_FIELD *field);     /* 필드이름으로 한개 필드의 data 입력 */
int         Map_EditNext( MAP *map, MAP_EDIT *edit);                        /* next field edit */
int         Map_EditPrev( MAP *map, MAP_EDIT *edit);                        /* prev field edit */
int         Map_EditHome( MAP *map, MAP_EDIT *edit);                        /* first field edit */
int         Map_EditEnd( MAP *map, MAP_EDIT *edit);                         /* last field edit */

