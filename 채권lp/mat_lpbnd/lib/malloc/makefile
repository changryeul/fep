SUFFIXES	=.c.o
include    	$(FEP_SRC_DIR)/lib/make.env
TARGET		= main
SOURCE		= main.c task.c malloc.c
OBJ			= ${SOURCE:.c=.o}
HEADER		= ${SOURCE:.c=.h}
INC			= -I.
LIB			= -lfep

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(LIBTARGET) malloc.o

header		:
	rm -f $(INCDIR)/malloc.h
	ln -s $(DIR)/malloc/malloc.h $(INCDIR)/malloc.h

clean		:
	rm -f $(OBJ) $(TARGET)


