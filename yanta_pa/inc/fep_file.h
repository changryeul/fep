#ifndef		__FEP_FILE_H
#define		__FEP_FILE_H
/*------------------------------------------------------------------------
#	Module	: common variables concerned in file read and write
#	File	: fep_file.h
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
	Constants and Structures
------------------------------------------------------------------------*/
/* SAM file flags - read	*/
#define		PS_R_FLAG	1		/* used for order (Add_Count () used)	*/
#define		PS_R_1		10				/* input file 1, read count 1	*/
#define		PS_R_2		11				/* input file 1, read count 2	*/
#define		PS_R_3		12				/* input file 1, read count 3	*/
#define		PS_R_4		13				/* input file 1, read count 4	*/
#define		PS_R_5		14				/* input file 1, read count 5	*/
#define		PS_R_6		15				/* input file 1, read count 6	*/
#define		PS_R_7		16				/* input file 1, read count 7	*/
#define		PS_R_8		17				/* input file 1, read count 8	*/
#define		PS_R_9		18				/* input file 1, read count 9	*/

#define		TS_R1_FLAG	2
#define		TS_R1_1		20				/* input file 1, read count 1	*/
#define		TS_R1_2		21				/* input file 1, read count 2	*/
#define		TS_R1_3		22				/* input file 1, read count 3	*/
#define		TS_R1_4		23				/* input file 1, read count 4	*/
#define		TS_R1_5		24				/* input file 1, read count 5	*/
#define		TS_R1_6		25				/* input file 1, read count 6	*/
#define		TS_R1_7		26				/* input file 1, read count 7	*/
#define		TS_R1_8		27				/* input file 1, read count 8	*/
#define		TS_R1_9		28				/* input file 1, read count 9	*/

#define		TS_R2_FLAG	3
#define		TS_R2_1		30				/* input file 2, read count 1	*/
#define		TS_R2_2		31				/* input file 2, read count 2	*/
#define		TS_R2_3		32				/* input file 2, read count 3	*/
#define		TS_R2_4		33				/* input file 2, read count 4	*/
#define		TS_R2_5		34				/* input file 2, read count 5	*/
#define		TS_R2_6		35				/* input file 2, read count 6	*/
#define		TS_R2_7		36				/* input file 2, read count 7	*/
#define		TS_R2_8		37				/* input file 2, read count 8	*/
#define		TS_R2_9		38				/* input file 2, read count 9	*/

#define		TS_R3_FLAG	4
#define		TS_R3_1		40				/* input file 3, read count 1	*/
#define		TS_R3_2		41				/* input file 3, read count 2	*/
#define		TS_R3_3		42				/* input file 3, read count 3	*/
#define		TS_R3_4		43				/* input file 3, read count 4	*/
#define		TS_R3_5		44				/* input file 3, read count 5	*/
#define		TS_R3_6		45				/* input file 3, read count 6	*/
#define		TS_R3_7		46				/* input file 3, read count 7	*/
#define		TS_R3_8		47				/* input file 3, read count 8	*/
#define		TS_R3_9		48				/* input file 3, read count 9	*/

/* SAM file flags - write   */
#define     TS_W1_FLAG  1
#define     TS_W1_1     10              /* output file 1, write count 1 */
#define     TS_W1_2     11              /* output file 1, write count 2 */
#define     TS_W1_3     12              /* output file 1, write count 3 */

#define     TS_W2_FLAG  2
#define     TS_W2_1     20              /* output file 2, write count 1 */
#define     TS_W2_2     21              /* output file 2, write count 2 */

#define     TS_W3_FLAG  3
#define     TS_W3_1     30              /* output file 3, write count 1 */
#define     TS_W3_2     31              /* output file 3, write count 2 */

#define     TS_W4_FLAG  4
#define     TS_W4_1     40              /* output file 4, write count 1 */
#define     TS_W4_2     41              /* output file 4, write count 2 */

#define     TS_W5_FLAG  5
#define     TS_W5_1     50              /* output file 5, write count 1 */
#define     TS_W5_2     51              /* output file 5, write count 2 */

