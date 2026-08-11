#------------------------------------------------------------------------
#	Makefile for utl/ utility tools
#	Location: st01/utl/
#
#	Usage:
#	  make                    # build all utilities
#	  make order_inject       # build order injection test tool only
#	  make clean              # remove binaries
#------------------------------------------------------------------------

CC		= cc
CFLAGS		= -O2 -Wall
BINDIR		= ../bin

#------------------------------------------------------------------------
#	Targets
#------------------------------------------------------------------------

all: order_inject lat_report

order_inject: order_inject.c
	$(CC) $(CFLAGS) -o $(BINDIR)/order_inject order_inject.c
	@echo "==> $(BINDIR)/order_inject 생성 완료"

lat_report: lat_report.c
	$(CC) $(CFLAGS) -o $(BINDIR)/lat_report lat_report.c
	@echo "==> $(BINDIR)/lat_report 생성 완료"

clean:
	rm -f $(BINDIR)/order_inject $(BINDIR)/lat_report

.PHONY: all clean order_inject lat_report
