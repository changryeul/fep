SUFFIXES	= .c.o
include		$(MAT_LIB)/make.env
TARGET		= main
LIB_TARGET	= $(DIR)/libmats.a
SOURCE		= main.c task.c
OBJ			= ${SOURCE:.c=.o}
HEADER		= ${SOURCE:.c=.h}
INC			= -I.
LIB			= -lmats

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET)


