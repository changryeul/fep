SUFFIXES		=.c.o
include			$(FEP_SRC_DIR)/lib/make.env
EXPAT			= /kmbapp/fep/src/package/expat
TARGET			= $(LIBTARGET)
SOURCE			= xml.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.. -I. -I$(EXPAT)/include
LIB				= -lfep -L$(EXPAT)/lib -lexpat

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ)

install		:
	$(AR) $(ARFLAGS) $(LIBTARGET) xml.o

header		:
	rm -f $(INCDIR)/xml.h
	ln -s $(DIR)/xml/xml.h $(INCDIR)/xml.h



