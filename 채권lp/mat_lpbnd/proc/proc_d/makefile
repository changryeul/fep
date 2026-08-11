SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= main
LIB_TARGET		= $(DIR)/libmat.a
SOURCE			= main.c ../proc.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I. -I..
LIB				= -lmat

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(ORALIB) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET) ${ORASRC:.pc=.c} *.lis

install		:
	mv $(MAT_BIN)/proc_d $(MAT_BIN)/proc_d.bak
	cp $(TARGET) $(MAT_BIN)/proc_d

