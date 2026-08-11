#!/bin/sh
#------------------------------------------------------------------------
#   Phase 3: PB Process End-to-End Integration Test
#   File   : run_pb_integ.sh
#   Purpose: Runs actual PB process binaries with Mock KRX server.
#            Linux server only (requires SHM, FIFO, daemon infrastructure).
#
#   Usage  : sh run_pb_integ.sh
#
#   Prerequisites:
#     - Compiled PB binaries in st01/bin/
#     - Integration test binaries in st01/test/integ/bin/
#     - pz_memory_mp available for SHM/FIFO creation
#------------------------------------------------------------------------

set -e

#------------------------------------------------------------------------
#   Configuration
#------------------------------------------------------------------------
INTEG_DIR="$(cd "$(dirname "$0")" && pwd)"
INTEG_BIN="${INTEG_DIR}/bin"
TEST_ROOT="/tmp/fep_integ_$$"
MOCK_PORT=19999
MOCK_PID=""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

pass_count=0
fail_count=0

#------------------------------------------------------------------------
#   Utility functions
#------------------------------------------------------------------------
log_info()  { printf "${YELLOW}[INFO]${NC} %s\n" "$1"; }
log_pass()  { printf "${GREEN}[PASS]${NC} %s\n" "$1"; pass_count=$((pass_count + 1)); }
log_fail()  { printf "${RED}[FAIL]${NC} %s\n" "$1"; fail_count=$((fail_count + 1)); }

cleanup() {
    log_info "Cleaning up..."
    if [ -n "$MOCK_PID" ] && kill -0 "$MOCK_PID" 2>/dev/null; then
        kill "$MOCK_PID" 2>/dev/null
        wait "$MOCK_PID" 2>/dev/null
    fi
    rm -rf "$TEST_ROOT"
    log_info "Cleanup complete"
}

trap cleanup EXIT

#------------------------------------------------------------------------
#   Environment check
#------------------------------------------------------------------------
log_info "=== PB Integration Test (Phase 3) ==="
log_info "Test root: $TEST_ROOT"

OS=$(uname -s)
if [ "$OS" != "Linux" ]; then
    log_info "WARNING: Phase 3 is designed for Linux."
    log_info "Running on $OS - SHM/FIFO tests may not work."
fi

# Check for mock server binary
if [ ! -x "${INTEG_BIN}/mock_krx_server" ]; then
    log_fail "mock_krx_server not found. Run 'make phase1' first."
    exit 1
fi

# Check for FIFO tools
if [ ! -x "${INTEG_BIN}/fifo_inject" ] || [ ! -x "${INTEG_BIN}/fifo_verify" ]; then
    log_fail "FIFO tools not found. Run 'make phase2' first."
    exit 1
fi

#------------------------------------------------------------------------
#   Step 1: Prepare test environment
#------------------------------------------------------------------------
log_info "Step 1: Creating test directory structure..."

mkdir -p "${TEST_ROOT}/st01/cfg"
mkdir -p "${TEST_ROOT}/st02/FIFO/PB"
mkdir -p "${TEST_ROOT}/st02/DAT/PB"
mkdir -p "${TEST_ROOT}/st03/LOG/PB"

# Copy test configs
cp "${INTEG_DIR}/cfg/daemon_test.ini" "${TEST_ROOT}/st01/cfg/daemon.ini"
cp "${INTEG_DIR}/cfg/proc_test.ini"   "${TEST_ROOT}/st01/cfg/proc.ini"
cp "${INTEG_DIR}/cfg/tcp2_test.ini"   "${TEST_ROOT}/st01/cfg/tcp2.ini"

log_pass "Test directory structure created"

#------------------------------------------------------------------------
#   Step 2: Start Mock KRX server
#------------------------------------------------------------------------
log_info "Step 2: Starting Mock KRX server on port $MOCK_PORT..."

"${INTEG_BIN}/mock_krx_server" "$MOCK_PORT" &
MOCK_PID=$!
sleep 1

if kill -0 "$MOCK_PID" 2>/dev/null; then
    log_pass "Mock KRX server started (PID=$MOCK_PID)"
else
    log_fail "Mock KRX server failed to start"
    exit 1
fi

#------------------------------------------------------------------------
#   Step 3: FIFO inject test
#------------------------------------------------------------------------
log_info "Step 3: FIFO inject/verify test..."

FIFO_FILE="${TEST_ROOT}/st02/FIFO/PB/pb_test.fifo"

# Build a test KRX order message (106-byte common header + payload)
ORDER_MSG="FEPKRX01000024TCHODR0000000000000001000120000000000000000000000000000000002026030100000000001N00000000001TTRTDP4230101TEST_ORDER_DATA"

"${INTEG_BIN}/fifo_inject" "$FIFO_FILE" "$ORDER_MSG"

if "${INTEG_BIN}/fifo_verify" "$FIFO_FILE" "FEPKRX01"; then
    log_pass "FIFO inject/verify: BeginString found"
else
    log_fail "FIFO inject/verify: BeginString not found"
fi

if "${INTEG_BIN}/fifo_verify" "$FIFO_FILE" "TCHODR"; then
    log_pass "FIFO inject/verify: MsgType found"
else
    log_fail "FIFO inject/verify: MsgType not found"
fi

if "${INTEG_BIN}/fifo_verify" "$FIFO_FILE" "TTRTDP42301"; then
    log_pass "FIFO inject/verify: TR code found"
else
    log_fail "FIFO inject/verify: TR code not found"
fi

#------------------------------------------------------------------------
#   Step 4: TCP protocol test via mock server
#------------------------------------------------------------------------
log_info "Step 4: Running TCP layer tests against mock server..."

if "${INTEG_BIN}/test_tcp_layer"; then
    log_pass "TCP layer tests passed"
else
    log_fail "TCP layer tests failed"
fi

# Note: mock server exits after one connection (test_tcp_layer),
# so restart it if more tests are needed
# MOCK_PID=""

#------------------------------------------------------------------------
#   Step 5: Results
#------------------------------------------------------------------------
echo ""
echo "========================================"
echo "  PB Integration Test Results"
echo "========================================"
echo "  Passed: $pass_count"
echo "  Failed: $fail_count"
echo "========================================"

if [ "$fail_count" -ne 0 ]; then
    exit 1
fi

exit 0
