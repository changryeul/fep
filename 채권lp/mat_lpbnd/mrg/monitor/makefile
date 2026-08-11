SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
include			/sw/fxwin/src/wfrx/environments
TARGET			= main
LIB_TARGET		= $(LIBDIR)/libmat.a
SOURCE			= main.c task.c mon.c aplog.c print.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.. -I.  -I/sw/fxwin/src/wfrx/inc/wws -I/sw/fxwin/src/wfrx/inc/com -I/sw/fxwin/oracle/app/db_19c/precomp/public -I/sw/fxwin/src/mymq/include
LIB				= -lwwslm03
CFLAG			= $(PLFLAGS) $(LFLAGS) $(ORA_CFLAGS)

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(CFLAG) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET)

func		:
	func -o task.h $(SOURCE)

#install		:
#	cp $(TARGET) $(MAT_BIN)/smq



