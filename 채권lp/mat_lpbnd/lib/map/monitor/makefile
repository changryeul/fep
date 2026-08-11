SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= main
LIB_TARGET		= $(LIBDIR)/libmat.a
SOURCE			= main.c task.c mon.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.. -I. 
LIB				=

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET)

func		:
	func -o task.h $(SOURCE)

#install		:
#	cp $(TARGET) $(MAT_BIN)/smq



