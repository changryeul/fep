#------------------------------------------------------------------------
#	Makefile for FEP PB Integration Tests
#	Location: st01/test/integ/
#
#	Platforms: HP-UX, SunOS, AIX, Linux, Darwin (macOS)
#
#	Usage:
#	  make              # build all phases
#	  make phase1       # Phase 1: KRX protocol + TCP layer tests
#	  make phase2       # Phase 2: FIFO I/O tests
#	  make run          # build + run all tests
#	  make run1         # run Phase 1 tests (starts mock server)
#	  make run2         # run Phase 2 tests
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
    NET_LIBS	=
endif

ifeq ($(OSNAME),SunOS)
    CFLAGS	= $(UNITY_DEFS)
    LDFLAGS	=
    NET_LIBS	= -lsocket -lnsl
endif

ifeq ($(OSNAME),AIX)
    CFLAGS	= -O -qcpluscmt $(UNITY_DEFS)
    LDFLAGS	=
    NET_LIBS	=
endif

ifeq ($(OSNAME),Linux)
    CFLAGS	= -Wall -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE $(UNITY_DEFS)
    LDFLAGS	= -lm -lncurses
    NET_LIBS	= -ltirpc
endif

ifeq ($(OSNAME),Darwin)
    CFLAGS	= -Wall $(UNITY_DEFS)
    LDFLAGS	= -lncurses
    NET_LIBS	=
endif

#==========================================================================
#	Paths
#==========================================================================

SUB_DIR		= ../../sub
INC_DIR		= ../../inc
FW_DIR		= framework
LIB_DIR		= lib
MOCK_DIR	= mock
TOOLS_DIR	= tools
STUBS_DIR	= stubs
OBJ_DIR		= obj
BIN_DIR		= bin

# Include path for sub/ sources that need fep_sub.h
INC_FLAGS	= -I$(INC_DIR)

#==========================================================================
#	Build targets
#==========================================================================

PHASE1_TESTS	= $(BIN_DIR)/test_krx_protocol $(BIN_DIR)/test_tcp_layer
PHASE1_TOOLS	= $(BIN_DIR)/mock_krx_server

PHASE2_TESTS	= $(BIN_DIR)/test_fifo_rw
PHASE2_TOOLS	= $(BIN_DIR)/fifo_inject $(BIN_DIR)/fifo_verify

ALL_BINS	= $(PHASE1_TESTS) $(PHASE1_TOOLS) $(PHASE2_TESTS) $(PHASE2_TOOLS)

all: dirs phase1 phase2
	@echo "[BUILD] All integration test binaries OK ($(OSNAME))"

phase1: dirs $(PHASE1_TESTS) $(PHASE1_TOOLS)
	@echo "[BUILD] Phase 1 binaries OK"

phase2: dirs $(PHASE2_TESTS) $(PHASE2_TOOLS)
	@echo "[BUILD] Phase 2 binaries OK"

dirs:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)
	@test -e $(FW_DIR) || ln -s ../unit/framework $(FW_DIR)

#==========================================================================
#	Framework object
#==========================================================================

$(OBJ_DIR)/unity.o: $(FW_DIR)/unity.c $(FW_DIR)/unity.h $(FW_DIR)/unity_internals.h
	$(CC) $(CFLAGS) -c -o $@ $<

#==========================================================================
#	KRX Protocol Library
#==========================================================================

$(OBJ_DIR)/krx_protocol.o: $(LIB_DIR)/krx_protocol.c $(LIB_DIR)/krx_protocol.h
	$(CC) $(CFLAGS) -c -o $@ $<

#==========================================================================
#	Stubs
#==========================================================================

$(OBJ_DIR)/stub_fep_runtime.o: $(STUBS_DIR)/stub_fep_runtime.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/stub_shm_minimal.o: $(STUBS_DIR)/stub_shm_minimal.c
	$(CC) $(CFLAGS) -c -o $@ $<

#==========================================================================
#	Sub/ source objects (compiled with FEP include path)
#	We override the fep_fepp.h dependencies with our stubs at link time.
#==========================================================================

$(OBJ_DIR)/tcpip_sock.o: $(SUB_DIR)/tcpip_sock.c
	$(CC) $(CFLAGS) $(INC_FLAGS) -c -o $@ $<

$(OBJ_DIR)/tcpip_connect.o: $(SUB_DIR)/tcpip_connect.c
	$(CC) $(CFLAGS) $(INC_FLAGS) -c -o $@ $<

$(OBJ_DIR)/tcpip_send.o: $(SUB_DIR)/tcpip_send.c
	$(CC) $(CFLAGS) $(INC_FLAGS) -c -o $@ $<

$(OBJ_DIR)/tcpip_recv.o: $(SUB_DIR)/tcpip_recv.c
	$(CC) $(CFLAGS) $(INC_FLAGS) -c -o $@ $<

