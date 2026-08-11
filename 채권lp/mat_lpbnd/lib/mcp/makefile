SUFFIXES	= .c.o
include		$(MAT_LIB)/make.env
TARGET		= $(LIBTARGET)
SOURCE		= mcp.c
OBJ			= ${SOURCE:.c=.o}
HEADER		= ${SOURCE:.c=.h}
INC			= -I.
LIB			=

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(LIBTARGET) mcp.o

header      :
	rm -f $(INCDIR)/mcp.h
	ln -s $(DIR)/mcp/mcp.h $(INCDIR)/mcp.h

func        :
	func -o mcp.h mcp.c


clean		:
	rm -f $(OBJ)


