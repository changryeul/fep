include ./environments

DIR=lib pibb pibd pibm pibo7000 pibo7500 pibo7600 b2b sfile chck cmd util psys

all:
	for i in $(DIR); do\
		(cd $$i; $(MAKE) all);\
	done

install:
	for i in $(DIR); do\
		(cd $$i; $(MAKE) install);\
	done
clean:
	for i in $(DIR); do\
		(cd $$i; $(MAKE) clean);\
	done
config:
	for d in $(PREFIX) $(INC_DIR) $(LIB_DIR) $(BIN_DIR) $(ETC_DIR) $(CFG_DIR) $(LOG_DIR) $(SRC_DIR) $(SHL_DIR) $(TMP_DIR); do\
		if [ ! -d $$d ]; then mkdir $$d; fi \
	done
