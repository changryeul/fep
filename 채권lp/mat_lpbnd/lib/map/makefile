SUFFIXES	=.c.o
include		$(MAT_LIB)/make.env
TARGET		= $(LIBTARGET)
SOURCE		= map.c map_menu.c map_edit.c
OBJ			= ${SOURCE:.c=.o}
HEADER		= ${SOURCE:.c=.h}
INC			= -I. -I$(INCDIR)
LIB			= -lcdc

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(LIBTARGET) $(OBJ)

header		:
	rm -f $(INCDIR)/map.h
	ln -s $(DIR)/map/map.h $(INCDIR)/map.h

func		:
	func -o map.h map.c map_menu.c map_edit.c

clean		:
	rm -f $(OBJ)

#install		:
#	cp $(TARGET) $(DIR)/lib
#	cp $(HEADER) $(DIR)/include



