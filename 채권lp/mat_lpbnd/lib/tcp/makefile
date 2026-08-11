SUFFIXES	= .c.o
include		$(MAT_LIB)/make.env
TARGET		= $(LIBTARGET)
SOURCE		= tcp.c
OBJ			= ${SOURCE:.c=.o}
HEADER		= ${SOURCE:.c=.h}
INC			= -I.
LIB			= -lmats

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(LIBTARGET) tcp.o

clean		:
	rm -f $(OBJ)

header      :
	rm -f $(INCDIR)/tcp.h
	ln -s $(DIR)/tcp/tcp.h $(INCDIR)/tcp.h


