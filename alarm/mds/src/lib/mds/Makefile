include ../../environments
#
INC=../../inc/mds
CFLAGS=$(L_CFLAGS) -I$(INC)  $(P_CFLAGS)
CFLAGS=-g $(L_CFLAGS) -I$(INC) -I$(WFGPATH)/src/inc  -I$(SRC_HOME)/src/inc/com $(P_CFLAGS) -I/fsfxwin/wfa/src/inc -I/fsfxwin/xwaw/inc -I/fsfxwin/wfg/src/adj/wgate
CXXFLAGS=-g $(L_CXXFLAGS) -I$(INC) $(P_CFLAGS)
LFLAGS=$(L_LFLAGS) $(LFORA) $(PLFLAGS) -L$(DB2_LIB) -lkeitr
AFLAGS=$(MY_AFLAGS)
AROPTS=$(MY_AROPTS)

X_GFLIB=libmds2.a
S_GFLIB=libmds2.so

OBJECT=$(X_GFLIB) $(S_GFLIB) *.o

#INCF=$(INC)/mdfold.h $(INC)/context.h $(INC)/stream.h $(INC)/config.h $(INC)/cmbstrn.h $(WFGPATH)/src/inc/axsnd.h
ifeq ($(EAI_SYSTEM_ENV), D)
    DBNAME=DSKEIT
else
    DBNAME=DSKEIU
endif

OBJF = mdsap.o xmlcfg.o time.o
## Suffix
.SUFFIXES : .c .sqc

# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@
.c~:
	$(GET) $(GFLAGS) -p $< > $*.c
	$(CC) $(CFLAGS) $(LFLAGS) $*.c -o $@
	-rm -f $*.c

 .sqc.c :
	@echo "DEBUG .sqc.c"
	@db2 CONNECT TO $(DBNAME) USER $(DBUSER) USING $(DBPASS)
	db2 prep $*.sqc bindfile using $(BNDPATH)/$*.bnd package using $(BINDCD)$* UNSAFENULL YES COMPATIBILITY_MODE ORA
	db2 bind $(BNDPATH)/$*.bnd
	db2 quit

.c.a:
	$(CC) -c $(CFLAGS) $<
	$(AR) $(AFLAGS) $@ $*.o; rm -f $*.o 
.s~.s:
	$(GET) $(GFLAGS) $<
.h~.h:
	$(GET) $(GFLAGS) $<
.c~.c:
	$(GET) $(GFLAGS) $<

all:		$(S_GFLIB)
$(S_GFLIB): 	$(X_GFLIB)
		if [ "${PLATFORM}" = "AIX" ]; then \
			$(CC) -o $(S_GFLIB) $(OBJF) $(CFLAGS) $(LFLAGS); \
		fi
		if [ "${PLATFORM}" = "SunOS" ]; then \
			$(AR) $(AROPTS) -x $(X_GFLIB) $(OBJF); \
			$(CC) -o $(S_GFLIB) $(OBJF) $(CFLAGS) $(LFLAGS); \
			rm -f $(OBJF); \
		fi
		if [ "${PLATFORM}" = "Linux" ]; then \
			$(AR) $(AROPTS) -x $(X_GFLIB) $(OBJF); \
			$(LD) -o $(S_GFLIB) $(OBJF) $(LFLAGS); \
			rm -f $(OBJF); \
		fi

OBJF = mdsap.o xmlcfg.o time.o
$(X_GFLIB):	$(X_GFLIB)(mdsap.o)		\
		$(X_GFLIB)(xmlcfg.o)		\
		$(X_GFLIB)(time.o)
		if [ -s /usr/bin/ranlib ]; then ranlib $(X_GFLIB); fi
		@echo $(X_GFLIB) is up-to-date.
$(X_GFLIB)(mdsap.o):		$(INCF)
$(X_GFLIB)(xmlcfg.o):		$(INCF)
$(X_GFLIB)(time.o):			$(INCF)

install: all
	rm -f $(LIB_DIR)/$(X_GFLIB)
	rm -f $(LIB_DIR)/$(S_GFLIB)
	cp -f $(X_GFLIB) $(LIB_DIR)/$(X_GFLIB)
	cp -f $(S_GFLIB) $(LIB_DIR)/$(S_GFLIB)

clean:
	rm -f $(OBJECT) mdsap.c
