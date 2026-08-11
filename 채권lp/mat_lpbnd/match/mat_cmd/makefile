SUFFIXES		=.c.pc.o
HOME            = /sw/fxwin
SRC_HOME        = $(HOME)/src
include			$(MAT_LIB)/make.env
include         $(MAT_LIB)/make.ora
TARGET			= main
LIB_TARGET		= $(LIBDIR)/libcdc.a
ORA_SOURCE      = db_access.pc
SOURCE			= main.c task.c proc.c \
					mon.c mon_small.c mon_index.c mon_smq.c mon_statis.c mon_order.c \
					../mat_lib/mat_jang.c ../mat_lib/mat_stat.c mat_batch.c \
					${ORA_SOURCE:.pc=.c} 
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.. -I. -I/fxwin/sw/wfrx/inc
LIB				= -lmat -L/fxwin/app/wfrx/lib -L/fxwin/app/wfrx/lib/com $(ORA_LFLAGS)

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(ORA_CFLAGS) $(INCS) $(INC) -o $@ -c $<

${ORA_SOURCE:.pc=.c}	: $(ORA_SOURCE)
	proc $(ORA_PCFLAGS) $(PRO_CFLAGS) iname=$*.pc

clean		:
	rm -f $(OBJ) $(TARGET) ${ORA_SOURCE:.pc=.c}

install		:
	mv $(MAT_BIN)/mat $(MAT_BIN)/mat.bak
	cp $(TARGET) $(MAT_BIN)/mat



