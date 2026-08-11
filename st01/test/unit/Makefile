#------------------------------------------------------------------------
#	Makefile for FEP unit tests (Unity framework)
#	Location: st01/test/unit/
#
#	Platforms: HP-UX, SunOS, AIX, Linux, Darwin (macOS)
#
#	Usage:
#	  make              # build + run all tests
#	  make build        # compile only
#	  make run          # run all test binaries
#	  make clean        # remove obj/, bin/ contents
#------------------------------------------------------------------------

#==========================================================================
#	OS Detection
#==========================================================================

OSNAME		:= $(shell uname -s 2>/dev/null)

#==========================================================================
#	Compiler / Flags per OS
#==========================================================================

CC		= cc

UNITY_DEFS	= -DUNITY_INCLUDE_DOUBLE

ifeq ($(OSNAME),HP-UX)
    CFLAGS	= -Ae +DAportable $(UNITY_DEFS)
    LDFLAGS	=
endif

ifeq ($(OSNAME),SunOS)
    CFLAGS	= $(UNITY_DEFS)
    LDFLAGS	=
endif

ifeq ($(OSNAME),AIX)
    CFLAGS	= -O -qcpluscmt $(UNITY_DEFS)
    LDFLAGS	=
endif

ifeq ($(OSNAME),Linux)
    CFLAGS	= -Wall -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE $(UNITY_DEFS)
    LDFLAGS	= -lm -lncurses
endif

ifeq ($(OSNAME),Darwin)
    CFLAGS	= -Wall $(UNITY_DEFS)
    LDFLAGS	= -lncurses
endif

#==========================================================================
#	Paths
#==========================================================================

SUB_DIR		= ../../sub
INC_DIR		= ../../inc
FW_DIR		= framework
OBJ_DIR		= obj
BIN_DIR		= bin

#==========================================================================
#	Test list
#==========================================================================

TESTS	= $(BIN_DIR)/test_atoif \
	  $(BIN_DIR)/test_atolf \
	  $(BIN_DIR)/test_atodf \
	  $(BIN_DIR)/test_itoaf \
	  $(BIN_DIR)/test_ltou \
	  $(BIN_DIR)/test_utol \
	  $(BIN_DIR)/test_chk_kor \
	  $(BIN_DIR)/test_bsearch \
	  $(BIN_DIR)/test_ip_format \
	  $(BIN_DIR)/test_lat_trace \
	  $(BIN_DIR)/test_lat_report \
	  $(BIN_DIR)/test_log_hot_shm \
	  $(BIN_DIR)/test_log_hot_default \
	  $(BIN_DIR)/test_seq_throttle_default \
	  $(BIN_DIR)/test_seq_throttle_on \
	  $(BIN_DIR)/test_fd_cache \
	  $(BIN_DIR)/test_miche_idx \
	  $(BIN_DIR)/test_seam_queue \
	  $(BIN_DIR)/test_strategy_letter \
	  $(BIN_DIR)/test_krx_struct

#==========================================================================
#	Primary targets
#==========================================================================

all: dirs build run

build: dirs $(TESTS)
	@echo "[BUILD] All test binaries OK ($(OSNAME))"

dirs:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

#==========================================================================
#	Framework object
#==========================================================================

$(OBJ_DIR)/unity.o: $(FW_DIR)/unity.c $(FW_DIR)/unity.h $(FW_DIR)/unity_internals.h
	$(CC) $(CFLAGS) -c -o $@ $<

#==========================================================================
#	Tier 1A: No header needed (pure functions, no includes)
#==========================================================================

$(OBJ_DIR)/atoif.o: $(SUB_DIR)/atoif.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/atolf.o: $(SUB_DIR)/atolf.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/atodf.o: $(SUB_DIR)/atodf.c
	$(CC) $(CFLAGS) -c -o $@ $<

#==========================================================================
#	Tier 1B: Need -I$(INC_DIR) for fep_sub.h / fep_fepp.h
#==========================================================================

$(OBJ_DIR)/itoaf.o: $(SUB_DIR)/itoaf.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c -o $@ $<

$(OBJ_DIR)/ltou.o: $(SUB_DIR)/ltou.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c -o $@ $<

$(OBJ_DIR)/utol.o: $(SUB_DIR)/utol.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c -o $@ $<

$(OBJ_DIR)/chk_kor.o: $(SUB_DIR)/chk_kor.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c -o $@ $<

$(OBJ_DIR)/key_search.o: $(SUB_DIR)/key_search.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c -o $@ $<

$(OBJ_DIR)/ip_format.o: $(SUB_DIR)/ip_format.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c -o $@ $<

$(OBJ_DIR)/lat_trace.o: $(SUB_DIR)/lat_trace.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c -o $@ $<

#==========================================================================
#	Stubs (for SHM globals referenced by Key_Search but never called)
#==========================================================================

$(OBJ_DIR)/stub_shm.o: stubs/stub_shm.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/stub_log.o: stubs/stub_log.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/log_hot.o: $(SUB_DIR)/log_hot.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/seq_throttle.o: $(SUB_DIR)/seq_throttle.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/fd_cache.o: $(SUB_DIR)/fd_cache.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/miche_idx.o: $(SUB_DIR)/miche_idx.c $(INC_DIR)/miche_idx.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -c -o $@ $<

