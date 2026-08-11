SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= $(LIBTARGET)
SOURCE			= log.c lsm.c
CFLAG			= -D ERRMSG=0
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I. -I/kmbapp/app/inc
LIB				= -lmats

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) -DERRMSG $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(LIBTARGET) log.o lsm.o
# log_mem.o

#import		:
#	rm -f $(INCDIR)/errmsg.h
#	ln -s $(IMPORT_HEADER) $(INCDIR)/errmsg.h

header		:
	rm -f $(INCDIR)/log.h
	ln -s $(DIR)/log/log.h $(INCDIR)/log.h
	rm -f $(INCDIR)/log_conv.h
	ln -s $(DIR)/log/log_conv.h $(INCDIR)/log_conv.h

func		:
	func -o log.h $(SOURCE)

clean		:
	rm -f $(OBJ)

#install		:
#	cp $(TARGET) $(DIR)/lib
#	cp $(HEADER) $(DIR)/include



