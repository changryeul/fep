SUFFIXES		=.c.o
DIR				= ../..
LIBDIR			= 
TARGET			= jbrs
CC				= gcc
LD				= gcc
CFLAGS			= -c -g
CPPFLAGS		= -c -g
LDFLAGS			= -g -o
SOURCE			= main.c task.c $(DIR)/cmd/cmd.c $(DIR)/log/log.c $(DIR)/dll/dll.c $(DIR)/cfg/dev/cfg.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I$(HOME)/src/include 		\
						-I. -I.. -I$(DIR)/cmd -I$(DIR)/log -I$(DIR)/dll -I$(DIR)/cfg/dev
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



