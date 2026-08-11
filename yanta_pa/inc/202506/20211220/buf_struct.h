#ifndef		__BUF_STRUCT_H
#define		__BUF_STRUCT_H
/*------------------------------------------------------------------------
#	Module	: common structures of file format
#	File	: buf_struct.h
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
/* file buffer format (50 + 20 + DATA_SIZE + 1 bytes)	*/
typedef struct {
	char	Seq[8];					/* 일련번호							*/
	char	If_Seq[8];				/* 통신일련번호						*/
	char	ApType[8];				/* 업무구분식별자					*/
	char	ResponseCode[4];   		/* 응답코드							*/
	char	RecvTime1[10];			/* system 시각 (nnnnnnnnnn (sec))	*/
	char	RecvTime2[12];			/* system 시각 (HHMMSSmmmmmm)		*/
	char	DataHeader[20];			/* data header						*/
	char	Data[DATA_SIZE];		/* data								*/
	char	LineFeed[1];			/* line feed (0x0a)					*/
}	FILE_BUFF_FORMAT;

typedef struct {
	int					seq;
	FILE_BUFF_FORMAT	buf;
}	DATA_FORMAT;

typedef struct {
	char	Seq[8];
	char	RecvTime[12];
	char	Data[DATA_SIZE];
	char	LineFeed[1];
}	SISE_BUFF_FORMAT;

/* 62 + 72 + DATA_SIZE + 1	*/
typedef struct {
	char	Seq[8+2];				/* 일련번호							*/
	char	If_Seq[8+2];			/* 통신일련번호						*/
	char	ApType[8+2];			/* 업무구분식별자					*/
	char	ResponseCode[4+2];		/* 응답코드							*/
	char	RecvTime1[10+2];		/* system 시각 (nnnnnnnnnn (sec))	*/
	char	RecvTime2[12+2];		/* system 시각 (HH:MM:SS-mmm)		*/
	char	DataHeader[20+2];		/* data header						*/
	char	Data[DATA_SIZE];		/* data								*/
	char	LineFeed[1];			/* line feed (0x0a)					*/
}	FILE_READ_WRITE_FORMAT;

/*************************************************************************
	End of Program (buf_struct.h)
*************************************************************************/
#endif