#define     TS_W6_FLAG  6
#define     TS_W6_1     60              /* output file 6, write count 1 */
#define     TS_W6_2     61              /* output file 6, write count 2 */

#define     TS_W7_FLAG  7
#define     TS_W7_1     70              /* output file 7, write count 1 */
#define     TS_W7_2     71              /* output file 7, write count 2 */

#define     TS_W8_FLAG  8
#define     TS_W8_1     80              /* output file 8, write count 1 */
#define     TS_W8_2     81              /* output file 8, write count 2 */

#define     TS_W9_FLAG  9
#define     TS_W9_1     90              /* output file 9, write count 1 */
#define     TS_W9_2     91              /* output file 9, write count 2 */

#if defined ISAM_INCL
/* C-ISAM file flags	*/
#define		CISAM_R1	10							/* input c-isam 1	*/
#define		CISAM_R2	11							/* input c-isam 2	*/
#define		CISAM_R3	12							/* input c-isam 3	*/

#define		CISAM_W1	20							/* output c-isam 1	*/
#define		CISAM_W2	21							/* output c-isam 2	*/
#define		CISAM_W3	22							/* output c-isam 3	*/
#endif

/* file buffer header (50 + 20 = 70 bytes)	*/
typedef struct {
	char	Seq[8];					/* ÀÏ·Ã¹øÈ£							*/
	char	If_Seq[8];				/* Åë½ÅÀÏ·Ã¹øÈ£						*/
	char	ApType[8];				/* ¾÷¹«±¸ºÐ½Äº°ÀÚ					*/
	char	ResponseCode[4];   		/* ÀÀ´äÄÚµå							*/
	char	RecvTime1[10];			/* system ½Ã°¢ (nnnnnnnnnn (sec))	*/
	char	RecvTime2[12];			/* system ½Ã°¢ (HH:MM:SS-mmm)		*/
	char	DataHeader[20];			/* data header					    */
}	BUFF_RW_HEAD;

/* file header (62 + 22 = 84 bytes)	*/
typedef struct {
	char	Seq[8+2];				/* ÀÏ·Ã¹øÈ£							*/
	char	If_Seq[8+2];			/* Åë½ÅÀÏ·Ã¹øÈ£						*/
	char	ApType[8+2];			/* ¾÷¹«±¸ºÐ½Äº°ÀÚ					*/
	char	ResponseCode[4+2];		/* ÀÀ´äÄÚµå							*/
	char	RecvTime1[10+2];		/* system ½Ã°¢ (nnnnnnnnnn (sec))	*/
	char	RecvTime2[12+2];		/* system ½Ã°¢ (HH:MM:SS-mmm)		*/
	char	DataHeader[20+2];		/* data header					    */
}	FILE_RW_HEAD;

/* ½Ã¼¼ file header (8 + 12 = 20 bytes)	*/
typedef struct {
	char	Seq[8];
	char	RecvTime[12];
}	SISE_RW_HEAD;

#if defined ISAM_INCL
/* structures for C-ISAM */
typedef struct {
	char	BranchNo[3];									/* ÁöÁ¡¹øÈ£	*/
	char	OrderNo[7];										/* ÁÖ¹®¹øÈ£	*/
}	ISAM_KEY;

typedef struct {
	ISAM_KEY	Key;
	char		Data[70];
}	ISAM_DATA_FORMAT;
#endif

/* KB ¿¿¿¿¿¿(¿¿¿¿,¿¿¿ ¿¿) */
typedef struct {
	char	BodyLength[3];			// ¿¿¿¿¿¿(BodyLength ¿¿) ¿, ¿ pacek¿¿ 100¿¿ 97
	char	Channel[1];				// ¿¿ (E:¿¿, G:Algo)
	char	Commodity[1];			// ¿¿ (C:¿¿, A:¿¿¿¿, B:¿¿¿¿, D:FX) 
	char	FcmId[10];				// ¿¿/¿¿ ¿¿¿¿¿
	char	RuleNo[15];				// Algo¿¿
	char	ReferenceNo[15];		// Algo¿¿
	char	GroupNo[10];			// Algo¿¿
}	KB_IN_HEADER;

/*************************************************************************
	End of Program (fep_file.h)
*************************************************************************/
#endif
