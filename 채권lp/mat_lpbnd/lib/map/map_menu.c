/** ***************************************************************************
**  @file       map.c
**  @date       2022/08/23
**  @author     cdc
**  @version    V2.0.20220823
**  @brif
**  map library module
***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "log.h"
#include "etc.h"
#include "map.h"

extern int		Continue;		/* loop stop by signal */

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @param      필드 이름
**  @param      record buf pointer
**  @param      record buf size
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  메뉴 open
***************************************************************************** */
MAP_MENU* Map_MenuOpen( MAP *map, int menu_type)
{
	MAP_MENU	*menu;
	MAP_FIELD	*field;

	menu = malloc( sizeof( MAP_MENU));
	if( menu == NULL)
	{
		LogErr( "menu malloc error. sz=[%d]", sizeof( MAP_MENU));
		goto error;
	}
	memset( menu, 0x00, sizeof( MAP_MENU));

	menu->type = menu_type;

	menu->menu = Dll_Open( 0);
	if( menu->menu == NULL)
	{
		LogLib( "menu Dll open error. ");
		goto error_1;
	}

	field = Dll_GetFirstPtr( map->field);
	while( field != NULL)
	{
		LogDel( "name=[%s] menu_type=[%d] field->type=[%d]", field->name, menu_type, field->type[ MAP_TYPE_MENU]);
		if( menu->type == field->type[ MAP_TYPE_MENU])
		{
			/*
			LogDbg( "check menu name=[%s] field->data=[%p] field->ptr=[%p]", 
					field->name, field->data, field->ptr);
			*/
			if( field->data != NULL || field->ptr != NULL) 
			{
				/*
				LogDbg( "add menu name=[%s] menu_type=[%d] field->type=[%d]", 
						field->name, menu_type, field->type[ MAP_TYPE_MENU]);
				*/
				Dll_Add( menu->menu, field, sizeof( MAP_FIELD));
			}
		}
		field = Dll_GetNextPtr( map->field);
	}

	menu->select = Dll_GetFirstPtr( menu->menu);
	if( menu->select == NULL)
	{
		LogDel( "not found menu");
	}
	else
	{
		LogDel( "select name=[%s] menu_type=[%d] field->type=[%d]", menu->select->name);
		menu->select->type[ MAP_TYPE_SELECT] = '1';
	}

	return menu;

	error_1:
		free( menu);
	error:
		return NULL;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @param      필드 이름
**  @param      record buf pointer
**  @param      record buf size
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  메뉴 open
***************************************************************************** */
Map_MenuScan( MAP *map, MAP_MENU *menu)
{
	MAP_FIELD	*field;

	field = Dll_GetFirstPtr( map->field);
	while( field != NULL)
	{
		LogDel( "name=[%s] menu_type=[%d] field->type=[%d]", field->name, menu_type, field->type[ MAP_TYPE_MENU]);
		if( menu->type == field->type[ MAP_TYPE_MENU])
		{
			/*
			LogDbg( "check menu name=[%s] field->data=[%p] field->ptr=[%p]", 
					field->name, field->data, field->ptr);
			*/
			if( field->data != NULL || field->ptr != NULL) 
			{
				/*
				LogDbg( "add menu name=[%s] menu_type=[%d] field->type=[%d]", 
						field->name, menu_type, field->type[ MAP_TYPE_MENU]);
				*/
				Dll_Add( menu->menu, field, sizeof( MAP_FIELD));
			}
		}
		field = Dll_GetNextPtr( map->field);
	}

	menu->select = Dll_GetFirstPtr( menu->menu);
	if( menu->select == NULL)
	{
		LogDel( "not found menu");
	}
	else
	{
		LogDel( "select name=[%s] menu_type=[%d] field->type=[%d]", menu->select->name);
		menu->select->type[ MAP_TYPE_SELECT] = '1';
	}

	return 1;

	error_1:
		free( menu);
	error:
		return NULL;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @param      필드 이름
**  @param      record buf pointer
**  @param      record buf size
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  메뉴 close
***************************************************************************** */
int Map_MenuClose( MAP *map, MAP_MENU *menu)
{
	int			rtn;

	rtn = Dll_Close( menu->menu);
	free( menu);
	menu = NULL;

	return rtn;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @param      필드 이름
**  @param      record buf pointer
**  @param      record buf size
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  메뉴 action
***************************************************************************** */
int Map_Menu( MAP *map, MAP_MENU *menu, int timeout)
{
	int			rtn;

	LogDel( "menu->select->vsz=[%d] data_len=[%d]", menu->select->vsz, strlen( menu->select->data));
	if( menu->select == NULL) 
	{
		menu->key =  Map_GetKey( map, timeout);
		if( menu->key == WIN_KEY_ESC)	return -1;
		return 0;
	}
	Win_GotoXY( map->win, menu->select->x + map->x, menu->select->y + map->y);
	rtn = Map_DisplayFieldDataA( map, menu->select, MAP_MENU_ATTR);
	/* Map_DisplayFieldDataA( map, menu->select, MAP_EDIT_ATTR); */
	LogDel( "Map_Menu ... wait key.");
	Win_GotoXY( map->win, menu->select->x + map->x, menu->select->y + map->y);
	menu->key = Map_GetKey( map, map->timeout);
	LogDel( "Map_Menu ... key=[%d]", menu->key);
	/* Map_DisplayFieldData( map, menu->select); */
	switch( menu->key)
	{
		case 0:
			rtn = 0;
			break;
		case WIN_KEY_ESC:
			rtn = -1;
			break;
		case WIN_KEY_SPACE:
			rtn = 0;
			break;
		case WIN_KEY_ENTER:
			if( menu->select == NULL) return 0;
			rtn = menu->select->id;
			break;
		case WIN_KEY_HOME:
			rtn = Map_MenuHome( map, menu);
			rtn = 0;
			break;
		case WIN_KEY_END:
			rtn = Map_MenuEnd( map, menu);
			rtn = 0;
			break;
		case WIN_KEY_TAB:
			rtn = Map_MenuNext( map, menu);
			rtn = 0;
			break;
#if 0
		case WIN_KEY_PGDN:
			rtn = Map_MenuNext( map, menu);
			rtn = 0;
			break;
		case WIN_KEY_PGUP:
			rtn = Map_MenuPrev( map, menu);
			rtn = 0;
			break;
#else
		case WIN_KEY_PGDN:
		case WIN_KEY_PGUP:
			rtn = 0;
			break;
#endif
		case WIN_KEY_RIGHT:
			rtn = Map_MenuRight( map, menu);
			rtn = 0;
			break;
		case WIN_KEY_LEFT:
			rtn = Map_MenuLeft( map, menu);
			rtn = 0;
			break;
		case WIN_KEY_UP:
			rtn = Map_MenuUp( map, menu);
			rtn = 0;
			break;
		case WIN_KEY_DOWN:
			rtn = Map_MenuDown( map, menu);
			rtn = 0;
			break;
		default:
			/*
			rtn = Map_MenuFind( map, menu);
			*/
			rtn = 0;
			break;
	}

	/*
	rtn = Map_DisplayFieldData( map, menu->select);
	*/
	LogDel( "Map_Menu end. key=[%d] rtn=[%d]", menu->key, rtn);

	return rtn;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @param      필드 이름
**  @param      record buf pointer
**  @param      record buf size
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  메뉴 next select
***************************************************************************** */
int Map_MenuNext( MAP *map, MAP_MENU *menu)
{
	MAP_FIELD	*select;

	if( menu->select == NULL) return -1;

	Dll_FindPtr( menu->menu, menu->select, sizeof( MAP_FIELD));
	select = Dll_GetNextPtr( menu->menu);
	if( select == NULL) menu->select = Dll_GetFirstPtr( menu->menu);
	if( select == NULL) return -1;

	menu->select->type[ MAP_TYPE_SELECT] = '0';
	menu->select = select;
	menu->select->type[ MAP_TYPE_SELECT] = '1';

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @param      필드 이름
**  @param      record buf pointer
**  @param      record buf size
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  메뉴 prev select
***************************************************************************** */
int Map_MenuPrev( MAP *map, MAP_MENU *menu)
{
	MAP_FIELD	*select;

	if( menu->select == NULL) return -1;

	Dll_FindPtr( menu->menu, menu->select, sizeof( MAP_FIELD));
	select = Dll_GetPrevPtr( menu->menu);
	if( select == NULL) menu->select = Dll_GetLastPtr( menu->menu);
	if( select == NULL) return -1;

	menu->select->type[ MAP_TYPE_SELECT] = '0';
	menu->select = select;
	menu->select->type[ MAP_TYPE_SELECT] = '1';

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @param      필드 이름
**  @param      record buf pointer
**  @param      record buf size
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  메뉴 home select
***************************************************************************** */
int Map_MenuHome( MAP *map, MAP_MENU *menu)
{
	MAP_FIELD	*select;

	select = Dll_GetFirstPtr( menu->menu);
	if( select == NULL) return -1;

	menu->select->type[ MAP_TYPE_SELECT] = '0';
	menu->select = select;
	menu->select->type[ MAP_TYPE_SELECT] = '1';

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @param      필드 이름
**  @param      record buf pointer
**  @param      record buf size
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  메뉴 end select
***************************************************************************** */
int Map_MenuEnd( MAP *map, MAP_MENU *menu)
{
	MAP_FIELD	*select;

	select = Dll_GetLastPtr( menu->menu);
	if( select == NULL) return -1;

	menu->select->type[ MAP_TYPE_SELECT] = '0';
	menu->select = select;
	menu->select->type[ MAP_TYPE_SELECT] = '1';

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @param      필드 이름
**  @param      record buf pointer
**  @param      record buf size
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  메뉴 next select
***************************************************************************** */
int Map_MenuRight( MAP *map, MAP_MENU *menu)
{
	MAP_FIELD	*field, *select = NULL;
	int			y;
	int			x;

	if( menu->select == NULL) return -1;

	y = menu->select->y;
	x = menu->select->x;

	field = Dll_GetFirstPtr( menu->menu);
	while( field != NULL)
	{
		if( field->y == y)
		{
			if( field->x > x)	
			{
				if( select != NULL)
				{
					if( field->x < select->x) select = field;
				}
				else
				{
					select = field;
				}
			}
		}
		field = Dll_GetNextPtr( menu->menu);
	}

	if( select == NULL) return -1;

	menu->select->type[ MAP_TYPE_SELECT] = '0';
	menu->select = select;
	menu->select->type[ MAP_TYPE_SELECT] = '1';

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @param      필드 이름
**  @param      record buf pointer
**  @param      record buf size
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  메뉴 next select
***************************************************************************** */
int Map_MenuLeft( MAP *map, MAP_MENU *menu)
{
	MAP_FIELD	*field, *select = NULL;
	int			y;
	int			x;

	if( menu->select == NULL) return -1;

	y = menu->select->y;
	x = menu->select->x;

	field = Dll_GetFirstPtr( menu->menu);
	while( field != NULL)
	{
		if( field->y == y)
		{
			if( field->x < x)	
			{
				if( select != NULL)
				{
					if( field->x > select->x) select = field;
				}
				else
				{
					select = field;
				}
			}
		}
		field = Dll_GetNextPtr( menu->menu);
	}

	if( select == NULL) return -1;

	menu->select->type[ MAP_TYPE_SELECT] = '0';
	menu->select = select;
	menu->select->type[ MAP_TYPE_SELECT] = '1';

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @param      필드 이름
**  @param      record buf pointer
**  @param      record buf size
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  메뉴 next select
***************************************************************************** */
int Map_MenuUp( MAP *map, MAP_MENU *menu)
{
	MAP_FIELD	*field, *select = NULL;
	int			y;
	int			x;

	if( menu->select == NULL) return -1;

	y = menu->select->y;
	x = menu->select->x;

	field = Dll_GetFirstPtr( menu->menu);
	while( field != NULL)
	{
		if( field->x == x)
		{
			if( field->y < y)	
			{
				if( select != NULL)
				{
					if( field->y > select->y) select = field;
				}
				else
				{
					select = field;
				}
			}
		}
		field = Dll_GetNextPtr( menu->menu);
	}

	if( select == NULL) return -1;

	menu->select->type[ MAP_TYPE_SELECT] = '0';
	menu->select = select;
	menu->select->type[ MAP_TYPE_SELECT] = '1';

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @param      필드 이름
**  @param      record buf pointer
**  @param      record buf size
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  메뉴 next select
***************************************************************************** */
int Map_MenuDown( MAP *map, MAP_MENU *menu)
{
	MAP_FIELD	*field, *select = NULL;
	int			y;
	int			x;

	if( menu->select == NULL) return -1;

	y = menu->select->y;
	x = menu->select->x;

	field = Dll_GetFirstPtr( menu->menu);
	while( field != NULL)
	{
		if( field->x == x)
		{
			if( field->y > y)	
			{
				if( select != NULL)
				{
					if( field->y < select->y) select = field;
				}
				else
				{
					select = field;
				}
			}
		}
		field = Dll_GetNextPtr( menu->menu);
	}

	if( select == NULL) return -1;

	menu->select->type[ MAP_TYPE_SELECT] = '0';
	menu->select = select;
	menu->select->type[ MAP_TYPE_SELECT] = '1';

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @param      필드 이름
**  @param      record buf pointer
**  @param      record buf size
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  current id return
***************************************************************************** */
int Map_MenuGetId( MAP *map, MAP_MENU *menu)
{
	return menu->select->id;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @param      필드 이름
**  @param      record buf pointer
**  @param      record buf size
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  메뉴 next select
***************************************************************************** */
int Map_MenuFind( MAP *map, MAP_MENU *menu)
{
	MAP_FIELD	*field, *select = NULL;
	int			pos = 0;

	if( menu->select == NULL) return -1;

	Dll_FindPtr( menu->menu, menu->select, sizeof( MAP_FIELD));

	/* 입력 key와 일치하는 field 찾기 - space 제외 */
	field = Dll_GetNextPtr( menu->menu);
	while( field != NULL)
	{
		if( field->data != NULL)
		{
			pos = 0;
			while( field->data[ pos] == ' ') pos++;
			LogDel( "data[%d]=[%c] key=[%c]", pos, field->data[ pos], menu->key);
			if( toupper( field->data[ pos]) == toupper( menu->key))
			{
				select = field;
				break;
			}
		}
		field = Dll_GetNextPtr( menu->menu);
	}

	/* 없을시 처음부터 현재까지 검색 */
	if( select == NULL)
	{
		field = Dll_GetFirstPtr( menu->menu);
		while( field != menu->select)
		{
			pos = 0;
			while( field->data[ pos] == ' ') pos++;
			if( toupper( field->data[ pos]) == toupper( menu->key))
			{
				select = field;
				break;
			}
			field = Dll_GetNextPtr( menu->menu);
		}
	}

	if( select == NULL) return -1;

	menu->select->type[ MAP_TYPE_SELECT] = '0';
	menu->select = select;
	menu->select->type[ MAP_TYPE_SELECT] = '1';

	return 1;
}

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @param      필드 이름
**  @param      record buf pointer
**  @param      record buf size
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  선택된 field의 pointer get
***************************************************************************** */
MAP_FIELD* Map_MenuGetSelect( MAP *map, MAP_MENU *menu)
{
	return menu->select;
}

