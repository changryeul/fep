TARGET     = libyarb
STATIC_LIB = $(TARGET).a
CC         = gcc
CFLAGS     = -g -Wall -fPIC -Wextra -fno-pie
INCLUDE    = -I/fsfxwin/fep/yarb/inc -I/fsfxwin/fep/st01/inc
AR         = ar

SRCS =\
	  l_prc.c   \
	  l_set.c   \
	  l_ord.c   \
	  l_thr.c   \
	  l_def.c   \
	  l_dbg.c   \
	  l_sem.c   \
	  l_utl.c   \

OBJS = $(SRCS:.c=.o)
all  : $(STATIC_LIB) 
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@ -no-pie

$(STATIC_LIB) : $(OBJS)
	@echo "=====> Mow Linking $(TARGET)"
	$(AR) rUv $(STATIC_LIB) $(OBJS)
	@echo "=====> $(TARGET) Completed!!"

clean	:
	$(RM) -f $(OBJS)
	$(RM) -f $(TARGET)
	@echo "=====> All objects has been removed."

install	: $(TARGET)
	$(CP) -f $(TARGET) $(LIBBIN)
	@echo "=====> Installation Completed"
