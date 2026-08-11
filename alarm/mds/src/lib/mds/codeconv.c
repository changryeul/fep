#include "context.h"
#include "config.h"
#include "etcsvc.h"

#define	MAX_SYMBOL	1500
SYMINF	symlist[MAX_SYMBOL];
int		n_syminf;

//SYMINF *get_all_syminf(char *fnam)
int get_all_syminf(SYMINF *osymlist, char *fnam)
{
	int		i_exchange = -1, i_exchname = -1;
	int		i_symbol = -1, i_intsymbol = -1;
	int		i_ticksize = -1, i_tickvalue = -1, i_adjustvalue = -1;
	int		i_cabsize = -1, i_cabvalue = -1;
	int		i_priceindc = -1, i_priceadjust = -1;
	int		i_strikeindc = -1, i_strikeadjust1 = -1;
	int		i_strikeadjust2 = -1, i_strikeformat = -1;
	int		i_strikedivision = -1, i_optionstyle = -1;
	int		i_sector = -1, i_tradable = -1, i_currency = -1;
	int		i_name = -1;
	int		i_margin = -1;
	char	path_b[128];
	char	line_b[512];
	char	csv_b[32][128], chck_b[256];
	FILE	*csvF;
	int		csv_n;
	int		dodo = 0;
//	int		find_ok = 0;
	int		ii;
	SYMINF	*syminf, s;


	sprintf(path_b, "%s/%s", ETC_DIR, fnam);
	csvF = fopen(path_b, "r");
	if (csvF == NULL)
	{
		errno = ESRCH;
		return(-1);
	//	return NULL;
	}

	while ((fgets(line_b, sizeof(line_b), csvF)) == line_b)
	{
		chck_b[0] = '\0';
		sscanf(line_b, "%s", chck_b);
		switch (chck_b[0])
		{
		case '#':
		case '*':
		case '/':
		case '\0':
			continue;
		case '=':
			csv_n = csvform(&line_b[1], csv_b);
			if (csv_n <= 0)
				continue;
			for (ii = 0; ii < csv_n; ii++)
			{
				if (strcasecmp(csv_b[ii], "Exchange") == 0)
					i_exchange = ii;
				else if (strcasecmp(csv_b[ii], "ExchName") == 0)
					i_exchname = ii;
				else if (strcasecmp(csv_b[ii], "Symbol") == 0)
					i_symbol = ii;
				else if (strcasecmp(csv_b[ii], "IntSymbol") == 0)
					i_intsymbol = ii;
				else if (strcasecmp(csv_b[ii], "TickSize") == 0)
					i_ticksize = ii;
				else if (strcasecmp(csv_b[ii], "TickValue") == 0)
					i_tickvalue = ii;
				else if (strcasecmp(csv_b[ii], "AdjustValue") == 0)
					i_adjustvalue = ii;
				else if (strcasecmp(csv_b[ii], "CabSize") == 0)
					i_cabsize = ii;
				else if (strcasecmp(csv_b[ii], "CabValue") == 0)
					i_cabvalue = ii;
				else if (strcasecmp(csv_b[ii], "PriceIndc") == 0)
					i_priceindc = ii;
				else if (strcasecmp(csv_b[ii], "PriceAdjust") == 0)
					i_priceadjust = ii;
				else if (strcasecmp(csv_b[ii], "StrikeIndc") == 0)
					i_strikeindc = ii;
				else if (strcasecmp(csv_b[ii], "StrikeAdjust1") == 0)
					i_strikeadjust1 = ii;
				else if (strcasecmp(csv_b[ii], "StrikeAdjust2") == 0)
					i_strikeadjust2 = ii;
				else if (strcasecmp(csv_b[ii], "StrikeFormat") == 0)
					i_strikeformat = ii;
				else if (strcasecmp(csv_b[ii], "StrikeDivision") == 0)
					i_strikedivision = ii;
				else if (strcasecmp(csv_b[ii], "OptionStyle") == 0)
					i_optionstyle = ii;
				else if (strcasecmp(csv_b[ii], "Sector") == 0)
					i_sector = ii;
				else if (strcasecmp(csv_b[ii], "Tradable") == 0)
					i_tradable = ii;
				else if (strcasecmp(csv_b[ii], "Currency") == 0)
					i_currency = ii;
				else if (strcasecmp(csv_b[ii], "Name") == 0)
					i_name = ii;
				else if (strcasecmp(csv_b[ii], "Margin") == 0)
					i_margin = ii;
			}
			dodo = 1;
			continue;
		default:
			break;
		}
		if (!dodo)
			continue;
		memset(csv_b[0], 0x00, sizeof(csv_b));
		csv_n = csvform(line_b, csv_b);
		if (csv_n <= 0)
			continue;

		if (i_symbol == -1 || i_intsymbol == -1)
			continue;
			
		if (strlen(csv_b[i_symbol]) <= 0)	// no product symbol
			continue;

		if (strlen(csv_b[i_intsymbol]) <= 0)	// no internal symbol
			continue;

		syminf = (SYMINF *)&s;
		memset(&s, 0x00, sizeof(SYMINF));

        if (i_symbol != -1)
			strcpy(syminf->esym, csv_b[i_symbol]);  
        if (i_intsymbol != -1)
			strcpy(syminf->isym, csv_b[i_intsymbol]);  
        if (i_exchange != -1)
			strcpy(syminf->excd, csv_b[i_exchange]);  
        if (i_exchname != -1)
			strcpy(syminf->exnm, csv_b[i_exchname]);  
		if (i_ticksize != -1)
			syminf->tsiz = atof(csv_b[i_ticksize]);
		if (i_tickvalue != -1)
			syminf->tval = atof(csv_b[i_tickvalue]);
		if (i_adjustvalue != -1)
			syminf->adjv = atof(csv_b[i_adjustvalue]);
		if (i_cabsize != -1)
			syminf->csiz = atof(csv_b[i_cabsize]);
		if (i_cabvalue != -1)
			syminf->cval = atoi(csv_b[i_cabvalue]);
		if (i_priceindc != -1)
			syminf->pind = csv_b[i_priceindc][0];
			//syminf->pind = atoi(csv_b[i_priceindc]);
		if (i_priceadjust != -1)
			syminf->padj = atoi(csv_b[i_priceadjust]);
		if (i_strikeindc != -1)
			syminf->sind = atoi(csv_b[i_strikeindc]);
		if (i_strikeadjust1 != -1)
			syminf->sad1 = atoi(csv_b[i_strikeadjust1]);
		if (i_strikeadjust2 != -1)
			syminf->sad2 = atoi(csv_b[i_strikeadjust2]);
		if (i_strikeformat != -1)
			syminf->sfmt = atoi(csv_b[i_strikeformat]);
		if (i_strikedivision != -1)
			syminf->sdiv = atoi(csv_b[i_strikedivision]);
		if (i_optionstyle != -1)
			syminf->osty = atoi(csv_b[i_optionstyle]);
		if (i_sector != -1)
			syminf->sect = atoi(csv_b[i_sector]);
		if (i_tradable != -1)
			syminf->trdf = atoi(csv_b[i_tradable]);
		if (i_currency != -1)
			strcpy(syminf->curr, csv_b[i_currency]);
        if (i_name != -1)
			strcpy(syminf->enam, csv_b[i_name]);
        if (i_margin != -1)
			syminf->mrgn = atof(csv_b[i_margin]);

		memcpy(&symlist[n_syminf], syminf, sizeof(SYMINF));
		n_syminf++;
	}
	fclose(csvF);
	
	osymlist = symlist;
	return n_syminf;
}

