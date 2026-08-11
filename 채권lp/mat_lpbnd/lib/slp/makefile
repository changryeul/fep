SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= $(LIBTARGET)
SOURCE			= slp.c slp_sub.c slp_stat.c slp_sise.c print.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.
LIB				= -lmats

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

#install		:
#	$(AR) $(ARFLAGS) $(LIBTARGET) slp.o
#
#header		:
#	rm -f $(INCDIR)/slp.h
#	ln -s $(DIR)/slp/slp.h $(INCDIR)/slp.h

func		:
	func -o slp.h $(SOURCE)

print		:
	struct -o print.c slp.h

clean		:
	rm -f $(OBJ)

#install		:
#	cp $(TARGET) $(DIR)/lib
#	cp $(HEADER) $(DIR)/include



