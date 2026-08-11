SUFFIXES	=.c.o
include		$(MAT_LIB)/make.env
TARGET		= $(LIBTARGET)
SOURCE		= win.c
OBJ			= ${SOURCE:.c=.o}
HEADER		= ${SOURCE:.c=.h}
INC			= -I.
LIB			= -lcdc

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(LIBTARGET) win.o

header		:
	rm -f $(INCDIR)/win.h
	ln -s $(DIR)/map/win/win.h $(INCDIR)/win.h


clean		:
	rm -f $(OBJ)


