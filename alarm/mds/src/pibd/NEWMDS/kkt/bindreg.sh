

include ../../../environments
ifeq ($(EAI_SYSTEM_ENV), D)
	DBNAME=DSKEIT
else
	DBNAME=DSKEIU
endif

@db2 CONNECT TO $(DBNAME) USER $(DBUSER) USING $(DBPASS)
db2 prep $*.sqc bindfile using $(BNDPATH)/$*.bnd package using $(BINDCD)$* UNSAFENULL YES COMPATIBILITY_MODE ORA
db2 bind $(BNDPATH)/$*.bnd
db2 quit
