SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= libblp.a
SOURCE			= blp.c blp_sise.c blp_order.c blp_convert.c blp_print.c
CFLAG			= -D ERRMSG=0
OBJ				= ${SOURCE:.c=.o}
HEADER			= blp.h
INC				= -I/fsfxwin/fep/st01/inc -I.
LIB				= -lmats

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ)

header		:
	rm -f $(INCDIR)/$(HEADER)
	ln -fs $(DIR)/blp/$(HEADER) $(INCDIR)/$(HEADER)
	ln -fs $(DIR)/blp/blp.c $(INCDIR)/blp.c
	ln -fs $(DIR)/blp/blp_sise.c $(INCDIR)/blp_sise.c
	ln -fs $(DIR)/blp/blp_order.c $(INCDIR)/blp_order.c
	ln -fs $(DIR)/blp/blp_convert.c $(INCDIR)/blp_convert.c
	ln -fs $(DIR)/blp/blp_print.c $(INCDIR)/blp_print.c
#	ln -s $(DIR)/test/bond/$(SOURCE) $(INCDIR)/$(SOURCE)

func		:
	func -o $(HEADER) $(SOURCE)

clean		:
	rm -f $(OBJ)

#install		:
#	cp $(TARGET) $(DIR)/lib
#	cp $(HEADER) $(DIR)/include



