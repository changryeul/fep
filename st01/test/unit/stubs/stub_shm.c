/*------------------------------------------------------------------------
#   Stub  : SHM global variables for key_search.o linker satisfaction
#   File  : stubs/stub_shm.c
#
#   Key_Search() references Shm_FinFut, Shm_Item, Shm_Note but is NOT
#   tested (requires real SHM). Only BsearchMode/CmpExpcode/CmpLongcode
#   are tested. These stubs exist solely to satisfy the linker.
------------------------------------------------------------------------*/

/* Minimal stubs - never actually accessed */
long Shm_FinFut[64];
long Shm_Item[64];
long Shm_Note[64];
