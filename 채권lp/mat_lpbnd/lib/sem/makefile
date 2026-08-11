SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= $(LIBTARGET)
SOURCE			= sem.c
# semlog.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.
LIB				=

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(LIBTARGET) sem.o
# semlog.o

clean		:
	rm -f $(OBJ)

header		:
	rm -f $(INCDIR)/sem.h
	ln -s $(DIR)/sem/sem.h $(INCDIR)/sem.h

func		:
	func -o sem.h sem.c
# semlog.c

#install		:
#	cp $(TARGET) $(DIR)/lib
#	cp $(HEADER) $(DIR)/include



