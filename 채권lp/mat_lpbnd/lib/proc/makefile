SUFFIXES		=.c.o
include			$(MAT_LIB)/make.env
TARGET			= $(LIBTARGET)
SOURCE			= proc.c
OBJ				= ${SOURCE:.c=.o}
HEADER			= ${SOURCE:.c=.h}
INC				= -I.
LIB				= -lfep -lcurses

all			: $(TARGET)
$(TARGET)	: $(OBJ)

.c.o		:
	$(CC) $(CFLAGS) $(INCS) $(INC) -o $@ -c $<

clean		:
	rm -f $(OBJ)

install		:
	@echo "proc library는 20231101부로 삭제 ... 모듈로 전환 "
#	$(AR) $(ARFLAGS) $(LIBTARGET) proc.o

header		:
	rm -f $(INCDIR)/proc.h
	ln -s $(DIR)/proc/proc.h $(INCDIR)/proc.h


