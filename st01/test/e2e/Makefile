#------------------------------------------------------------------------
#	Makefile for E2E harness (F5 order-pipeline-dshm V-04/05/06)
#	Location: st01/test/e2e/
#
#	Usage:
#	  make            # build mock_oms + integ mock_krx_server
#	  make clean
#------------------------------------------------------------------------

CC	= cc
CFLAGS	= -O2 -Wall

all: mock_oms mock_krx

mock_oms: mock_oms.c
	$(CC) $(CFLAGS) -o mock_oms mock_oms.c
	@echo "==> mock_oms 생성 완료"

mock_krx:
	$(MAKE) -C ../integ phase1
	@echo "==> integ mock_krx_server 준비 완료"

clean:
	rm -f mock_oms
	rm -rf result

.PHONY: all clean mock_krx
