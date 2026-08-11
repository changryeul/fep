SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= $(MAT_LIB)/libmat.a
#TARGET			= $(LIBTARGET)
SOURCE			= mat.c \
					mat_stat.c \
					mat_group.c \
					mat_jang.c \
					smq.c \
					convert.c \
					print.c \
					aplog.c \
#					mat_batch.c \
#					mat_order.c \
#					mat_match.c \
#					mat_execute.c \
#					mat_sise.c \

OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.
LIB				=

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

install		:
	$(AR) $(ARFLAGS) $(TARGET) $(OBJ)

clean		:
	rm -f $(OBJ)

header		:
	rm -f $(INCDIR)/mat.h
	rm -f $(INCDIR)/smq.h
	rm -f $(INCDIR)/comm.h
	echo $(DIR)
	ln -s $(SRCDIR)/match/mat_lib/mat.h $(INCDIR)/mat.h
	ln -s $(SRCDIR)/match/mat_lib/smq.h $(INCDIR)/smq.h
	ln -s $(SRCDIR)/match/mat_lib/comm.h $(INCDIR)/comm.h

func		:
	func -o mat.h $(SOURCE)

#install		:
#	cp $(TARGET) $(DIR)/lib
#	cp $(HEADER) $(DIR)/include



