SUFFIXES		=.c.o
include			$(FEP_SRC_DIR)/lib/make.env
TARGET			= $(LIBTARGET)
SOURCE			= stp.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.
LIB				= -lfep -lcurses

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(LIBTARGET) stp.o

clean		:
	rm -f $(OBJ)

header		:
	rm -f $(INCDIR)/stp.h
	ln -s $(DIR)/stp/stp.h $(INCDIR)/stp.h

#install		:
#	cp $(TARGET) $(DIR)/lib
#	cp $(HEADER) $(DIR)/include

export			:
