TARGET = libalarm
STATIC_LIB = $(TARGET).a
CC      = gcc
CFLAGS  = -std=gnu99 -O2 -Wall -fPIC -Wextra -fno-pie
INCLUDE = -I./include
AR      = ar

SRCS = alarm.c

OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all :$(STATIC_LIB) 

# Compile Object File
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@ -Wl,--no-relax 

#static library
$(STATIC_LIB):$(OBJS)
	$(AR) rcs  $@ $^

#shared library
#$(SHARED_LIB):$(OBJS)
#	$(CC) -shared -o $@ $^

#clean

install:
	mv libalarm.a ./lib/libalarm.a
#cp libarb.so ../lib/libarb.so

clean:
	rm -f *.o *.a *.so
	@echo "Clean done."

