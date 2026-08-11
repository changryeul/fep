SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= main
LIB_TARGET		= $(LIBDIR)/libmat.a
SOURCE			= main.c task.c mon.c \
				../blp.c \
				../blp_sise.c \
				../blp_order.c \
				../blp_convert.c \
				../blp_print.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I/fsfxwin/fep/st01/inc -I.. -I.
LIB				= 

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET)

install		:
	cp $(TARGET) $(HOME)/bin