int get_symbinf(SYMINF *osym, char *fnam) 
{
	int		i_exchange = -1, i_exchname = -1;
	int		i_symbol = -1, i_intsymbol = -1;
	int		i_ticksize = -1, i_tickvalue = -1, i_adjustvalue = -1;
	int		i_cabsize = -1, i_cabvalue = -1;
	int		i_priceindc = -1, i_priceadjust = -1;
	int		i_strikeindc = -1, i_strikeadjust1 = -1;
	int		i_strikeadjust2 = -1, i_strikeformat = -1;
	int		i_strikedivision = -1, i_optionstyle = -1;
	int		i_sector = -1, i_tradable = -1, i_currency = -1;
	int		i_name = -1;
	int		i_margin = -1;
	int		i_atmadjust = -1, i_strikeinterval = -1;
	char	path_b[128];
	char	line_b[512];
	char	csv_b[32][128], chck_b[256];
	FILE	*csvF;
	int		csv_n;
	int		dodo = 0;
	int		find_ok = 0;
	int		ii;
	SYMINF	sym, *syminf;

	//sprintf(path_b, "%s/information-symb.csv", ETC_DIR);
	sprintf(path_b, "%s/%s", ETC_DIR, fnam);
	csvF = fopen(path_b, "r");
	if (csvF == NULL)
	{
		errno = ESRCH;
		return(-1);
	}
	syminf = &sym;
	//memset(&sym, 0x00, sizeof(SYMINF));
	memcpy(syminf, osym, sizeof(SYMINF));
	while ((fgets(line_b, sizeof(line_b), csvF)) == line_b)
	{
		chck_b[0] = '\0';
		sscanf(line_b, "%s", chck_b);
		switch (chck_b[0])
		{
		case '#':
		case '*':
		case '/':
		case '\0':
			continue;
		case '=':
			csv_n = csvform(&line_b[1], csv_b);
			if (csv_n <= 0)
				continue;
			for (ii = 0; ii < csv_n; ii++)
			{
				if (strcasecmp(csv_b[ii], "Exchange") == 0)
					i_exchange = ii;
				else if (strcasecmp(csv_b[ii], "ExchName") == 0)
					i_exchname = ii;
				else if (strcasecmp(csv_b[ii], "Symbol") == 0)
					i_symbol = ii;
				else if (strcasecmp(csv_b[ii], "IntSymbol") == 0)
					i_intsymbol = ii;
				else if (strcasecmp(csv_b[ii], "TickSize") == 0)
					i_ticksize = ii;
				else if (strcasecmp(csv_b[ii], "TickValue") == 0)
					i_tickvalue = ii;
				else if (strcasecmp(csv_b[ii], "AdjustValue") == 0)
					i_adjustvalue = ii;
				else if (strcasecmp(csv_b[ii], "CabSize") == 0)
					i_cabsize = ii;
				else if (strcasecmp(csv_b[ii], "CabValue") == 0)
					i_cabvalue = ii;
				else if (strcasecmp(csv_b[ii], "PriceIndc") == 0)
					i_priceindc = ii;
				else if (strcasecmp(csv_b[ii], "PriceAdjust") == 0)
					i_priceadjust = ii;
				else if (strcasecmp(csv_b[ii], "StrikeIndc") == 0)
					i_strikeindc = ii;
				else if (strcasecmp(csv_b[ii], "StrikeAdjust1") == 0)
					i_strikeadjust1 = ii;
				else if (strcasecmp(csv_b[ii], "StrikeAdjust2") == 0)
					i_strikeadjust2 = ii;
				else if (strcasecmp(csv_b[ii], "StrikeFormat") == 0)
					i_strikeformat = ii;
				else if (strcasecmp(csv_b[ii], "StrikeDivision") == 0)
					i_strikedivision = ii;
				else if (strcasecmp(csv_b[ii], "OptionStyle") == 0)
					i_optionstyle = ii;
				else if (strcasecmp(csv_b[ii], "Sector") == 0)
					i_sector = ii;
				else if (strcasecmp(csv_b[ii], "Tradable") == 0)
					i_tradable = ii;
				else if (strcasecmp(csv_b[ii], "Currency") == 0)
					i_currency = ii;
				else if (strcasecmp(csv_b[ii], "Name") == 0)
					i_name = ii;
				else if (strcasecmp(csv_b[ii], "Margin") == 0)
					i_margin = ii;
				else if (strcasecmp(csv_b[ii], "AtmAdjust") == 0)
					i_atmadjust = ii;
				else if (strcasecmp(csv_b[ii], "StrikeInterval") == 0)
					i_strikeinterval = ii;
			}
			dodo = 1;
			continue;
		default:
			break;
		}
		if (!dodo)
			continue;
		memset(csv_b[0], 0x00, sizeof(csv_b));
		csv_n = csvform(line_b, csv_b);
		if (csv_n <= 0)
			continue;

		if (i_symbol == -1 || i_intsymbol == -1)
			continue;
			
		if (strlen(csv_b[i_symbol]) <= 0)	// no product symbol
			continue;

		if (strlen(csv_b[i_intsymbol]) <= 0)	// no internal symbol
			continue;

        if (i_exchange != -1)
			strcpy(syminf->excd, csv_b[i_exchange]);  
        if (i_exchname != -1)
			strcpy(syminf->exnm, csv_b[i_exchname]);  
		if (i_ticksize != -1)
			syminf->tsiz = atof(csv_b[i_ticksize]);
		if (i_tickvalue != -1)
			syminf->tval = atof(csv_b[i_tickvalue]);
		if (i_adjustvalue != -1)
			syminf->adjv = atof(csv_b[i_adjustvalue]);
		if (i_cabsize != -1)
			syminf->csiz = atof(csv_b[i_cabsize]);
		if (i_cabvalue != -1)
			syminf->cval = atoi(csv_b[i_cabvalue]);
		if (i_priceindc != -1)
			syminf->pind = csv_b[i_priceindc][0];
			//syminf->pind = atoi(csv_b[i_priceindc]);
		if (i_priceadjust != -1)
			syminf->padj = atoi(csv_b[i_priceadjust]);
		if (i_strikeindc != -1)
			syminf->sind = atoi(csv_b[i_strikeindc]);
		if (i_strikeadjust1 != -1)
			syminf->sad1 = atoi(csv_b[i_strikeadjust1]);
		if (i_strikeadjust2 != -1)
			syminf->sad2 = atoi(csv_b[i_strikeadjust2]);
		if (i_strikeformat != -1)
			syminf->sfmt = atoi(csv_b[i_strikeformat]);
		if (i_strikedivision != -1)
			syminf->sdiv = atoi(csv_b[i_strikedivision]);
		if (i_optionstyle != -1)
			syminf->osty = atoi(csv_b[i_optionstyle]);
		if (i_sector != -1)
			syminf->sect = atoi(csv_b[i_sector]);
		if (i_tradable != -1)
			syminf->trdf = atoi(csv_b[i_tradable]);
		if (i_currency != -1)
			strcpy(syminf->curr, csv_b[i_currency]);
        if (i_name != -1)
			strcpy(syminf->enam, csv_b[i_name]);
        if (i_margin != -1)
			syminf->mrgn = atof(csv_b[i_margin]);
        if (i_atmadjust != -1)
			syminf->atma = atof(csv_b[i_atmadjust]);
        if (i_strikeinterval != -1)
			syminf->sint = atof(csv_b[i_strikeinterval]);

		if (!strcmp(syminf->esym, csv_b[i_symbol]))
		{
			find_ok = 1;
			strcpy(syminf->isym, csv_b[i_intsymbol]);
			break;
		}
		if (!strcmp(syminf->isym, csv_b[i_intsymbol]))
		{
			find_ok = 1;
			strcpy(syminf->esym, csv_b[i_symbol]);
			break;
		}
	}
	fclose(csvF);
	
	if (find_ok == 1)
	{
		memcpy(osym, syminf, sizeof(SYMINF));
		return(0);
	}
	else
		return(-1);
}

