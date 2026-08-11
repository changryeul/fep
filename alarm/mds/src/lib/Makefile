include ../environments

DIR=mds com

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
