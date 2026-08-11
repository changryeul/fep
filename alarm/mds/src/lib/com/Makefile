include ../../environments
#
INC=../../inc/mds
CFLAGS=$(L_CFLAGS) -I$(INC) 
CFLAGS=-g $(L_CFLAGS) -I$(INC) 
CXXFLAGS=-g $(L_CXXFLAGS) -I$(INC) 
LFLAGS=$(L_LFLAGS)
AFLAGS=$(MY_AFLAGS)
AROPTS=$(MY_AROPTS)

X_GFLIB=libcom.a
S_GFLIB=libcom.so

OBJECT=$(X_GFLIB) $(S_GFLIB) *.o

INCF=$(INC)/mdfold.h $(INC)/context.h $(INC)/stream.h $(INC)/config.h 

OBJF=ipcsub.o netconf.o

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

$(X_GFLIB):	$(X_GFLIB)(ipcsub.o)		\
		$(X_GFLIB)(netconf.o)
		if [ -s /usr/bin/ranlib ]; then ranlib $(X_GFLIB); fi
		@echo $(X_GFLIB) is up-to-date.
$(X_GFLIB)(ipcsub.o):		$(INCF)
$(X_GFLIB)(netconf.o):		$(INCF)

install: all
	rm -f $(LIB_DIR)/$(X_GFLIB)
	rm -f $(LIB_DIR)/$(S_GFLIB)
	cp -f $(X_GFLIB) $(LIB_DIR)/$(X_GFLIB)
	cp -f $(S_GFLIB) $(LIB_DIR)/$(S_GFLIB)

clean:
	rm -f $(OBJECT)
