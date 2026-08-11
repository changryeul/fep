/*------------------------------------------------------------------------
#   Module  : Minimal SHM stubs for integration tests
#   File    : stub_shm_minimal.c
#   Purpose : Provides fixed-value replacements for SHM-based macros
#             (TIME_OUT, Shm_FinFut, etc.) used by sub/ sources.
------------------------------------------------------------------------*/

/*------------------------------------------------------------------------
    SHM pointer stubs (referenced by key_search.c and others)
------------------------------------------------------------------------*/
long    Shm_FinFut[64]  = {0};
long    Shm_Item[64]    = {0};
long    Shm_Note[64]    = {0};

/*------------------------------------------------------------------------
    End of stub_shm_minimal.c
------------------------------------------------------------------------*/
