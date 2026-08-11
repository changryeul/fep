SUFFIXES		=.c.o
include			$(FEP_LIB_DIR)/make.env
EXPAT			= $(FEP_APP_DIR)/package/expat
TARGET			= main
LIB_TARGET		= $(DIR)/libcdc.a
SOURCE			= main.c task.c server.c process.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.. -I.
LIB				= -lcdc -L$(EXPAT) -lexpat

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET)

install		:
	cp $(TARGET) $(FEP_BIN_DIR)



