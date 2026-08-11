SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= main
LIB_TARGET		= $(LIBDIR)/libcdc.a
SOURCE			= main.c task.c mat_match.c mat_fee.c aplog.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I. -I..
# 수수료 라이브러리
LIB				= -lwwslm03

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIB) $(LIBS)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET)

install		:
	mv $(MAT_BIN)/mat_mat $(MAT_BIN)/mat_mat.bak
	cp $(TARGET) $(MAT_BIN)/mat_mat



