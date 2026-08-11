include			$(MAT_LIB)/make.env
SUB_DIRS		= mat_lib \
					mat_bat \
					mat_rcv \
					mat_ord \
					mat_mat \
					mat_sis \
					mat_exe \
					mat_snd \
					mat_cmd \
					smq_cmd \
#					exe_rcv \
#					mat_sis \

all				:
	@echo "[1;31mmat_lib ¸ğµâÀ» build ÇØ¼­ library update ÈÄ ÀüÃ¼ ÄÄÆÄÀÏÀ» ¼öÇà ÇÏ½Ê½Ã¿ä.[0m"
	@for dir in $(SUB_DIRS) ; \
	do \
		cd $$dir ; \
		echo "\n##### [$$dir] #####" ; \
		$(MAKE) ; \
		cd .. ; \
	done 

clean			:
	@echo "[1;31mmat_lib ¸ğµâÀ» build ÇØ¼­ library update ÈÄ ÀüÃ¼ ÄÄÆÄÀÏÀ» ¼öÇà ÇÏ½Ê½Ã¿ä.[0m"
	@for dir in $(SUB_DIRS) ; \
	do \
		cd $$dir ; \
		echo "\n##### [$$dir] #####" ; \
		$(MAKE) $@ ; \
		cd .. ; \
	done 

install			:
	@echo "[1;31mmat_lib ¸ğµâÀ» build ÇØ¼­ library update ÈÄ ÀüÃ¼ ÄÄÆÄÀÏÀ» ¼öÇà ÇÏ½Ê½Ã¿ä.[0m"
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