$(OBJ_DIR)/seam_ring.o: $(SUB_DIR)/seam_ring.c $(INC_DIR)/seam_queue.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -c -o $@ $<

$(OBJ_DIR)/strategy_letter.o: $(SUB_DIR)/strategy_letter.c $(INC_DIR)/strategy_letter.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -c -o $@ $<

#==========================================================================
#	Test objects (only need unity.h + function prototypes)
#==========================================================================

$(OBJ_DIR)/test_atoif.o: test_atoif.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_atolf.o: test_atolf.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_atodf.o: test_atodf.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_itoaf.o: test_itoaf.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_ltou.o: test_ltou.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_utol.o: test_utol.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_chk_kor.o: test_chk_kor.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_bsearch.o: test_bsearch.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_ip_format.o: test_ip_format.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_lat_trace.o: test_lat_trace.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_lat_report.o: test_lat_report.c ../../utl/lat_report.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -I../../utl -DLAT_REPORT_NO_MAIN -c -o $@ $<

$(OBJ_DIR)/test_log_hot_shm.o: test_log_hot_shm.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_log_hot_default.o: test_log_hot_default.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_seq_throttle_default.o: test_seq_throttle_default.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_seq_throttle_on.o: test_seq_throttle_on.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_fd_cache.o: test_fd_cache.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_miche_idx.o: test_miche_idx.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -I$(INC_DIR) -c -o $@ $<

$(OBJ_DIR)/test_seam_queue.o: test_seam_queue.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -I$(INC_DIR) -c -o $@ $<

$(OBJ_DIR)/test_strategy_letter.o: test_strategy_letter.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -I$(INC_DIR) -c -o $@ $<

$(OBJ_DIR)/test_krx_struct.o: test_krx_struct.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -I$(INC_DIR) -c -o $@ $<

#==========================================================================
#	Test binaries (link test + SUT + unity)
#==========================================================================

$(BIN_DIR)/test_atoif: $(OBJ_DIR)/test_atoif.o $(OBJ_DIR)/atoif.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_atolf: $(OBJ_DIR)/test_atolf.o $(OBJ_DIR)/atolf.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_atodf: $(OBJ_DIR)/test_atodf.o $(OBJ_DIR)/atodf.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_itoaf: $(OBJ_DIR)/test_itoaf.o $(OBJ_DIR)/itoaf.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_ltou: $(OBJ_DIR)/test_ltou.o $(OBJ_DIR)/ltou.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_utol: $(OBJ_DIR)/test_utol.o $(OBJ_DIR)/utol.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_chk_kor: $(OBJ_DIR)/test_chk_kor.o $(OBJ_DIR)/chk_kor.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_bsearch: $(OBJ_DIR)/test_bsearch.o $(OBJ_DIR)/key_search.o $(OBJ_DIR)/stub_shm.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_ip_format: $(OBJ_DIR)/test_ip_format.o $(OBJ_DIR)/ip_format.o $(OBJ_DIR)/atoif.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_lat_trace: $(OBJ_DIR)/test_lat_trace.o $(OBJ_DIR)/lat_trace.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_lat_report: $(OBJ_DIR)/test_lat_report.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_log_hot_shm: $(OBJ_DIR)/test_log_hot_shm.o $(OBJ_DIR)/log_hot.o $(OBJ_DIR)/stub_log.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_log_hot_default: $(OBJ_DIR)/test_log_hot_default.o $(OBJ_DIR)/log_hot.o $(OBJ_DIR)/stub_log.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_seq_throttle_default: $(OBJ_DIR)/test_seq_throttle_default.o $(OBJ_DIR)/seq_throttle.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_seq_throttle_on: $(OBJ_DIR)/test_seq_throttle_on.o $(OBJ_DIR)/seq_throttle.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_fd_cache: $(OBJ_DIR)/test_fd_cache.o $(OBJ_DIR)/fd_cache.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_miche_idx: $(OBJ_DIR)/test_miche_idx.o $(OBJ_DIR)/miche_idx.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_seam_queue: $(OBJ_DIR)/test_seam_queue.o $(OBJ_DIR)/seam_ring.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_strategy_letter: $(OBJ_DIR)/test_strategy_letter.o $(OBJ_DIR)/strategy_letter.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_krx_struct: $(OBJ_DIR)/test_krx_struct.o $(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

#==========================================================================
#	Run all tests
#==========================================================================

run: $(TESTS)
	@PASS=0; FAIL=0; TOTAL=0; \
	for t in $(TESTS); do \
		echo ""; \
		echo "--- Running $$t ---"; \
		if $$t; then \
			PASS=$$((PASS + 1)); \
		else \
			FAIL=$$((FAIL + 1)); \
		fi; \
		TOTAL=$$((TOTAL + 1)); \
	done; \
	echo ""; \
	echo "========================================"; \
	echo "  Unit Tests: $$PASS passed, $$FAIL failed ($$TOTAL total)"; \
	echo "========================================"; \
	if [ $$FAIL -ne 0 ]; then exit 1; fi

#==========================================================================
#	Clean
#==========================================================================

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(BIN_DIR)/test_*
	@echo "[CLEAN] Complete"

.PHONY: all build dirs run clean
