SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= main
LIB_TARGET		= $(LIBDIR)/libmat.a
SOURCE			= main.c task.c process.c file_load.c ../map.c ../map_edit.c ../map_menu.c ../win/win.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.. -I.
LIB				= -lmat

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET)

install		:
	cp $(TARGET) $(FEP_BIN_DIR)



