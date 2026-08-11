SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= $(LIBTARGET)
SOURCE			= cmd.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.
LIB				= -lcdc

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(LIBTARGET) cmd.o

header		:
	rm -f $(INCDIR)/cmd.h
	ln -s $(DIR)/cmd/cmd.h $(INCDIR)/cmd.h


clean		:
	rm -f $(OBJ)

#install		:
#	cp $(TARGET) $(DIR)/lib
#	cp $(HEADER) $(DIR)/include