//
// esym2isym()
// 거래소코드 ==> 내부코드 변환
// ESZ14 C2005 -> OESZ14_C2005
// LOZ4 C10050 -> OCLZ14_C100.5
//
int esym2isym(MDMSTR *imstr, char *osymb)
{

	return(0);
}

//
// isym2esym()
// 내부코드 ==> 거래소코드
//
int isym2esym(char *isymb, char *osymb, char *oexcd)
{
	SYMINF 	symi;
	char    *sptr;
	char    tmpb[32];
	char    code[2][32];
	char	form[32], sval[16], strk[16];
	char	moncd, corp;
	int		yy, ival;

	if ((sptr = strchr(isymb, '_')) == NULL)
		return(-1);
	memset(code, 0x00, sizeof(code));

	/* 기초자산코드(Ex : 6AU14) */
	sprintf(code[0], "%.*s", (int)(sptr-isymb), isymb);
	/* 행사가코드(Ex : C0770) */
	sprintf(code[1], "%s", sptr+1);

	/* Month코드 */
	moncd = code[0][strlen(code[0])-3];
	/* Call/Put 구분 */
	corp = code[1][0];

	/* 품목코드 변환(내부코드->거래소코드) */
	memset(&symi, 0, sizeof(SYMINF));
	/* 내부품목코드 */
	sprintf(symi.isym, "%.*s", (int)(strlen(code[0])-3), code[0]);
	if (get_symbinf(&symi, OPTSYMB_INFO_FILE) < 0)
		return(-1);

#if (0)
printf("%s> esym=%s sfmt=%d sad2=%d sdiv=%d\n", 
	__func__, symi.esym, symi.sfmt, symi.sad2, symi.sdiv);
#endif

	/* YEAR변환(14->4) */
	sprintf(tmpb, "%.2s", &code[0][strlen(code[0])-2]);
	yy = atoi(tmpb) % 10;

	/* Strike-Price변환 */
	sprintf(tmpb, "%s", &code[1][1]);
	sprintf(form, "%%0%dd", symi.sfmt);

//	ival = atof(tmpb)*symi.sad2;
    sprintf(sval, "%f", atof(tmpb)*symi.sad2);
    ival = atoi(sval);
	sprintf(strk, form, ival/symi.sdiv);
	
	/* 코드조합 */
	sprintf(osymb, "%s%c%d %c%s", symi.esym, moncd, yy, corp, strk);
	strcpy(oexcd, symi.excd);
//printf("osymb=%s \n", osymb);

	return(0);
}

