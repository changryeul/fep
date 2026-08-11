SUFFIXES		=.c.o
HOME            = /sw/fxwin
SRC_HOME        = $(HOME)/src
include			$(MAT_LIB)/make.env
TARGET			= main
LIB_TARGET		= $(LIBDIR)/libcdc.a
SOURCE			= main.c task.c proc.c mon.c mon_small.c mon_index.c mon_smq.c mon_statis.c mon_order.c ../../mat_lib/mat_stat.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.. -I. 
LIB				= -lmat $(ORA_LFLAGS)

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET) ${ORA_SOURCE:.pc=.c}

install		:
	mv $(MAT_BIN)/mat $(MAT_BIN)/mat.bak
	cp $(TARGET) $(MAT_BIN)/mat



