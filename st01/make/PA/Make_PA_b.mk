##########################################################################
#   Module  : makefile - PA - b (DB)
#   File    : Make_PA_b.mk
##########################################################################

PROC_FLAGS= SQLCHECK=FULL PARSE=NONE    \
            userid=$(ORACLE_UID)/$(ORACLE_PWD)@$(ORACLE_TNS)

CC=/usr/bin/cc
PROC=$(ORACLE_HOME)/bin/proc
ECHO=/usr/bin/echo

PROCFLAGS = include=$(_P_INC) $(PROC_FLAGS)
DBLIBS= -L$(ORACLE_HOME)/lib -lclntsh
INCFLAGS = -I$(ORACLE_INC) -I$(_P_INC)

LIBD= $(_P_LIB)/libfepP.a
LIBS    =   -L$(_P_LIB) -lFepDb   \
			-L$(_P_LIB) -lfepP    \
			-lnsl -lm       \
			-L$(ORACLE_HOME)/lib -lclntsh -l$(SQL_VER) \
			-L/NEOKM/lib/platform -lcryptuserora \
			`cat $(ORACLE_HOME)/lib/sysliblist`

DEF= $(DEF_TMP) -I$(_P_INC) -I$(ORACLE_HOME)/precomp/public -L$(ORACLE_HOME)/lib -l clntsh

INCD= $(_P_INC)/fep_sub.h\
	$(_P_INC)/fep_file.h\
	$(_P_INC)/fep_fepp.h\
	$(_P_INC)/def_error.h\
	$(_P_INC)/shm_memory.h\
	$(_P_INC)/fep_interface.h\
	$(_P_INC)/fep_tcpip.h\
	$(_P_INC)/pa_struct.h\
	$(_P_INC)/buf_struct.h

CFLAGS  = -g -O -w -I$(INCFLAGS)

INCS= -I$(_P_INC)
TARGET   =$(FEP_SRC_TMP)/$(SRC_TARGET)
PSRCS    =$(SRC_TARGET)
PSRCC    =$(FEP_SRC_TMP)/$(PSRCS:.pc=.c)
POBJS    =$(FEP_OBJ_TMP)/$(PSRCS:.pc=.o)
RUN      =$(_P_BIN)/$(SRC_TARGET)

$(TARGET):  $(TARGET).o
	@echo "--- START 05 ----"
	$(CC) $(CFLAGS) -o $(RUN) $(TARGET).o $(LIBS)

	mv $(TARGET).o $(FEP_OBJ_TMP)/
	rm -f $(TARGET).c $(TARGET).lis
	@echo "--- START 06 ----"

.SUFFIXES: .pc .c .o

.pc.c:
	@echo "--- START 01 ----"
	$(PROC) $(PROCFLAGS) iname=$*.pc oname=$*.c
	@echo "--- START 02 ----"

.pc.o:
	@echo "--- START 03 ----"
	$(PROC) $(PROCFLAGS) iname=$*.pc oname=$*.c
	$(CC) $(CFLAGS) -c $*.c
	mv $(SRC_TARGET).o $(FEP_SRC_TMP)/
	@echo "--- START 04 ----"

##########################################################################
#   End of File (Make_PA_b.mk)
##########################################################################
