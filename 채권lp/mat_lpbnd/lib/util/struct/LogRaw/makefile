SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= main
SOURCE			= main.c task.c struct.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.
LIB				=

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET)

install		:
	cp $(TARGET) $(MAT_APP)/utl/struct



