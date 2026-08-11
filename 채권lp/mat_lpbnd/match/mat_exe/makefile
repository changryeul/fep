SUFFIXES		=.c.o
HOME			= /sw/fxwin
include			$(MAT_LIB)/make.env
include			$(MAT_LIB)/make.ora
include			$(MAT_LIB)/make.fx
TARGET			= main
ORA_SOURCE		= db_access.pc
SOURCE			= main.c proc.c mat_execute.c ${ORA_SOURCE:.pc=.c} aplog.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.. -I. $(FX_INC)
LIB				= $(FX_LIB) $(ORA_LFLAGS)

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) $(LFLAGS) -o $@ $(OBJ) $(LIB) $(LIBS)

.c.o		:
	$(CC) $(CFLAGS) $(ORA_CFLAGS) $(INCS) $(INC) -o $@ -c $<

${ORA_SOURCE:.pc=.c}	: $(ORA_SOURCE)
	proc $(ORA_PCFLAGS) iname=$*.pc 
#	$(PRO_CFLAGS) 

clean		:
	rm -f $(OBJ) $(TARGET) ${ORA_SOURCE:.pc=.c}

install		:
	mv $(MAT_BIN)/mat_exe $(MAT_BIN)/mat_exe.bak
	cp $(TARGET) $(MAT_BIN)/mat_exe



