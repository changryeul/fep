#ifndef     __CLI_INTERFACE_H
#define     __CLI_INTERFACE_H
/*------------------------------------------------------------------------
#   System  : PK System
#   Author  : P.S.H
#   Module  : structures - For CLI/IP Socket
#   File    : tcp_interface.h
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    Constants and Structures
------------------------------------------------------------------------*/
#define CLI_BUFF_MAX_LEN        4096

#define CLI_HEAD_LEN            (sizeof (CLI_HEAD))         /*   50 */
#define CLI_DATA_LEN			500

typedef struct {
        char    Length[4];          /* Length Field             */
        char    MsgType[4];         /* Message Type             */
                            /* LINK/LIOK, LIVE/LIVE, DATA/DAOK, EROR */
        char    ResponsCode[4];     /* OK:0000, NOTOK : !0000   */
        char    TradeDate[8];       /* Trade Data   '20200101'  */
        char    SeqNo[8];           /* Sequence number          */
        char    Filler[22];         /* Space                    */
}   CLI_HEAD;

/* 20210514 */
typedef struct {
	CLI_HEAD	Head;
	char		Data[CLI_DATA_LEN];
}	CLI_FORMAT;

/*************************************************************************
    End of Program (interface_struct.h)
*************************************************************************/
#endif

