SUFFIXES		=.c.o
include			$(HOME)/lib/make.env
include			$(HOME)/lib/mymq.env
TARGET			= main
LIB_TARGET		= $(LIBDIR)/libcdc.a
SOURCE			= main.c task.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I. -I..

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET)

install		:
	cp $(TARGET) $(MAT_BIN)/exe_rcv



