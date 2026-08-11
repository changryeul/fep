DLL_REC	*Dll_FindFunc( DLL *dll, void *data, int sz, int (*comp_func)( void *arg, void *rec, int sz))
{
	int		i = 1;
	DLL_REC		*rp;

	rp = dll->star;

	while( rp != NULL)
	{
		if( comp_func( data, rp->rec, sz) == 0)
		{
			dll->curr = rp;
			return rp;
		}
		rp = rp->next;
	}

	return NULL;
}

DLL_REC	*Dll_First( DLL *dll)
{
	int		i = 1;
	DLL_REC		*rp;

	dll->curr = dll->star;
	return dll->star;
}

DLL_REC	*Dll_Last( DLL *dll)
{
	int		i = 1;
	DLL_REC		*rp;

	dll->curr = dll->last;
	return dll->last;
}

DLL_REC	*Dll_Curr( DLL *dll)
{
	int		i = 1;
	DLL_REC		*rp;

	return dll->curr;
}

DLL_REC	*Dll_Next( DLL *dll)
{
	int		i = 1;
	DLL_REC		*rp;

	if( dll->curr == NULL) return NULL;
	if( dll->curr->next == NULL) return NULL;

	dll->curr = dll->curr->next;
	return dll->curr;
}

DLL_REC	*Dll_Prev( DLL *dll)
{
	int		i = 1;
	DLL_REC		*rp;

	if( dll->curr == NULL) return NULL;
	if( dll->curr->prev == NULL) return NULL;

	dll->curr = dll->curr->prev;
	return dll->curr;
}

int	Dll_Get( DLL *dll, char *rec, int sz)
{
	int		cp_sz = sz;
	DLL_REC		*rp;

	if( dll->recs <= 0) return 0;
	if( dll->curr == NULL) return 0;

	rp = dll->curr;
	if( rp->sz < sz) 
	{
		cp_sz = rp->sz;
		rec[ cp_sz] = 0;
	}
	memcpy( rec, rp->rec, cp_sz);

	return rp->sz;
}

int	Dll_GetData( DLL *dll, int option, char *rec, int sz)
{
	DLL_REC		*rp;

	if( dll == NULL) return -1;

	switch( option)
	{
		case DLL_CURR : rp = Dll_Curr( dll);  break;
		case DLL_NEXT : rp = Dll_Next( dll);  break;
		case DLL_PREV : rp = Dll_Prev( dll);  break;
		case DLL_FIRST: rp = Dll_First( dll); break;
		case DLL_LAST : rp = Dll_Last( dll);  break;
		default:                              return 0;
	}
	if( rp == NULL) return 0;
	if( rp->sz > sz) 
	{
		LogCri( "size error. dll=[%d] arg=[%d]", rp->sz, sz);
		return 0;
	}
	memcpy( rec, rp->rec, rp->sz);

	return rp->sz;
}

void *Dll_GetDataPtr( DLL *dll, int option)
{
	DLL_REC		*rp;

	if( dll == NULL) return NULL;

	switch( option)
	{
		case DLL_CURR : rp = Dll_Curr( dll);  break;
		case DLL_NEXT : rp = Dll_Next( dll);  break;
		case DLL_PREV : rp = Dll_Prev( dll);  break;
		case DLL_FIRST: rp = Dll_First( dll); break;
		case DLL_LAST : rp = Dll_Last( dll);  break;
		default:                              return 0;
	}
	if( rp == NULL) return NULL;

	return rp->rec;
}

int	Dll_GetFind( DLL *dll, char *find, int f_sz, char *rec, int sz)
{
	DLL_REC		*rp;

	rp = Dll_Find( dll, find, f_sz);
	if( rp == NULL) return 0;
	if( rp->sz > sz) 
	{
		LogCri( "size error. dll=[%d] arg=[%d]", rp->sz, sz);
		return 0;
	}
	memcpy( rec, rp->rec, rp->sz);

	return rp->sz;
}

void *Dll_GetFindPtr( DLL *dll, char *find, int f_sz)
{
	DLL_REC		*rp;

	rp = Dll_Find( dll, find, f_sz);
	if( rp == NULL) return NULL;

	return rp->rec;
}

