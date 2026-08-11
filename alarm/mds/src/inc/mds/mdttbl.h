#ifndef __MDTTBL_H__
#define __MDTTBL_H__

#define TTBL_GUBN_FUTRUE        "F"
#define TTBL_GUBN_SPREAD        "C"
#define TTBL_GUBN_OPTION        "O"

#define TTBL_CODE_FUTURE        "004"
#define TTBL_CODE_SPREAD        "119"
#define TTBL_CODE_CALL          "001"
#define TTBL_CODE_PUT           "002"

#define MAX_TABLE   100

typedef struct {
	char        unpd[SYMB_LEN];
	char        type[SYMB_LEN];
	char        gubn[4];        // "F" "C" "O"
	char        code[8];        // "004" "119" "001" "002"
	double      from;           // >= from
	double      to;             // < to
	double      size;           // Æ½»çÀÌÁî
} TICKTABLE;

typedef struct {
	int         nrec;
	TICKTABLE   table[MAX_TABLE];
} MDTTBL;

int init_ticktable(MARKET *market);

#endif
