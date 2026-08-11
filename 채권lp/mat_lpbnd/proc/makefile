include			$(MAT_LIB)/make.env
SUB_DIRS		= proc_d \
					process \

all				:
	@for dir in $(SUB_DIRS) ; \
	do \
		cd $$dir ; \
		echo "\n##### [$$dir] #####" ; \
		$(MAKE) ; \
		cd .. ; \
	done 

clean			:
	@for dir in $(SUB_DIRS) ; \
	do \
		cd $$dir ; \
		echo "\n##### [$$dir] #####" ; \
		$(MAKE) $@ ; \
		cd .. ; \
	done 

install			:
	@for dir in $(SUB_DIRS) ; \
	do \
		cd $$dir ; \
		echo "\n##### [$$dir] #####" ; \
		$(MAKE) $@ ; \
		cd .. ; \
	done 

header			:
	@for dir in $(SUB_DIRS) ; \
	do \
		cd $$dir ; \
		echo "\n##### [$$dir] #####" ; \
		$(MAKE) $@ ; \
		cd .. ; \
	done 

export :

