SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= main
LIB_TARGET		= $(LIBDIR)/libcdc.a
SOURCE			= main.c task.c print.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I. -I.. -I../.. -I/fsfxwin/fep/st01/inc
LIB				=

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET)

install		:
	cp $(TARGET) $(MAT_BIN)/mat_sis



