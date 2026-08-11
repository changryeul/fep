
CFLAGS	= -O -Ae +DA1.0 +DS1.0 -DUNIX -D_REENTRANT -D_DISAM_DEFINE -D_TR_IMPROVE  
INCLUDE = -I$(_HTS_HOME_)/src/inc
CLIB = -lm 

che_inform  = che_inform.o 
uid         = uid.o
uid_delete  = uid_delete.o
uid_info    = uid_info.o
uid_discord = uid_discord.o

loantrans   = loantrans.o loantrlib.o tr_9908.o

libobj = $(_HTS_HOME_)/src/c.obj/archlib_disam.o 

all : che_inform uid uid_delete loantrans uid_info uid_discord

clean:
	rm -f *.o

install :
	chmod 755 che_inform
	mv che_inform $(_HTS_HOME_)/run/bin
	chmod 755 uid
	mv uid $(_HTS_HOME_)/run/bin
	chmod 755 uid_delete
	mv uid_delete $(_HTS_HOME_)/run/bin
	chmod 755 uid_info
	mv uid_info $(_HTS_HOME_)/run/bin
	chmod 755 uid_discord
	mv uid_discord $(_HTS_HOME_)/run/bin

##------------------------------------------------
.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $*.c

#------------------------------------------------
che_inform : $(che_inform)
	$(CC) -o $@ $(che_inform) $(libobj) $(CLIB)
##------------------------------------------------
uid : $(uid)
	$(CC) -o $@ $(uid) $(libobj) $(CLIB)
##------------------------------------------------
uid_delete : $(uid_delete)
	$(CC) -o $@ $(uid_delete) $(libobj) $(CLIB)
##------------------------------------------------
loantrans : $(loantrans)
	$(CC) -o $@ $(loantrans) $(libobj) $(CLIB)
##------------------------------------------------
uid_info : $(uid_info)
	$(CC) -o $@ $(uid_info) $(libobj) $(CLIB)
##------------------------------------------------
uid_discord : $(uid_discord)
	$(CC) -o $@ $(uid_discord) $(libobj) $(CLIB)
##------------------------------------------------
#################################################
