SUB_DIRS		= lib \
					proc \
					log \
					match \

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

