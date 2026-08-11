#------------------------------------------------------------------------
#	Makefile for config-db-migration test
#	Location: st01/test/
#
#	Platforms: HP-UX, SunOS, AIX, Linux, Darwin (macOS)
#
#	Usage:
#	  make              # full test (build + import + export + verify)
#	  make build        # compile ini2db, db2ini
#	  make import       # ini2db: INI -> DB
#	  make export       # db2ini: DB -> INI
#	  make verify       # roundtrip comparison
#	  make dbcheck      # inspect DB contents
#	  make clean        # remove all test artifacts
#	  make ENV_ID=REAL1 # test with REAL1 environment
#------------------------------------------------------------------------

#==========================================================================
#	OS Detection (same pattern as pkg_env.sh)
#==========================================================================

OSNAME		:= $(shell uname -s 2>/dev/null)

#==========================================================================
#	Compiler / Flags per OS
#==========================================================================

CC		= cc

ifeq ($(OSNAME),HP-UX)
    CFLAGS	= -Ae +DAportable
    LDFLAGS	= -lsqlite3
endif

ifeq ($(OSNAME),SunOS)
    CFLAGS	=
    LDFLAGS	= -lsqlite3
endif

ifeq ($(OSNAME),AIX)
    CFLAGS	= -O -qcpluscmt
    LDFLAGS	= -lsqlite3
endif

ifeq ($(OSNAME),Linux)
    CFLAGS	= -Wall -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE
    LDFLAGS	= -lsqlite3
endif

ifeq ($(OSNAME),Darwin)
    CFLAGS	= -Wall
    LDFLAGS	= -lsqlite3
endif

#==========================================================================
#	Paths -- use FEP env vars if available, else relative
#==========================================================================

ifdef _P_UTL
    UTL_DIR	= $(_P_UTL)
else
    UTL_DIR	= ../utl
endif

ifdef _P_CFG
    CFG_DIR	= $(_P_CFG)
else
    CFG_DIR	= ../cfg
endif

ifdef _P_INC
    INC_DIR	= $(_P_INC)
else
    INC_DIR	= ../inc
endif

ifdef _P_LIB
    LIB_DIR	= $(_P_LIB)
else
    LIB_DIR	= ../lib
endif

#-- Environment auto-detect (from pkg_env.sh _FEP_DIV) --------------------
ifdef _FEP_DIV
    ENV_ID	?= $(_FEP_DIV)
else
    ENV_ID	?= TEST
endif

#-- Local paths ------------------------------------------------------------
BIN_DIR		= .
DB_FILE		= fep_config.db
EXPORT_DIR	= export

#-- Targets ----------------------------------------------------------------
INI2DB		= $(BIN_DIR)/ini2db
DB2INI		= $(BIN_DIR)/db2ini

#-- INI files to verify ----------------------------------------------------
INI_FILES	= daemon.ini file.ini dshm.ini tcp1.ini tcp2.ini \
		  udpip.ini sisetr.ini proc.ini
IP_FILES	= client.ini pc.ini

#==========================================================================
#	Primary targets
#==========================================================================

all: build import export verify
	@echo ""
	@echo "========================================"
	@echo "  All tests PASSED  ($(OSNAME), ENV=$(ENV_ID))"
	@echo "========================================"

test: all

#==========================================================================
#	Build
#==========================================================================

build: $(INI2DB) $(DB2INI)
	@echo "[BUILD] Complete ($(OSNAME), CC=$(CC))"

$(INI2DB): $(UTL_DIR)/ini2db.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "[BUILD] ini2db OK"

$(DB2INI): $(UTL_DIR)/db2ini.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "[BUILD] db2ini OK"

#==========================================================================
#	Import: INI -> DB
#==========================================================================

import: $(INI2DB)
	@rm -f $(DB_FILE)
	$(INI2DB) -e $(ENV_ID) -c $(CFG_DIR) -d $(DB_FILE)
	@test -f $(DB_FILE) || (echo "[FAIL] DB file not created" && exit 1)
	@echo "[IMPORT] Complete"

#==========================================================================
#	Export: DB -> INI
#==========================================================================

export: $(DB2INI) $(DB_FILE)
	@rm -rf $(EXPORT_DIR)
	@mkdir -p $(EXPORT_DIR)
	$(DB2INI) -e $(ENV_ID) -d $(DB_FILE) -o $(EXPORT_DIR)
	@echo "[EXPORT] Complete"

#==========================================================================
#	Verify: Roundtrip comparison
#==========================================================================

