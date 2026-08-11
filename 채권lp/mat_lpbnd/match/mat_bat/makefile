SUFFIXES		=.c.pc.o
HOME			= /sw/fxwin
SRC_HOME		= $(HOME)/src
include			$(MAT_LIB)/make.env
include			$(MAT_LIB)/make.ora
include			$(MAT_LIB)/make.fx
TARGET			= main
ORA_SOURCE		= db_access.pc
SOURCE			= main.c proc.c mat_batch.c ${ORA_SOURCE:.pc=.c}
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= $(ORA_CFLAGS) -I. -I.. $(FX_INC)
#-I/sw/fxwin//src/wfrx/inc/wws
#-I/sw/fxwin/src/wfrx/inc/glb 
#-I/sw/fxwin/src/wfrx/inc/com 
#-I/sw/fxwin/src/mymq/include
LIB				= $(FX_LIB) $(ORA_LFLAGS)
#-L/app/fxwin/wfrx/lib -lwwscm

#DAT_DIR=\"/app/fxwin/mds/dat\"
#ETC_DIR=\"/app/fxwin/mds/etc\"
#LOG_DIR=\"/log/fxwin/mds\"
#SRC_DIR=/sw/fxwin/src
#APP_DIR=/app/fxwin

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) $(LFLAGS) -o $@ $(OBJ) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(ORA_CFLAGS) $(INCS) $(INC) -o $@ -c $<

${ORA_SOURCE:.pc=.c}		: $(ORA_SOURCE)
	proc $(ORA_PCFLAGS) $(PRO_CFLAGS) iname=$*.pc

clean		:
	rm -f $(OBJ) $(TARGET) ${ORA_SOURCE:.pc=.c}

install		:
	mv $(MAT_BIN)/mat_bat $(MAT_BIN)/mat_bat.bak
	cp $(TARGET) $(MAT_BIN)/mat_bat

