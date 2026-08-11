SUFFIXES	=.c.o
include		$(MAT_LIB)/make.env
TARGET		= $(LIBTARGET)
SOURCE		= mat.c
OBJ			= ${SOURCE:.c=.o}
HEADER		= ${SOURCE:.c=.h}
INC			= -I. -I$(INCDIR)
LIB			= -lcdc

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(LIBTARGET) mat.o

header		:
	rm -f $(INCDIR)/mat.h
	ln -s $(DIR)/mat/mat.h $(INCDIR)/mat.h

func		:
	func -o mat.h mat.c

clean		:
	rm -f $(OBJ)

#install		:
#	cp $(TARGET) $(DIR)/lib
#	cp $(HEADER) $(DIR)/include



