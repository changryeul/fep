SUFFIXES	=.c.o
DIR		= $(HOME)/test
LIBDIR		= $(HOME)/test
TARGET		= cfunc
CC		= gcc
LD		= gcc
CFLAGS		= -c -g
CPPFLAGS	= -c -g
LDFLAGS		= -g -o
SOURCE		= cfunc.c
OBJ		= ${SOURCE:.c=.o}
HEADER		= ${SOURCE:.c=.h}
INC		= -I$(HOME)/src/include 		\
			-I. -I..
LIB			= -L$(DIR)/src/lib
LIBS		=

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) $@ $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET)

#install		:
#	cp $(TARGET) $(DIR)/lib
#	cp $(HEADER) $(DIR)/include



