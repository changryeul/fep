SUFFIXES	= .c.o
include		$(MAT_LIB)/make.env
TARGET		= $(LIBTARGET)
SOURCE		= udp.c
OBJ			= ${SOURCE:.c=.o}
HEADER		= ${SOURCE:.c=.h}
INC			= -I.
LIB			=

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(LIBTARGET) udp.o

header      :
	rm -f $(INCDIR)/udp.h
	ln -s $(DIR)/udp/udp.h $(INCDIR)/udp.h

func        :
	func -o udp.h udp.c


clean		:
	rm -f $(OBJ)


