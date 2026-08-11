SUFFIXES		=.c.o

DIR			= $(HOME)/cdc/lib
LIBDIR		= $(DIR)
INCDIR		= $(DIR)/include
LIBS		= -L$(LIBDIR) -lsocket -lnsl
INCS		= -I$(INCDIR)
CC			= gcc
LD			= gcc
AR			= ar
CFLAGS		= -c -m64 -g
CPPFLAGS	= -c -m64 -g
LDFLAGS		= -g -m64
ARFLAGS		= rucs

TARGET			= struct
SOURCE			= main.c task.c ../struct.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.. -I.
LIB				= -lcdc

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET)

#install		:
#	cp $(TARGET) $(DIR)/lib
#	cp $(HEADER) $(DIR)/include



