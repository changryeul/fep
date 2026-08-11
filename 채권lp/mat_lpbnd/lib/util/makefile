include			$(FEP_LIB_DIR)/make.env
SUB_DIRS		= calc  \
					cfunc  \
					func  \
					struct
#					convert  \
#					err_no  \
#					jbrs  \
#					tail  \
#					test  \
#					time  \
#					tree  \
#					web  \
#					zmodem

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


