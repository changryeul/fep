SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= $(LIBTARGET)
SOURCE			= msg.c
# msglog.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.
LIB				= -lcdc

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(LIBTARGET) msg.o
# msglog.o

clean		:
	rm -f $(OBJ)

func		:
	func -o msg.h $(SOURCE)

header		:
	rm -f $(INCDIR)/msg.h
	ln -s $(DIR)/msg/msg.h $(INCDIR)/msg.h

#install		:
#	cp $(TARGET) $(DIR)/lib
#	cp $(HEADER) $(DIR)/include



