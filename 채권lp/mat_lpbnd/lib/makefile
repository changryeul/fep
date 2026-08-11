include			make.env
SUB_DIRS		= cfg \
					cmd \
					dll \
					etc \
					log \
					map \
					mem \
					sem \
					tcp \
					udp \
					mcp \
#					mat \
#					smq \
#					util \
#					proc \
#					sam \
#					stp \
#					bat \
#					xml \

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

ctags			:
	@for dir in $(SUB_DIRS) ; \
	do \
		cd $$dir ; \
		echo "\n##### [$$dir] #####" ; \
		$(MAKE) $@ ; \
		cd .. ; \
	done 

export :

