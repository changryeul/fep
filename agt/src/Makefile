

CC=gcc
CFLAGS= -std=gnu99 -O2 -Wall -Wextra -g -I../include
LDLIBS= -lrt
OBJS_COMMON= lib/log.o lib/config.o lib/netutil.o lib/protocol.o lib/handlers.o lib/cid.o lib/msg.o
OBJS_COMMON2= lib/log.o lib/config.o lib/netutil.o lib/protocol.o lib/cid.o lib/msg.o
VPATH=src:lib

#TEST_APP := mock_client
#TEST_SRC := mock_client.c netutil.c protocol.c log.c 
#TEST_OBJ := $(TEST_SRC:.c=.o)
.PHONY: all clean

TARGETS=oms_agent bat_agent rds_agent

all:$(TARGETS)

oms_agent:oms_main.o $(OBJS_COMMON)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

bat_agent:bat_main.o $(OBJS_COMMON)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

rds_agent:rds_main.o $(OBJS_COMMON2)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install:
	@echo "installing targets ... "
	@mkdir -p ../bin/old 
	@for t in $(TARGETS); do \
		if [ -f ../bin/$$t ]; then \
			echo " --> 백업 ../bin/old/$$t"; \
			cp -f ../bin/$$t ../bin/old/$$t; \
		fi; \
		echo " -->  설치 : $$t -->../bin/$$t"; \
		cp -f $$t ../bin/$$t; \
		chmod 777 ../bin/$$t; \
	done
	echo "설치완료"

clean:
	rm -f $(TARGETS) $(TARGETS).o $(OBJS_COMMON) 
