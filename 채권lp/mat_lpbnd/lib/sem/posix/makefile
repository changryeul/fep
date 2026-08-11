SUFFIXES		=.c.o
include			$(MAT_LIB_DIR)/make.env
TARGET			= $(LIBTARGET)
SOURCE			= psem.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.
LIB				= -lcdc

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(LIBTARGET) sem.o

clean		:
	rm -f $(OBJ)

header		:
	rm -f $(INCDIR)/psem.h
	ln -s $(DIR)/sem/psem.h $(INCDIR)/psem.h

func		:
	touch psem_func.h
	rm psem_func.h
	func -o psem_func.h psem.c

#install		:
#	cp $(TARGET) $(DIR)/lib
#	cp $(HEADER) $(DIR)/include