#==========================================================================
#	Test objects
#==========================================================================

$(OBJ_DIR)/test_krx_protocol.o: test_krx_protocol.c $(LIB_DIR)/krx_protocol.h
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_tcp_layer.o: test_tcp_layer.c $(LIB_DIR)/krx_protocol.h
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

$(OBJ_DIR)/test_fifo_rw.o: test_fifo_rw.c
	$(CC) $(CFLAGS) -I$(FW_DIR) -c -o $@ $<

#==========================================================================
#	Mock KRX server
#==========================================================================

$(OBJ_DIR)/mock_krx_server.o: $(MOCK_DIR)/mock_krx_server.c $(MOCK_DIR)/mock_krx_server.h $(LIB_DIR)/krx_protocol.h
	$(CC) $(CFLAGS) -I$(MOCK_DIR) -c -o $@ $(MOCK_DIR)/mock_krx_server.c

#==========================================================================
#	Tools
#==========================================================================

$(OBJ_DIR)/fifo_inject.o: $(TOOLS_DIR)/fifo_inject.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/fifo_verify.o: $(TOOLS_DIR)/fifo_verify.c
	$(CC) $(CFLAGS) -c -o $@ $<

#==========================================================================
#	Phase 1 Binaries
#==========================================================================

# test_krx_protocol: protocol lib + unity (no sub/ needed)
$(BIN_DIR)/test_krx_protocol: $(OBJ_DIR)/test_krx_protocol.o \
		$(OBJ_DIR)/krx_protocol.o \
		$(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

# test_tcp_layer: protocol lib + sub/tcpip + stubs + unity
$(BIN_DIR)/test_tcp_layer: $(OBJ_DIR)/test_tcp_layer.o \
		$(OBJ_DIR)/krx_protocol.o \
		$(OBJ_DIR)/tcpip_sock.o \
		$(OBJ_DIR)/tcpip_connect.o \
		$(OBJ_DIR)/tcpip_send.o \
		$(OBJ_DIR)/tcpip_recv.o \
		$(OBJ_DIR)/stub_fep_runtime.o \
		$(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS) $(NET_LIBS)

# mock_krx_server: protocol lib (standalone, no unity)
$(BIN_DIR)/mock_krx_server: $(OBJ_DIR)/mock_krx_server.o \
		$(OBJ_DIR)/krx_protocol.o
	$(CC) -o $@ $^ $(LDFLAGS) $(NET_LIBS)

#==========================================================================
#	Phase 2 Binaries
#==========================================================================

# test_fifo_rw: unity only (self-contained file I/O)
$(BIN_DIR)/test_fifo_rw: $(OBJ_DIR)/test_fifo_rw.o \
		$(OBJ_DIR)/unity.o
	$(CC) -o $@ $^ $(LDFLAGS)

# fifo_inject: standalone tool
$(BIN_DIR)/fifo_inject: $(OBJ_DIR)/fifo_inject.o
	$(CC) -o $@ $^

# fifo_verify: standalone tool
$(BIN_DIR)/fifo_verify: $(OBJ_DIR)/fifo_verify.o
	$(CC) -o $@ $^

#==========================================================================
#	Run targets
#==========================================================================

run: all run1 run2
	@echo ""
	@echo "========================================"
	@echo "  Integration Tests Complete"
	@echo "========================================"

run1: phase1
	@echo ""
	@echo "--- Phase 1: KRX Protocol Tests ---"
	@$(BIN_DIR)/test_krx_protocol
	@echo ""
	@echo "--- Phase 1: TCP Layer Tests (with mock server) ---"
	@$(BIN_DIR)/mock_krx_server 19999 &
	@sleep 1
	@$(BIN_DIR)/test_tcp_layer; RT=$$?; \
	 kill %1 2>/dev/null; wait %1 2>/dev/null; \
	 exit $$RT

run2: phase2
	@echo ""
	@echo "--- Phase 2: FIFO I/O Tests ---"
	@$(BIN_DIR)/test_fifo_rw
	@echo ""
	@echo "--- Phase 2: FIFO Tools Smoke Test ---"
	@rm -f /tmp/fep_integ_fifo_smoke
	@$(BIN_DIR)/fifo_inject /tmp/fep_integ_fifo_smoke "SMOKE_TEST_DATA"
	@$(BIN_DIR)/fifo_verify /tmp/fep_integ_fifo_smoke "SMOKE_TEST"
	@rm -f /tmp/fep_integ_fifo_smoke
	@echo "[SMOKE] FIFO tools OK"

#==========================================================================
#	Clean
#==========================================================================

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(BIN_DIR)/*
	rm -rf /tmp/fep_integ_test
	@echo "[CLEAN] Complete"

.PHONY: all phase1 phase2 dirs run run1 run2 clean