//
// symbconv1()
// 코드 변환 
// ESZ4 -> ESZ14
void symbconv1(char *isymb, char *osymb)
{
	struct	tm ctm;
	time_t	tclock;
	char	tmpb[32], comd[16];
	char	moncd;
	int		year, yy;

	tclock = time(0);
	localtime_r(&tclock, &ctm);
	year = ctm.tm_year + 1900;

	tmpb[0] = comd[0] = '\0';
	strcpy(tmpb, isymb); 
	//l_rtrim(tmpb);
	sprintf(comd, "%.*s", (int)(strlen(tmpb)-2), tmpb);
	moncd = tmpb[strlen(tmpb)-2];
	yy = tmpb[strlen(tmpb)-1] - '0';

	if (yy < (year%10))
		yy = (((((year+1900)/10)*10)+10+yy)%100);
	else
		yy = (((((year+1900)/10)*10)+yy)%100);

	/* conversion symbol */
	sprintf(osymb, "%s%c%02d", comd, moncd, yy);
}

#if (0)
void symbconv2(char *isymb, char *osymb)
{
}
#endif

//
// get_loctz()
// Initialzie product symbol
//
int get_loctz(char *exch, char *tzon)
{
    char    path_b[128];
    char    line_b[512];
    FILE    *csvF;
    char    arg1[80], arg2[80];
    int     endf = 0;

    sprintf(path_b, "%s/TZONINFO.cfg", ETC_DIR);
    csvF = fopen(path_b, "r");
    if (csvF == NULL)
    {
        errno = ESRCH;
        return(-1);
    }
    while ((fgets(line_b, sizeof(line_b), csvF)) == line_b)
    {
        switch (line_b[0])
        {
        case '#':
        case '*':
        case '/':
        case '\0':
            continue;

        default:
            sscanf (line_b, "%s %s", arg1, arg2);
            if (strcmp(exch, arg1) == 0)
            {
                sprintf(tzon, "TZ=%s", arg2);
                endf = 1;
            }
            break;
        }

        if (endf)
            break;
    }
    fclose(csvF);
    return(0);
}

