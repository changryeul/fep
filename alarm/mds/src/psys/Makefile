include ../environments

#
AIX_LD=$(CC)
SunOS_LD=ld
HPUX_LD=ld
Linux_LD=ld
LD=$($(PLATFORM)_LD)

AIX_LDOPT=-bE:export.xq.aix -bM:SRE -bnoentry -lc
SunOS_LDOPT=-b -G -lc ${socketlib}
HPUX_LDOPT=-b -c export.xq.hpux
#Linux_LDOPT=-lc -lnsl
Linux_LDOPT=-lc

#
# compress methods
#
#CFLAGS=-O $(MY_CFLAGS) $(MY_SFLAGS) -w
LFLAGS=$(MY_LFLAGS) $($(PLATFORM)_LDOPT)  $(WFAAPI_LFLAGS)
#CFLAGS=-O $($(PLATFORM)_CFLAGS) $(L_CFLAGS) $(MY_SFLAGS) -w
CFLAGS=-O -I../inc/mds $(MY_CFLAGS) $(FOX_CFLAGS) $($(DATABASE)_CFLAGS) $($(PLATFORM)_CFLAGS) $(L_CFLAGS) $(MY_SFLAGS) -w
#LFLAGS=$($(PLATFORM)_SFLAGS) $($(PLATFORM)_LDOPT)
AFLAGS=$(MY_AFLAGS)
AROPTS=$(MY_AROPTS)

X_TST= psysmon pclrlog logbackup

OBJECT=$(X_TST) *.o 
X_INC=../inc/mds/config.h 

# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@
.c~:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(CC) $(CFLAGS) $(LFLAGS) $*.c -o $@
	-rm -f $*.c
.c.a:
	$(CC) -c $(CFLAGS) $<
	$(AR) $(AFLAGS) $@ $*.o; rm -f $*.o 
.s~.s:
	$(GET) $(GFLAGS) $<
.h~.h:
	$(GET) $(GFLAGS) $<
.c~.c:
	$(GET) $(GFLAGS) $<

all:	$(X_TST) 

psysmon:	psysmon.c comsub.c $(X_INC) 
	$(CC) -o $@ $@.c comsub.c $(CFLAGS) $(LFLAGS)

pclrlog:	pclrlog.c comsub.c $(X_INC) 
	$(CC) -o $@ $@.c comsub.c $(CFLAGS) $(LFLAGS)

logbackup:	logbackup.c comsub.c $(X_INC) 
	$(CC) -o $@ $@.c comsub.c $(CFLAGS) $(LFLAGS)

install: all
	rm -f $(BIN_DIR)/psysmon
	cp $(X_TST) $(BIN_DIR)

clean:
	-rm -f $(OBJECT)
