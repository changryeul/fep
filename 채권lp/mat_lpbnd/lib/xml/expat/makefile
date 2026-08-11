SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
EXPAT			= /kmbapp/fep/src/package/expat
TARGET			= main
SOURCE			= main.c task.c my.c ../expat.c
#elements.c 
#elements2.c 
#sample.c
#sample3.c
#my_sample.c
#outline.c
#aa.c

OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.. -I. -I$(EXPAT)/include
LIB				= -L$(EXPAT)/lib -lexpat

#all			: $(TARGET)

$(TARGET)	: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(LIB)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ) $(TARGET)

install		:
	cp $(TARGET) $(FEP_BIN_DIR)



