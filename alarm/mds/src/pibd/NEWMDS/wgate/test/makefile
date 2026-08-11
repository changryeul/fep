#*******************************************************************************
#* (C) COPYRIGHT Winway Co., Ltd. 2009
#* All Rights Reserved
#* Licensed Materials - Property of Winway
#*
#* This program contains proprietary information of Winway System.
#* All embodying confidential information, ideas and expressions can't be
#* reproceduced, or transmitted in any form or by any means, electronic,
#* mechanical, or otherwise without the written permission of Winway System.
#*
#*  Components   : makefile	--
#*  Rev. History :
#*		Ver		Date	Description
#*		-------	-------	-----------------------------------------------
#*		1.00	2009-12	Winway initial version. (jason)
#*******************************************************************************
include /fsfxwin/wfg/env/wfg.mak
SHELL= /bin/sh

INCS= -I$(AGP_INC) -I$(AXS_INC) -I/fsfxwin/win/src/inc/com -I/fsfxwin/win/src/inc/glb -I/fsfxwin/oms/package/misam/include -I/fsfxwin/win/src/inc/trn

CFLAGS= $($(SVRID).CFLAGS) $(INCS) -D_AXCOMCOM_
LDFLAGS= $(AX_LIB) -L$(AGP_API) $(AGL_XPI) $(AG_PKGS) $(AG_LIBS) $(SYS_LIB) $(DMA_LIB) $(BPS_LIB) -L/fsfxwin/win/lib -lkillmon -L/fsfxwin/oms/lib -lmisam
YFLAGS= -I/fsfxwin/mds/src/inc/mds -I/fsudb/db2inst1/sqllib/include -I/fsfxwin/wfa/src/inc -L/fsfxwin/wfa/lib -lwfaapi -L/fsfxwin/mds/lib -ljnlf -L/fsudb/db2inst1/sqllib/lib -ldb2 -lrdb -lcom -lxml2 -lmds2 -L/fsfxwin/mymq/lib -lmymq

PROGS= wQuoteGate wqview wqclient
OBJS= *.o

# Default Rules:
#.SUFFIXES : .cpp
#.cpp.o:
#	$(XC) $(XCFLAGS} -c ${<} -o ${<:.cpp=.o}
#
.SUFFIXES : .c .sqc
.sqc.c:
	@echo "DEBUG .sqc.c"
	@db2 CONNECT TO $(DBNAME) USER $(DBUSER) USING $(DBPASS)
	db2 prep $*.sqc bindfile using $(BNDPATH)/$*.bnd package using $(BINDCD)$* COMPATIBILITY_MODE ORA
	db2 bind $(BNDPATH)/$*.bnd
	db2 quit
.c:
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@
.c~:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(CC) $(CFLAGS) $(LDFLAGS) $*.c -o $@
	rm -f $*.c
.c.a:
	$(CC) -c $(CFLAGS) $(?:.o=.c)
	ar rv $(LIBARC) $*.o
	rm -f $*.o
.s~.s:
	$(GET) $(GFLAGS) $<
.h~.h:
	$(GET) $(GFLAGS) $<
.c~.c:
	$(GET) $(GFLAGS) $<

all: $(PROGS)

yari: yari.c
		$(CC) -g $(CFLAGS) -o $@ $@.c $(LDFLAGS) $(YFLAGS) -L/fsfxwin/wfg/lib -lordchk -laxapi -lapiord -lkeitr -lkeimu 
		rm -f $@.o $@.c
yy: yy.c
		$(CC) -g $(CFLAGS) -o $@ $@.c $(LDFLAGS) $(YFLAGS) -L/fsfxwin/wfg/lib -lordchk -laxapi -lapiord -lkeitr -lkeimu 
		rm -f $@.o $@.c
apiclient: apiclient.c
		$(CC) -g $(CFLAGS) -o $@ $@.c $(LDFLAGS) $(YFLAGS) -L/fsfxwin/wfg/lib -lordchk -laxapi -lapiord -lkeitr -lkeimu -I/fsfxwin/win/src/lib/cJSON-master -L/fsfxwin/win/src/lib/cJSON-master -lcjson -lcurl
		rm -f $@.o
yjpush: yjpush.c
		$(CC) -g $(CFLAGS) -o $@ $@.c $(LDFLAGS) $(YFLAGS)
		rm -f $@.o
yjpop: yjpop.c
		$(CC) -g $(CFLAGS) -o $@ $@.c $(LDFLAGS) $(YFLAGS)
		rm -f $@.o
yqpop: yqpop.c
		$(CC) -g $(CFLAGS) -o $@ $@.c $(LDFLAGS) $(YFLAGS) -lkeimu
		rm -f $@.o
yqpush: yqpush.c
		$(CC) -g $(CFLAGS) -o $@ $@.c $(LDFLAGS) $(YFLAGS)
		rm -f $@.o

install: all
#	rm -f $(AXP_BIN)/wfgApiSndQuote
#	cp $(PROGS) $(AXP_BIN)

clean:
	touch *.c
#	rm -f $(PROGS) $(OBJS)
#	rm -f *.txt

clear:
#	rm -f $(AXP_BIN)/wfgMdReq
#	rm -f $(AXP_BIN)/wfgMdReqSMB

