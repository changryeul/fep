include			$(FEP_SRC_DIR)/lib/make.env
TARGET			= sam.o
LIB_TARGET		= $(LIBTARGET)
SOURCE			= sam.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.. -I.
LIB				= -lfep

#all			: $(TARGET)

$(TARGET)	: $(SOURCE)
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install     :
	$(AR) $(ARFLAGS) $(LIB_TARGET) sam.o

header      :
	rm -f $(INCDIR)/sam.h
	ln -s $(DIR)/sam/sam.h $(INCDIR)/sam.h

clean		:
	rm -f $(TARGET)




