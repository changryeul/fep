#ifndef DLL_H
#define	DLL_H

#define	DLL_GET_PTR		0
#define DLL_GET_VAR		1

#define	DLL_CURR		0
#define	DLL_NEXT		1
#define	DLL_PREV		2
#define	DLL_FIRST		3
#define	DLL_LAST		4

#define Dll_GetCurr( dll, rec, sz)		Dll_GetData( ( DLL *)dll, DLL_CURR, rec, sz)
#define Dll_GetNext( dll, rec, sz)		Dll_GetData( ( DLL *)dll, DLL_NEXT, rec, sz)
#define Dll_GetPrev( dll, rec, sz)		Dll_GetData( ( DLL *)dll, DLL_PREV, rec, sz)
#define Dll_GetFirst( dll, rec, sz)		Dll_GetData( ( DLL *)dll, DLL_FIRST, rec, sz)
#define Dll_GetLast( dll, rec, sz)		Dll_GetData( ( DLL *)dll, DLL_LAST, rec, sz)

#define Dll_GetCurrPtr( dll)			Dll_GetDataPtr( ( DLL *)dll, DLL_CURR)
#define Dll_GetNextPtr( dll)			Dll_GetDataPtr( ( DLL *)dll, DLL_NEXT)
#define Dll_GetPrevPtr( dll)			Dll_GetDataPtr( ( DLL *)dll, DLL_PREV)
#define Dll_GetFirstPtr( dll)			Dll_GetDataPtr( ( DLL *)dll, DLL_FIRST)
#define Dll_GetLastPtr( dll)			Dll_GetDataPtr( ( DLL *)dll, DLL_LAST)

typedef struct __dll_rec__
{
	struct __dll_rec__	*prev;
	struct __dll_rec__	*next;
	void	*rec;
	size_t	sz;
}	DLL_REC;

typedef struct __dll__
{
	DLL_REC	*star;
	DLL_REC	*last;
	DLL_REC	*curr;
	int	recs;
	int	opt;						/* 1=malloc, 0=pointer */
}	DLL;

#endif

/***** Module : dll.c *****/
DLL*        Dll_Open( int opt);
DLL_REC*    Dll_MakeRec( DLL *dll, void *data, int sz);
void*       Dll_AddFirst( DLL *dll, void *data, int sz);
void*       Dll_Add( DLL *dll, void *data, int sz);
void*       Dll_Insert( DLL *dll, void *data, int sz);
void*       Dll_Update( DLL *dll, void *data, int sz);
DLL_REC*    Dll_Find( DLL *dll, void *data, int sz);
DLL_REC*    Dll_FindPtr( DLL *dll, void *data, int sz);
DLL_REC*    Dll_FindFunc( DLL *dll, void *data, int sz, int (*comp_func)( void *arg, void *rec, int sz));
DLL_REC*    Dll_First( DLL *dll);
DLL_REC*    Dll_Last( DLL *dll);
DLL_REC*    Dll_Curr( DLL *dll);
DLL_REC*    Dll_Next( DLL *dll);
DLL_REC*    Dll_Prev( DLL *dll);
int         Dll_Get( DLL *dll, char *rec, int sz);
int         Dll_GetData( DLL *dll, int option, char *rec, int sz);
void*       Dll_GetDataPtr( DLL *dll, int option);
int         Dll_GetFind( DLL *dll, char *find, int f_sz, char *rec, int sz);
void*       Dll_GetFindPtr( DLL *dll, char *find, int f_sz);
void*       Dll_GetFindFuncPtr( DLL *dll, char *find, int f_sz, int (*comp_func)( void *arg, void *rec, int sz));
int         Dll_Pop( DLL *dll, char *rec, int sz);
void*       Dll_Del( DLL *dll);
void*       Dll_DelAll( DLL *dll);
int         Dll_Print( DLL *dll);
int         Dll_PrintFunc( DLL *dll, int(*func)( char *rec, void *data, int sz));
int         Dll_Close( DLL *dll);
DLL_REC*    Dll_FindFuncLocal( DLL *dll, void *data, int sz, int (*comp_func)( void *arg, void *rec, int sz));
void*       Dll_GetFindFuncPtrLocal( DLL *dll, char *find, int f_sz, int (*comp_func)( void *arg, void *rec, int sz));

