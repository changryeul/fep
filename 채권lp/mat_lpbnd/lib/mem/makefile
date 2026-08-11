SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= $(LIBTARGET)
SOURCE			= mem.c
# memlog.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.
LIB				= -lcdc

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(LIBTARGET) mem.o
# memlog.o

clean		:
	rm -f $(OBJ)

func		:
	func -o mem.h $(SOURCE)

header		:
	rm -f $(INCDIR)/mem.h
	ln -s $(DIR)/mem/mem.h $(INCDIR)/mem.h

#install		:
#	cp $(TARGET) $(DIR)/lib
#	cp $(HEADER) $(DIR)/include



