include ../../../environments
include /fsfxwin/xwaw/env/angel.mak

WFAAPI_LFLAGS=-L/fsfxwin/wfa/lib -lwfaapi -laxapi

LIB=$(LIB_DIR)
INC=../../../inc/mds
COMINC=../../../inc/com
#CFLAGS=-I$(INC) $(P_CFLAGS) -I$(COMINC) -I$(INC_SRCDIR)/com  -I$(SRC_HOME)/src/inc/com -I/fsfxwin/wfa/src/inc -I/fsfxwin/wfg/src/inc -I/fsfxwin/xwaw/inc $(L_CFLAGS) $(MY_CFLAGS) $($(DATABASE)_CFLAGS) -I/fsfxwin/win/src/inc/glb
CFLAGS=$(P_CFLAGS) -I$(INC) -I$(COMINC) -I$(INC_SRCDIR)/com  -I$(SRC_HOME)/src/inc/com -I/fsfxwin/wfa/src/inc -I/fsfxwin/wfg/src/inc -I/fsfxwin/xwaw/inc $(L_CFLAGS) $(MY_CFLAGS) $($(DATABASE)_CFLAGS) -I/fsfxwin/win/src/inc/glb
LFLAGS=$(MY_LFLAGS) $(FOX_LFLAGS) $($(DATABASE)_LFLAGS) $(LFORA) -lkillmon -L/fsfxwin/xwaw/lib -lagxpi  -L/fsfxwin/mds/lib -lmds2 $(WFAAPI_LFLAGS) $(AG_PKGS) $(AGL_XPI) $(AG_LIBS) $(SYS_LIB)
AFLAGS=$(MY_AFLAGS)
AROPTS=$(MY_AROPTS)

PROGS=newmds eai4CMBS viewmst viewmds updtmst mdsinit ndfmds mdscheck
#mdscheck
OBJS=func.o mds2.o mdsrdb.o
#OBJSWORK=func.o mds2.o mdsrdb.work.o

## Suffix
.SUFFIXES : .c .sqc
.c.o:
	$(CC) -g $(CFLAGS) -c $<

.sqc.c:
	@echo "DEBUG .sqc.c"
	@db2 CONNECT TO $(DBNAME) USER $(DBUSER) USING $(DBPASS)
	db2 prep $*.sqc bindfile using $(BNDPATH)/$*.bnd package using $(BINDCD)$* COMPATIBILITY_MODE ORA
	db2 bind $(BNDPATH)/$*.bnd
	db2 quit

all: $(PROGS) 
newmds : main.o $(OBJS)
	$(CC) $(CFLAGS) main.o $(OBJS) -o $@ $(WFAAPI_LFLAGS) $(PLFLAGS) $(LFLAGS)

xmlconfchk.o:
	$(CC) -g $(CFLAGS) -c xmlconfchk.c

mdscheck : mdscheck.o xmlconfchk.o
	$(CC) -g $(CFLAGS) xmlconfchk.o $@.o -o $@ $(WFAAPI_LFLAGS) $(PLFLAGS) $(LFLAGS)

eai4CMBS : eai4CMBS.o func.o
	$(CC) $(CFLAGS) $@.o func.o -o $@ $(WFAAPI_LFLAGS) $(PLFLAGS) $(LFLAGS) -lkeimu

mdsinit : mdsinit.o $(OBJS)
	$(CC) $(CFLAGS) $@.o $(OBJS) -o $@ $(WFAAPI_LFLAGS) $(PLFLAGS) $(LFLAGS)

ndfmds : ndfmds.o func.o
	$(CC) $(CFLAGS) $@.o $(OBJS) -o $@ $(WFAAPI_LFLAGS) $(PLFLAGS) $(LFLAGS)

viewmst : viewmst.o
	$(CC) $(CFLAGS) $@.o -o $@ $(WFAAPI_LFLAGS) $(PLFLAGS) $(LFLAGS) -lncurses

viewmds : viewmds.o
	$(CC) $(CFLAGS) $@.o -o $@ $(WFAAPI_LFLAGS) $(PLFLAGS) $(LFLAGS) -lncurses

yy : yy.o
	$(CC) $(CFLAGS) $@.o -o $@ $(WFAAPI_LFLAGS) $(PLFLAGS) $(LFLAGS)

updtmst : updtmst.o
	$(CC) $(CFLAGS) $@.o -o $@ $(WFAAPI_LFLAGS) $(PLFLAGS) $(LFLAGS) -lncurses

install: all
	install $(PROGS) $(BIN_DIR)

clean:	
	rm -f *.o

run :
	mv -f $(PROG) $(BIN_DIR)
	mymqkill $(PROG)



AIX:
HPUX:
HPUX10:
HPUX11:
OSF1:
SUNOS:
LINUX:
