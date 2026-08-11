SUFFIXES		=.c.o
include			$(FEP_SRC_DIR)/lib/make.env
TARGET			= $(LIBTARGET)
SOURCE			= msg.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I. -I$(INCDIR)
LIB				= -lfep

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(LIBTARGET) msg.o

header		:
	rm -f $(INCDIR)/msg.h
	ln -s $(DIR)/msg/msg.h $(INCDIR)/msg.h

clean		:
	rm -f $(OBJ)

#install		:
#	cp $(TARGET) $(FEP_BIN_DIR)