verify: export
	@echo ""
	@echo "========================================"
	@echo "  Roundtrip Verification"
	@echo "========================================"
	@FAIL=0; \
	for f in $(INI_FILES); do \
		ORIG=$$(grep -E '^[A-Za-z_0-9]+=' $(CFG_DIR)/$$f 2>/dev/null | tr '[:lower:]' '[:upper:]' | sort); \
		EXPD=$$(grep -E '^[A-Za-z_0-9]+=' $(EXPORT_DIR)/$$f 2>/dev/null | tr '[:lower:]' '[:upper:]' | sort); \
		if [ "$$ORIG" = "$$EXPD" ]; then \
			echo "  PASS  $$f"; \
		else \
			echo "  FAIL  $$f"; \
			FAIL=1; \
		fi; \
	done; \
	for f in $(IP_FILES); do \
		ORIG=$$(grep -E '^[0-9]' $(CFG_DIR)/$$f 2>/dev/null | sort); \
		EXPD=$$(grep -E '^[0-9]' $(EXPORT_DIR)/$$f 2>/dev/null | sort); \
		if [ "$$ORIG" = "$$EXPD" ]; then \
			echo "  PASS  $$f"; \
		else \
			echo "  FAIL  $$f"; \
			FAIL=1; \
		fi; \
	done; \
	echo "========================================"; \
	if [ $$FAIL -eq 1 ]; then \
		echo "  RESULT: FAIL"; \
		exit 1; \
	else \
		echo "  RESULT: ALL PASS (10/10 files)"; \
	fi

#==========================================================================
#	DB inspection
#==========================================================================

dbcheck: $(DB_FILE)
	@echo "=== Tables ==="
	@sqlite3 $(DB_FILE) ".tables"
	@echo ""
	@echo "=== Keys by Group ==="
	@sqlite3 -header -column $(DB_FILE) \
		"SELECT group_id, COUNT(*) as keys FROM fep_config WHERE env_id='$(ENV_ID)' GROUP BY group_id;"
	@echo ""
	@echo "=== Total ==="
	@sqlite3 $(DB_FILE) "SELECT COUNT(*) || ' config keys' FROM fep_config WHERE env_id='$(ENV_ID)';"
	@sqlite3 $(DB_FILE) "SELECT COUNT(*) || ' IP entries' FROM fep_ip_whitelist WHERE env_id='$(ENV_ID)';"
	@echo ""
	@echo "=== Environments ==="
	@sqlite3 -header -column $(DB_FILE) "SELECT * FROM fep_environment;"
	@echo ""
	@echo "=== DB Meta ==="
	@sqlite3 -header -column $(DB_FILE) "SELECT * FROM fep_db_meta;"

#==========================================================================
#	Dry-run (parse only, no DB)
#==========================================================================

dryrun: $(INI2DB)
	$(INI2DB) -e $(ENV_ID) -c $(CFG_DIR) --dry-run

#==========================================================================
#	cfg_verify -- server only (requires libfepP + SHM)
#==========================================================================

CFG_VERIFY	= $(BIN_DIR)/cfg_verify

cfgverify: $(CFG_VERIFY)
	@echo "[BUILD] cfg_verify OK (run with: ./cfg_verify -e $(ENV_ID) -d $(DB_FILE) -v)"

$(CFG_VERIFY): $(UTL_DIR)/cfg_verify.c
ifeq ($(OSNAME),Darwin)
	@echo "[SKIP] cfg_verify requires libfepP (server only)"
else
	$(CC) $(CFLAGS) -o $@ $< -I$(INC_DIR) -L$(LIB_DIR) -lfepP $(LDFLAGS)
	@echo "[BUILD] cfg_verify OK"
endif

#==========================================================================
#	Install: copy binaries to st01/bin/
#==========================================================================

ifdef _P_BIN
    INST_DIR = $(_P_BIN)
else
    INST_DIR = ../bin
endif

install: $(INI2DB) $(DB2INI)
	cp $(INI2DB) $(INST_DIR)/ini2db
	cp $(DB2INI) $(INST_DIR)/db2ini
	@echo "[INSTALL] ini2db, db2ini -> $(INST_DIR)"

#==========================================================================
#	Info: show detected environment
#==========================================================================

info:
	@echo "========================================"
	@echo "  Build Environment"
	@echo "========================================"
	@echo "  OS        : $(OSNAME)"
	@echo "  CC        : $(CC)"
	@echo "  CFLAGS    : $(CFLAGS)"
	@echo "  LDFLAGS   : $(LDFLAGS)"
	@echo "  ENV_ID    : $(ENV_ID)"
	@echo "  UTL_DIR   : $(UTL_DIR)"
	@echo "  CFG_DIR   : $(CFG_DIR)"
	@echo "  INC_DIR   : $(INC_DIR)"
	@echo "  LIB_DIR   : $(LIB_DIR)"
	@echo "  INST_DIR  : $(INST_DIR)"
	@echo "========================================"

#==========================================================================
#	Clean
#==========================================================================

clean:
	rm -f $(INI2DB) $(DB2INI) $(CFG_VERIFY) $(DB_FILE)
	rm -rf $(EXPORT_DIR)
	@echo "[CLEAN] Complete"

.PHONY: all test build import export verify dbcheck dryrun cfgverify install info clean
