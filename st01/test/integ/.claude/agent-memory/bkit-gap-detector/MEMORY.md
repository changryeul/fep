# Gap Detector Memory

## Project: FEP (st01/test/integ)

### Key Reference Files for Struct Verification
- `st01/inc/pa_struct.h` -- KRX_HEADER (82B), KRX_BODY_COMMON (24B), KRX_MSG_COMMON (106B)
- `st01/inc/fep_file.h` -- FILE_RW_HEAD (84B), BUFF_RW_HEAD (70B)
- KRX message types used in production: SCHLIQ, SCHLIR, SCHOPQ, SCHOPR, SCHHEQ, SCHHER (found in src/PA/ and src/PB/)

### PB Integration Test Analysis (2026-03-01)
- Match Rate: 92% -- PASS
- All 16 deliverable files exist
- All struct sizes verified against production headers
- Main gap: `framework/` dir needs symlink to `../unit/framework`
- mk.sh INTEG branch confirmed at shl/mk.sh lines 41-43
- Unity test framework shared with unit test dir

### Build System Notes
- Makefile supports 5 OS targets: HP-UX, SunOS, AIX, Linux, Darwin
- Linux needs `-ltirpc`, SunOS needs `-lsocket -lnsl`
- sub/ sources compiled with `-I../../inc`
- Stubs resolve link dependencies for sub/ objects (Log, LtoU, UtoL, AtoIf)
