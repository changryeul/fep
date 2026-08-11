SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= main
LIB_TARGET		= $(DIR)/libfep.a
SOURCE			= main.c task.c server.c ../tcp.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.. -I.
LIB				= -lmats

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET)

install		:
	cp $(TARGET) $(FEP_BIN_DIR)



