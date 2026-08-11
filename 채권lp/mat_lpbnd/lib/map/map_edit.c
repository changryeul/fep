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

#include "etc.h"
#include "map.h"

/** ***************************************************************************
**  @func       int Map_()
**  @param      MAP pointer
**  @param      필드 이름
**  @param      record buf pointer
**  @param      record buf size
**  @return     성공 - 0
**  @retval     실패 - -1
**  @brief
**  Editor open
***************************************************************************** */
MAP_EDIT* Map_EditOpen( MAP *map, int edit_type, char *mode)
{
	int			sz;			/* old size - field data copy */
	MAP_EDIT	*edit;
	MAP_FIELD	*field;

	edit = malloc( sizeof( MAP_EDIT));
	if( edit == NULL)
	{
		LogErr( "edit malloc error. sz=[%d]", sizeof( MAP_EDIT));
		goto error;
	}
	memset( edit, 0x00, sizeof( MAP_EDIT));
	if( mode == NULL)	memcpy( edit->mode, MAP_EDIT_MODE_EDIT, sizeof( edit->mode));
	else				memcpy( edit->mode, mode, sizeof( edit->mode));

	LogDel( "MAP_EDIT_MODE_INSERT=[%d]", edit->mode[ MAP_EDIT_MODE_INSERT]);

	edit->edit = Dll_Open( 0);
	if( edit->edit == NULL)
	{
		LogLib( "edit Dll open error. ");
		goto error_1;
	}

	edit->rec = malloc( edit->sz +1);
	if( edit->rec == NULL)
	{
		LogErr( "edit rec malloc error. sz=[%d]", edit->sz +1);
		goto error_2;
	}
	edit->rec[ edit->sz] = 0;

	field = Dll_GetFirstPtr( map->field);
	while( field != NULL)
	{
		if( edit_type == field->type[ MAP_TYPE_EDIT])
		{
			// memcpy( field->attr, WIN_ATTR_EDIT, sizeof( field->attr));
			LogDel( "field->sz=[%d]", field->sz);
			sz = edit->sz;
			edit->sz += field->sz;
			edit->rec = realloc( edit->rec, edit->sz +1);
			memcpy( &edit->rec[ sz], field->data, field->sz);
			edit->rec[ edit->sz] = 0;
			Dll_Add( edit->edit, field, sizeof( MAP_FIELD));
		}
		field = Dll_GetNextPtr( map->field);
	}

	edit->select = Dll_GetFirstPtr( edit->edit);

	return edit;

	error_2:
		Dll_Close( edit->edit);
	error_1:
		free( edit);
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
**  Editor close
***************************************************************************** */
int Map_EditClose( MAP *map, MAP_EDIT *edit)
{
	free( edit->rec);	edit->rec = NULL;
	Dll_Close( edit->edit);
	if( edit->attr != NULL) free( edit->attr);
	free( edit);
	edit = NULL;
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
**  put data to edit field
***************************************************************************** */
int Map_EditPutData( MAP *map, MAP_EDIT *edit, char *rec, int sz)
{
	int 		pos = 0;
	MAP_FIELD	*field;

	field = Dll_GetFirstPtr( edit->edit);
	while( field != NULL)
	{
		if( pos + field->sz >= sz) break;
		memcpy( field->data, &rec[ pos], field->sz);
		pos += field->sz;
		field = Dll_GetNextPtr( edit->edit);
	}

	return pos;
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
**  Edit process
***************************************************************************** */
int Map_Edit( MAP *map, MAP_EDIT *edit)
{
	int		rtn; 

	if( edit->select == NULL) return 0;

	Map_CursorOn( map);

	while( 1)
	{
		LogDel( "edit->select->name=[%s] edit->select->data=[%p]", edit->select->name, edit->select->data);
		LogDel( "edit->select->type[ MAP_TYPE_TYPE]=[%d]", edit->select->type[ MAP_TYPE_TYPE]);
		switch( edit->select->type[ MAP_TYPE_TYPE])
		{
			case '1':		/* int */
			case '2':		/* float */
			case '3':		/* long */
			case '4':		/* long long */
			case '5':		/* double */
				LogDel( "Edit int  select=[%p]", edit->select);
				LogDel( "          data=[%p] data=[%d]", edit->select->data, *( int *)edit->select->data);
				edit->key = Map_EditFieldNum( map, edit, edit->select);
				if( rtn < 0)
				{
					return -1;
				}
				break;
			default:		/* string or etc */
				LogDel( "Edit string  select=[%p]", edit->select);
				LogDel( "             data=[%p] data=[%s]", edit->select->data, edit->select->data);
				edit->key = Map_EditFieldString( map, edit, edit->select);
				if( rtn < 0)
				{
					return -1;
				}
				break;
		}

		switch( edit->key)
		{
			case WIN_KEY_TAB:
			case WIN_KEY_DOWN:
				rtn = Map_EditNext( map, edit);
				break;
			case WIN_KEY_UP:
				rtn = Map_EditPrev( map, edit);
				break;
			case WIN_KEY_ENTER:
				rtn = Map_EditNext( map, edit);
				edit->pos = 0;
				break;
			default:
			case WIN_KEY_ESC:
				return edit->key;
		}
		if( edit->mode[ MAP_EDIT_MODE_POS] == 0)		edit->pos = 0;
	}

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
**  Edit process
***************************************************************************** */
int Map_EditFile( MAP *map, MAP_EDIT *edit, char **file)
{
	int		rtn; 

	if( edit->select == NULL) return 0;

	Map_CursorOn( map);

	while( 1)
	{
		LogDel( "edit->select->name=[%s] edit->select->data=[%p]", edit->select->name, edit->select->data);
		LogDel( "edit->select->type[ MAP_TYPE_TYPE]=[%d]", edit->select->type[ MAP_TYPE_TYPE]);
		switch( edit->select->type[ MAP_TYPE_TYPE])
		{
			case '1':		/* int */
			case '2':		/* float */
			case '3':		/* long */
			case '4':		/* long long */
			case '5':		/* double */
				LogDel( "Edit int  select=[%p]", edit->select);
				LogDel( "          data=[%p] data=[%d]", edit->select->data, *( int *)edit->select->data);
				edit->key = Map_EditFieldNum( map, edit, edit->select);
				if( rtn < 0)
				{
					return -1;
				}
				break;
			default:		/* string or etc */
				LogDel( "Edit string  select=[%p]", edit->select);
				LogDel( "             data=[%p] data=[%s]", edit->select->data, edit->select->data);
				edit->key = Map_EditFieldString( map, edit, edit->select);
				if( rtn < 0)
				{
					return -1;
				}
				break;
		}

		switch( edit->key)
		{
			case WIN_KEY_TAB:
			case WIN_KEY_DOWN:
				rtn = Map_EditNext( map, edit);
				break;
			case WIN_KEY_UP:
				rtn = Map_EditPrev( map, edit);
				break;
			case WIN_KEY_ENTER:
				rtn = Map_EditNext( map, edit);
				edit->pos = 0;
				break;
			default:
			case WIN_KEY_ESC:
				return edit->key;
		}
		if( edit->mode[ MAP_EDIT_MODE_POS] == 0)		edit->pos = 0;
	}

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
**  Edit process
***************************************************************************** */
int Map_EditView( MAP *map, MAP_EDIT *edit)
{
	int		rtn; 

	if( edit->select == NULL) return 0;

	Map_CursorOn( map);

	while( 1)
	{
		LogDel( "edit->select->type[ MAP_TYPE_TYPE]=[%d]", edit->select->type[ MAP_TYPE_TYPE]);
		switch( edit->select->type[ MAP_TYPE_TYPE])
		{
			case '1':		/* int */
			case '2':		/* float */
			case '3':		/* long */
			case '4':		/* long long */
			case '5':		/* double */
				LogDel( "Edit int  select=[%p]", edit->select);
				LogDel( "          data=[%p] data=[%d]", edit->select->data, *( int *)edit->select->data);
				edit->key = Map_EditFieldNum( map, edit, edit->select);
				if( rtn < 0)
				{
					return -1;
				}
				break;
			default:		/* string or etc */
				LogDel( "Edit string  select=[%p]", edit->select);
				LogDel( "             data=[%p] data=[%s]", edit->select->data, edit->select->data);
				edit->key = Map_EditFieldString( map, edit, edit->select);
				if( rtn < 0)
				{
					return -1;
				}
				break;
		}

		switch( edit->key)
		{
			case WIN_KEY_TAB:
			case WIN_KEY_DOWN:
				rtn = Map_EditNext( map, edit);
				break;
			case WIN_KEY_UP:
				rtn = Map_EditPrev( map, edit);
				break;
			case WIN_KEY_ENTER:
				rtn = Map_EditNext( map, edit);
				edit->pos = 0;
				break;
			default:
			case WIN_KEY_ESC:
				return edit->key;
		}
		if( edit->mode[ MAP_EDIT_MODE_POS] == 0)		edit->pos = 0;
	}

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
**  필드이름으로 한개 필드의 data 입력
***************************************************************************** */
unsigned int Map_EditField( MAP *map, MAP_EDIT *edit, MAP_FIELD *field)
{
	Map_DisplayFieldDataA( map, field, WIN_ATTR_EDIT);
	Map_GotoXY( map, field->x + edit->x, field->y + edit->y);

	if( field->data == NULL)
	{
		memcpy( edit->buf, field->ptr, strlen( field->data) +1);
		edit->buf[ field->sz] = 0;
	}
	else
	{
		memcpy( edit->buf, field->data, strlen( field->data) +1);
		edit->buf[ field->sz] = 0;
	}

	while( edit->key != WIN_KEY_ESC)
	{
		LogDel( "x=[%d] y=[%d]", field->x + edit->pos + edit->x, field->y + edit->y);
		Win_PutXYA( map->win, field->x + edit->x, field->y + edit->y, WIN_ATTR_EDIT, edit->buf);
		Map_GotoXY( map, field->x + edit->pos + edit->x, field->y + edit->y);

		edit->key = Map_GetKey( map, 0);
		LogDel( "edit->key=[%d][0x%08x] edit->pos=[%d]", edit->key, edit->key, edit->pos);

		switch( edit->key)
		{
			case WIN_KEY_UP:
			case WIN_KEY_DOWN:
			case WIN_KEY_TAB:
				goto end;
			default:
				LogDel( "edit->mode[ MAP_EDIT_MODE_INSERT]=[%d]", edit->mode[ MAP_EDIT_MODE_INSERT]);
				if( edit->mode[ MAP_EDIT_MODE_INSERT] == 1)
				{
					edit->buf[ edit->pos++] = edit->key;
				}
				else
				{
					memmove( &edit->buf[ edit->pos +1], &edit->buf[ edit->pos], field->sz - edit->pos);
					edit->buf[ edit->pos++] = edit->key;
					edit->buf[ field->sz] = 0;
				}
				break;
			case WIN_KEY_BACKSPACE:
				if( edit->pos < 1) break;
				memmove( &edit->buf[ edit->pos -1], &edit->buf[ edit->pos], field->sz - edit->pos +1);
				edit->pos--;
				edit->buf[ field->sz] = ' ';
				edit->buf[ field->sz +1] = 0;
				break;
			case WIN_KEY_LEFT:
				edit->pos--;
				break;
			case WIN_KEY_RIGHT:
				edit->pos++;
				break;
			case WIN_KEY_INS:
				edit->mode[ MAP_EDIT_MODE_INSERT] = ( edit->mode[ MAP_EDIT_MODE_INSERT] ) ? 0:1;
				break;
			case WIN_KEY_DEL:
				memset( edit->buf, ' ', field->sz);
				break;
			case WIN_KEY_HOME:
				edit->pos = 0;
				break;
			case WIN_KEY_END:
				edit->pos = field->sz -1;
				while( edit->buf[ edit->pos] == ' ') edit->pos--;
				edit->pos++;
				break;
			case WIN_KEY_ENTER:
				memcpy( field->data, edit->buf, field->sz +1);
				goto end;
		}
		LogDel( "pos=[%d] sz=[%d] buf=[%p]", edit->pos, field->sz, edit->buf);
		if( edit->pos < 0)			edit->pos = 0;
		else
		if( edit->pos >= field->sz)	edit->pos = field->sz-1;
	}

	end:
	memcpy( field->data, edit->buf, field->sz);
	Map_DisplayFieldData( map, field);
	return edit->key;
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
**  필드이름으로 한개 필드의 data 입력
***************************************************************************** */
unsigned int Map_EditFieldString( MAP *map, MAP_EDIT *edit, MAP_FIELD *field)
{
	int				type;
	int				x, y;
	char			rec[ 512];

	x = map->x + field->x + edit->x;
	y = map->y + field->y + edit->y;
#if 0
	edit->sz  = strlen( field->data);
#else
	edit->sz  = field->sz;
#endif

	memset( rec, 0x00, edit->sz +2);
	if( field->ptr == NULL)	type = '0';

	LogDel( "type=[%d] field->sz=[%d] ptr=[%p] data=[%p]", type, field->sz, field->ptr, field->data);
	if( type == '1')	sprintf( rec, "%s", ( char *)field->ptr);
	else				sprintf( rec, "%s", field->data);
	rec[ edit->sz] = 0;

#if 0
	Map_DisplayFieldDataA( map, field, WIN_ATTR_EDIT);
	Map_GotoXY( map, field->x + edit->x, field->y + edit->y);
#endif

	while( edit->key != WIN_KEY_ESC)
	{
		LogDel( "x=[%d] y=[%d]", edit->pos + x, y);
		if( edit->attr == NULL)	Win_PutXYA( map->win, x, y, field->attr, rec);
		else					Win_PutXYA( map->win, x, y, edit->attr, rec);
		Map_GotoXY( map, x + edit->pos, y);

		edit->key = Map_GetKey( map, 0);
		LogDel( "edit->key=[%d][0x%08x] pos=[%d]", edit->key, edit->key, pos);

		switch( edit->key)
		{
			case WIN_KEY_UP:
			case WIN_KEY_DOWN:
			case WIN_KEY_TAB:
				goto end;
			case WIN_KEY_BACKSPACE:
				if( edit->pos < 1) break;
				memmove( &rec[ edit->pos -1], &rec[ edit->pos], edit->sz - edit->pos +1);
				edit->pos--;
				rec[ edit->sz -1] = ' ';
				rec[ edit->sz] = 0;
				break;
			case WIN_KEY_LEFT:
				edit->pos--;
				break;
			case WIN_KEY_RIGHT:
				edit->pos++;
				break;
			case WIN_KEY_INS:
				edit->mode[ MAP_EDIT_MODE_INSERT] = ( edit->mode[ MAP_EDIT_MODE_INSERT] ) ? 0:1;
				break;
			case WIN_KEY_DEL:
				memmove( &rec[ edit->pos], &rec[ edit->pos +1], edit->sz - edit->pos);
				rec[ edit->sz -1] = ' ';
				rec[ edit->sz] = 0;
				LogSetDump( LOG_DUMP_DEC);
				LogDump( rec, edit->sz + 5, "%s", "HERE");
				LogDel( "rec=[%s]", rec);
				break;
			case WIN_KEY_HOME:
				edit->pos = 0;
				break;
			case WIN_KEY_END:
				edit->pos = edit->sz -1;
				while( rec[ edit->pos] == ' ') edit->pos--;
				edit->pos++;
				break;
			case WIN_KEY_ENTER:
				memcpy( field->data, rec, edit->sz +1);
				goto end;
			default:
				LogDel( "edit->mode[ MAP_EDIT_MODE_INSERT]=[%d]", edit->mode[ MAP_EDIT_MODE_INSERT]);
				if( edit->mode[ MAP_EDIT_MODE_INSERT] == 1)
				{
					rec[ edit->pos++] = edit->key;
				}
				else
				{
					memmove( &rec[ edit->pos +1], &rec[ edit->pos], edit->sz - edit->pos);
					rec[ edit->pos++] = edit->key;
					rec[ edit->sz] = 0;
				}
				break;
		}
		LogDel( "pos=[%d] sz=[%d] buf=[%p]", edit->pos, edit->sz, edit->rec);
		if( edit->pos < 0)			edit->pos = 0;
		else
		if( edit->pos >= edit->sz)	edit->pos = edit->sz-1;
	}

	end:
#if 0
	memcpy( field->data, rec, edit->sz);
#endif
	Map_DisplayFieldData( map, field);
	return edit->key;
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
**  필드이름으로 한개 필드의 data 입력
***************************************************************************** */
unsigned int Map_EditFieldNum( MAP *map, MAP_EDIT *edit, MAP_FIELD *field)
{
	int				x, y, sz;
	int				pos = 0;
	int				type;
	char			rec[ 512];
	char			*format;

	x = map->x + field->x + edit->x;
	y = map->y + field->y + edit->y;
	sz = field->vsz;
	pos = sz -1;
	memset( rec, 0x00, sz +2);
	if( field->ptr == NULL)	type = '0';

	switch( field->type[ MAP_TYPE_TYPE])
	{
		case '1':		/* int */
			format = "%*d";
			if( type == '1')	sprintf( rec, format, sz, *( int *)field->ptr);
			else				sprintf( rec, format, sz, *( int *)field->data);
			break;
		case '2':		/* float */
			format = "%*f";
			if( type == '1')	sprintf( rec, format, sz, *( float *)field->ptr);
			else				sprintf( rec, format, sz, *( float *)field->data);
			break;
		case '3':		/* long */
			format = "%*ld";
			if( type == '1')	sprintf( rec, format, sz, *( long *)field->ptr);
			else				sprintf( rec, format, sz, *( long *)field->data);
			break;
		case '4':		/* long long */
			format = "%*lld";
			if( type == '1')	sprintf( rec, format, sz, *( long long *)field->ptr);
			else				sprintf( rec, format, sz, *( long long *)field->data);
			break;
		case '5':		/* double */
			format = "%*f";
			if( type == '1')	sprintf( rec, format, sz, *( double *)field->ptr);
			else				sprintf( rec, format, sz, *( double *)field->data);
			break;
		default:
			format = "%*d";
			break;
	}


#if 0
	Map_DisplayFieldDataA( map, field, WIN_ATTR_EDIT);
	Map_GotoXY( map, field->x + edit->x, field->y + edit->y);
#endif

	LogDel( "%s", format);
	LogDel( "%s", rec);
	while( edit->key != WIN_KEY_ESC)
	{
		LogDel( "x=[%d] y=[%d]", field->x + edit->pos + edit->x, field->y + edit->y);
		Win_PutXYA( map->win, x, y, WIN_ATTR_EDIT, rec);
		Map_GotoXY( map, x + pos, y);

		edit->key = Map_GetKey( map, 0);
		LogDel( "edit->key=[%d][0x%08x] pos=[%d]", edit->key, edit->key, pos);

		switch( edit->key)
		{
			case WIN_KEY_UP:
			case WIN_KEY_DOWN:
			case WIN_KEY_TAB:
				goto end;
			case WIN_KEY_BACKSPACE:
				memmove( &rec[ 1], &rec[ 0], pos -1);
				rec[ 0] = ' ';
				break;
			case WIN_KEY_DEL:
				memmove( &rec[ 1], &rec[ 0], pos);
				rec[ 0] = ' ';
				break;
			case WIN_KEY_LEFT:
				pos--;
				break;
			case WIN_KEY_RIGHT:
				pos++;
				break;
			case WIN_KEY_INS:
				break;
			case WIN_KEY_HOME:
				pos = 0;
				while( rec[ pos] == ' ') pos++;
				break;
			case WIN_KEY_END:
				pos = sz -1;
				break;
			case WIN_KEY_ENTER:
				goto end;
			default:
				if( !isalnum( edit->key)) goto end;
				if( pos >= sz -1)
				{
					memmove( &rec[ 0], &rec[ 1], sz);
					rec[ pos] = edit->key;
					rec[ pos +1] = 0;
				}
				else
				{
					memmove( &rec[ 0], &rec[ 1], pos -1);
					rec[ pos -1] = edit->key;
				}
				break;
		}
		LogDel( "pos=[%d] sz=[%d] buf=[%p]", pos, field->sz, rec);
		if( pos < 0)			pos = 0;
		else
		if( pos >= sz)	pos = sz -1;
	}

	end:

	switch( field->type[ MAP_TYPE_TYPE])
	{
		case '1':		/* int */
			if( type == '1')	*( int *)field->ptr = atoi( rec);
			else				*( int *)field->data = atoi( rec);
			break;
		case '2':		/* float */
			if( type == '1')	*( float *)field->ptr = strtof( rec, NULL);
			else				*( float *)field->data = strtof( rec, NULL);
			break;
		case '3':		/* long */
			if( type == '1')	*( long *)field->ptr = atol( rec);
			else				*( long *)field->data = atol( rec);
			break;
		case '4':		/* long long */
			if( type == '1')	*( long long *)field->ptr = atoll( rec);
			else				*( long long *)field->data = atoll( rec);
			break;
		case '5':		/* double */
			if( type == '1')	*( double *)field->ptr = strtod( rec, NULL);
			else				*( double *)field->data = strtod( rec, NULL);
			break;
		default:
			format = "%*d";
			break;
	}
	Map_DisplayFieldData( map, field);
	return edit->key;
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
**  next field edit
***************************************************************************** */
int Map_EditNext( MAP *map, MAP_EDIT *edit)
{
	MAP_FIELD	*field;

	if( edit->select == NULL) return -1;

	edit->select->type[ MAP_TYPE_SELECT] = '0';

	field = Dll_GetFirstPtr( edit->edit);
	while( field != NULL)
	{
		if( field == edit->select)
		{
			field = Dll_GetNextPtr( edit->edit);
			if( field == NULL)
			{
				edit->select->type[ MAP_TYPE_SELECT] = '1';
				return 0;
			}
			edit->select = field;
			edit->select->type[ MAP_TYPE_SELECT] = '1';
			return 1;
		}

		field = Dll_GetNextPtr( edit->edit);
	}

	return 0;
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
**  prev field edit
***************************************************************************** */
int Map_EditPrev( MAP *map, MAP_EDIT *edit)
{
	if( edit->select == NULL) return -1;

	edit->select->type[ MAP_TYPE_SELECT] = '0';

	Dll_FindPtr( edit->edit, edit->select, sizeof( MAP_FIELD));
	edit->select = Dll_GetPrevPtr( edit->edit);
	if( edit->select == NULL) edit->select = Dll_GetLastPtr( edit->edit);
	if( edit->select == NULL) return -1;

	edit->select->type[ MAP_TYPE_SELECT] = '1';

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
**  first field edit
***************************************************************************** */
int Map_EditHome( MAP *map, MAP_EDIT *edit)
{
	if( edit->select == NULL) return -1;

	edit->select->type[ MAP_TYPE_SELECT] = '0';

	edit->select = Dll_GetFirstPtr( edit->edit);
	if( edit->select == NULL) return -1;

	edit->select->type[ MAP_TYPE_SELECT] = '1';

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
**  last field edit
***************************************************************************** */
int Map_EditEnd( MAP *map, MAP_EDIT *edit)
{
	if( edit->select == NULL) return -1;

	edit->select->type[ MAP_TYPE_SELECT] = '0';

	edit->select = Dll_GetLastPtr( edit->edit);
	if( edit->select == NULL) return -1;

	edit->select->type[ MAP_TYPE_SELECT] = '1';

	return 1;
}

























