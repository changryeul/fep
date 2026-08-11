SUFFIXES		=.c.o
DIR				= $(HOME)/test/lib
LIBDIR			= $(HOME)/test/lib
TARGET			= test
CC				= gcc
LD				= gcc
CFLAGS			= -c -g
CPPFLAGS		= -c -g
LDFLAGS			= -g -o
SOURCE			= test.c task.c cmd.c $(DIR)/log/log.c $(DIR)/dll/dll.c $(DIR)/cfg/cfg.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I$(HOME)/src/include 		\
						-I. -I.. -I$(DIR)/log -I$(DIR)/dll -I$(DIR)/cfg
LIB				= -L$(DIR)/src/lib
LIBS			=

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



