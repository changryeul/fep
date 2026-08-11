SUFFIXES	=.c.o
DIR			= ../..
LIBDIR		= $(DIR)
INCDIR		= $(DIR)/include
TARGET		= web
CC			= gcc
LD			= gcc
CFLAGS		= -c -g
CPPFLAGS	= -c -g
LDFLAGS		= -g -o
SOURCE		= main.c web.c
OBJ			= ${SOURCE:.c=.o}
HEADER		= ${SOURCE:.c=.h}
INC			= -I$(INCDIR) -I.
LIB			= -L$(DIR)
LIBS		= -lcdc

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) $@ $(OBJ) $(LIB) $(LIBS)

.c.o		:
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET)

#install		:
#	cp $(TARGET) $(DIR)/lib
#	cp $(HEADER) $(DIR)/include