void *Dll_GetFindFuncPtr( DLL *dll, char *find, int f_sz, int (*comp_func)( void *arg, void *rec, int sz))
{
	DLL_REC		*rp;

	rp = Dll_FindFunc( dll, find, f_sz, comp_func);
	if( rp == NULL) return NULL;

	return rp->rec;
}

int	Dll_Pop( DLL *dll, char *rec, int sz)
{
	int		cp_sz = sz;
	DLL_REC		*rp, *p;

	if( dll->recs <= 0) return 0;
	if( dll->last == NULL) return 0;

	rp = dll->last;
	p = rp->prev;
	if( p == NULL)
	{
		dll->last = NULL;
		dll->star = NULL;
		dll->curr = NULL;
	}
	else
	{
		p->next = NULL;
		dll->last = p;
		if( dll->star == rp) dll->star = p;
		if( dll->curr == rp) dll->curr = p;
		dll->recs--;
	}

	if( rp->sz < sz) 
	{
		cp_sz = rp->sz;
		rec[ cp_sz] = 0;
	}
	memcpy( rec, rp->rec, cp_sz);
	cp_sz = rp->sz;

	if( dll->opt == 1) free( rp->rec);
	free( rp);

	return cp_sz;
}

void *Dll_Del( DLL *dll)
{
	DLL_REC		*rp, *p, *n;

	if( dll->recs <= 0) return 0;
	if( dll->curr == NULL) return 0;

	rp = dll->curr;
	p = rp->prev;
	n = rp->next;


	if( p != NULL)	p->next = n;
	else			dll->star = n;
	if( n != NULL)	n->prev = p;
	else			dll->last = p;
	dll->recs--;

	if( n != NULL) 		dll->curr = n;
	else if( p != NULL) dll->curr = p;
	else				dll->curr = NULL;

	if( dll->opt == 1) free( rp->rec);
	free( rp);

	return dll->curr;
}

void *Dll_DelAll( DLL *dll)
{
	DLL_REC		*rp, *p;

	p = rp = dll->star;

	while( rp != NULL)
	{
		if( dll->opt == 1) free( rp->rec);
		rp = rp->next;
		free( p);
		p = rp;
	}

	dll->recs = 0;
	dll->star = NULL;
	dll->last = NULL;
	dll->curr = NULL;

	return NULL;
}

int	Dll_Print( DLL *dll)
{
	int			i = 1;
	char		curr[ 2] = " \0";
	DLL_REC		*rp;

	LogMsg( "=================================================");	

	rp = dll->star;

	while( rp != NULL)
	{
		if( rp == dll->curr)	curr[ 0] = '*';
		else					curr[ 0] = ' ';
		LogMsg( "%s%4d %p %6d %s", curr, i++, rp, rp->sz, rp->rec);
		rp = rp->next;
	}

	LogMsg( "-------------------------------------------------");	
	LogMsg( "dll pointer                = [%p]", dll);	
	LogMsg( "number of record(s)        = [%d]", dll->recs);	
	LogMsg( "dll option                 = [0x%08X]", dll->opt);	
	LogMsg( "=================================================");	

	return 1;
}

int	Dll_PrintFunc( DLL *dll, int(*func)( char *rec, void *data, int sz))
{
	int			i = 1, rtn;
	char		curr[ 2] = " \0";
	char		rec[ 512];
	DLL_REC		*rp;

	LogMsg( "=======================================================================");	
	LogMsg( "dll pointer                = [%p]", dll);	
	LogMsg( "number of record(s)        = [%d]", dll->recs);	
	LogMsg( "dll option                 = [0x%08X]", dll->opt);	
	LogMsg( "-----------------------------------------------------------------------");	

	rp = dll->star;

	while( rp != NULL)
	{
		if( rp == dll->curr)	curr[ 0] = '*';
		else					curr[ 0] = ' ';
		if( rp->rec != NULL) 	
		{
			rtn = func( rec, rp->rec, rp->sz);
			LogMsg( "%s%4d %p %s", curr, i++, rp, rec);
		}
		rp = rp->next;
	}

	LogMsg( "=======================================================================");	
	return 1;
}

int	Dll_Close( DLL *dll)
{
	DLL_REC		*rp, *p;

	p = rp = dll->star;

	while( rp != NULL)
	{
		if( dll->opt == 1) free( rp->rec);
		rp = rp->next;
		free( p);
		p = rp;
	}

	free( dll);

	return 1;
}


