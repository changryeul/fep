#ifndef		__KEY_CODE_H
#define		__KEY_CODE_H
/*------------------------------------------------------------------------
#	Module	: common header files
#	File	: key_code.h
------------------------------------------------------------------------*/

#define LT   1
#define LE   2
#define EQ   3
#define GE   4
#define GT   5

/*------------------------------------------------------------------------
	Header Files
------------------------------------------------------------------------*/
#define KEY_EXPCODE 1
typedef struct _KS_EXPCODE {
    int      idx;                /* Data index                               */
    char     expcode    [12];    /* 종목표준코드                             */
} KS_EXPCODE;

#define KEY_SHCODE 2
typedef struct _KS_SHCODE {
    int      idx;                /* Data index                               */
    char     shcode     [ 9];    /* 종목단축코드                             */
} KS_SHCODE;

#define KEY_FUTCODE 3
typedef struct _KS_FUTCODE {
    int      idx;                /* Data index                               */
    char     futcode    [ 8];    /* 종목단축코드                             */
} KS_FUTCODE;

#define KEY_OPTCODE 4
typedef struct _KS_OPTCODE {
    int      idx;                /* Data index                               */
    char     optcode    [ 8];    /* 옵션 종목 단축코드                       */
} KS_OPTCODE;

#define KEY_UPJONGCODE 5
typedef struct _KS_UPJONGCODE {
    int      idx;                /* Data index                               */
    char     ujcode     [ 5];    /* 업종코드(TR2자리+업종코드3자리)          */
} KS_UPJONGCODE;

#define KEY_LONGCODE 6
typedef struct _KS_LONGCODE {
    int      idx;                /* Data index                               */
    char     longcode     [20];  /* 해외종목코드         */
} KS_LONGCODE;

/*************************************************************************
	End of Program (fep_fepp.h)
*************************************************************************/
#endif
