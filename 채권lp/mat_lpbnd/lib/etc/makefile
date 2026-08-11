SUFFIXES	=.c.o
include		$(MAT_LIB)/make.env
TARGET		= $(LIBTARGET)
SOURCE		= etc.c conv.c
OBJ			= ${SOURCE:.c=.o}
HEADER		= ${SOURCE:.c=.h}
INC			= -I.
LIB			= -lmats

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -D_GNU_SOURCE -o $@ -c $<
# _GNU_SOURCE ---> exp10

install		:
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ)

header		:
	rm -f $(INCDIR)/etc.h
	ln -s $(DIR)/etc/etc.h $(INCDIR)/etc.h

func		:
	func -o etc.h $(SOURCE)

clean		:
	rm -f $(OBJ)

export		:
