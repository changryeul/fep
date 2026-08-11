#include <stdio.h>
#include "log.h"

#include "sise.h"

int SISE_ENTRY_Print( SISE_ENTRY* ptr)
{
    LogRaw( "%s", "----[ SISE_ENTRY ]----------------------------------------------------------------------\n");
    LogRaw( "0=bid,1=offer                 MDEntry_Type           2    0 = [%.2s]\n", 	ptr->MDEntry_Type);
    LogRaw( "Price                         MDEntry_Px            16    2 = [%.16s]\n", 	ptr->MDEntry_Px);
    LogRaw( "Quantity                      MDEntry_Size          16   18 = [%.16s]\n", 	ptr->MDEntry_Size);
    LogRaw( "date                          MDEntry_Date           8   34 = [%.8s]\n", 	ptr->MDEntry_Date);
    LogRaw( "Unique identifier             Quote_EntryID         32   42 = [%.32s]\n", 	ptr->Quote_EntryID);
    LogRaw( "Always SP (SPOT) Tenor        Sett_Type              2   74 = [%.2s]\n", 	ptr->Sett_Type);
    LogRaw( "Best Price                    MDBest_Px             16   76 = [%.16s]\n", 	ptr->MDBest_Px);
    LogRaw( "Best Quantity                 MDBest_Size           16   92 = [%.16s]\n", 	ptr->MDBest_Size);
    LogRaw( "%s", "----------------------------------------------------------------------[ SISE_ENTRY ]----\n");

    return sizeof( SISE_ENTRY);
}

int FX_QUOTE_T_Print( FX_QUOTE_T* ptr)
{
    LogRaw( "%s", "----[ FX_QUOTE_T ]----------------------------------------------------------------------\n");
    LogRaw( "Message type.                 Msg_Type               4    0 = [%.4s]\n", 	ptr->Msg_Type);
    LogRaw( "Message sender identifie      Sender_CompID         15    4 = [%.15s]\n", 	ptr->Sender_CompID);
    LogRaw( "Sending time (GMT). Time      Sending_Time          24   19 = [%.24s]\n", 	ptr->Sending_Time);
    LogRaw( "Primary Currency/Counter      Symb                   8   43 = [%.8s]\n", 	ptr->Symb);
    LogRaw( "Number of entries in th       No_MDEntries           4   51 = [%.4s]\n", 	ptr->No_MDEntries);
    SISE_ENTRY_Print( &ptr->entry[0]);
    SISE_ENTRY_Print( &ptr->entry[1]);
    LogRaw( "%s", "----------------------------------------------------------------------[ FX_QUOTE_T ]----\n");

    return sizeof( FX_QUOTE_T);
}

