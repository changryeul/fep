SUFFIXES	=.c.o
include		$(FEP_SRC_DIR)/lib/make.env
TARGET		= main
LIB_TARGET	= $(LIBDIR)/libfep.a
SOURCE		= bat.c
OBJ			= ${SOURCE:.c=.o}
HEADER		= ${SOURCE:.c=.h}
INC			= -I.
LIB			= -lfep

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(LIB_TARGET) bat.o

clean		:
	rm -f $(OBJ) $(TARGET)

header		:
	rm -f $(INCDIR)/bat.h
	ln -s $(DIR)/bat/bat.h $(INCDIR)/bat.h