int	get_exid(char *excd)
{
	if (!strcmp (excd, "CME"))
		return	EXID_CME;
	else
	if (!strcmp (excd, "ECBOT"))
		return	EXID_CME;
	else
	if (!strcmp (excd, "EUREX"))
		return	EXID_EUREX;
	else
	if (!strcmp (excd, "SGX"))
		return	EXID_SGX;
	else
	if (!strcmp (excd, "HKE"))
		return	EXID_HKFE;
	else
	if (!strcmp (excd, "HKEX"))
		return	EXID_HKFE;
	else
	if (!strcmp (excd, "OSE"))
		return	EXID_OSE;
	else
	if (!strcmp (excd, "TSE"))
		return	EXID_TSE;
	else
	if (!strcmp (excd, "LIFFE"))
		return	EXID_LIFFE;
	else
	if (!strcmp (excd, "ICE"))
		return	EXID_ICE;
	else
	if (!strcmp (excd, "SFE"))
		return	EXID_SFE;
	else
		return	EXID_ETC;
}

//
// mds_setsyminf()
// Get symbol information
//
int mds_setsyminf(MARKET *market, MDMSTR *mstr, SYMINF *psymi)
{

	mstr->exid = get_exid(psymi->excd);
	mstr->pinc = psymi->tsiz;
	mstr->adjv = psymi->adjv;
	mstr->tval = psymi->tval;

	if (psymi->pind > 0) 
		mstr->pind = psymi->pind;
	else
	{
		switch (mstr->zdiv)
		{
		case 0:
		case 1: case 2: case 3:
		case 4: case 5: case 6:
		case 7: case 8: case 9:
			mstr->pind = (mstr->zdiv) + '1';
			break;
		default : mstr->pind = '1';
		}
	}

	if (psymi->aval > 0) mstr->aval = psymi->aval; 
	if (psymi->sect > 0) mstr->sect = psymi->sect; 
	if (psymi->trdf > 0) mstr->trdf = psymi->trdf; 

	return(0);
}

int get_diff_rate(double pric, double base, double *diff, double *rate)
{
	int		irate;

	*diff = pric - base;
	if (base != 0)
	{
		irate = (*diff/base) * 10000;
		*rate = (double)irate / 100;
	}
	else 
		*rate = 0;

	return(0);
}

int get_comd(char *isymb, char *comd)
{
	int	len;

	len = strlen(isymb);
	if (isdigit (isymb[len-2]))
		sprintf(comd, "%.*s", len-3, isymb);
	else
		sprintf(comd, "%.*s", len-2, isymb);

	return(0);
}

void *getfolder_leadmonth(MARKET *market, char *root)
{
	MDFOLD	*lf;
	WHERE	where;

	memset(&where, 0x00, sizeof(WHERE));
	mds_seekfolder(market, root, &where); 
	lf = where.l_folder;	
	if (lf != NULL)
		return(lf);
	else
		return(NULL);
}

void *getfolder_isym(MARKET *market, const char *symb)
{
	int	find = 0;
	MDFOLD	*folder;

	mds_setfolder(market, NULL);
	while ((folder = mds_popfolder(market)) != NULL)
	{
		if (strcmp(folder->mstr.isym, symb))
			continue;
		else
		{
			find = 1;
			break;
		}
	}

	if (find)
		return(folder);
	else
		return(NULL);
}
