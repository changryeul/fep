##########################################################################
#	Module	: makefile - PZ (°ü¸®)
#	File	: Make_PZ_m.mk
##########################################################################

.SUFFIXES: .c .o

LIBD= $(_P_LIB)/libfepP.a

#LIBS= -L$(_P_LIB) -lfepP -lisam -lsec	# C-ISAM
LIBS= -L$(_P_LIB) -lfepP

INCD= $(_P_INC)/fep_sub.h\
	$(_P_INC)/fep_file.h\
	$(_P_INC)/fep_fepp.h\
	$(_P_INC)/def_error.h\
	$(_P_INC)/shm_memory.h\
	$(_P_INC)/fep_interface.h\
	$(_P_INC)/buf_struct.h

INCS= -I$(_P_INC)
SRC= $(FEP_SRC_TMP)/$(SRC_TMP)
OBJ= $(FEP_OBJ_TMP)/$(SRC_TMP:.c=.o)
RUN= $(_P_BIN)/$(RUN_TMP)
ROBJ= $(OBJ_TMP)
DEF= $(DEF_TMP)

all: $(OBJ) $(RUN)

$(RUN): $(ROBJ)
	$(CC) -o $@ $(ROBJ) $(LIBS) $(INCS) $(DEF)

$(OBJ): $(SRC) $(INCD) $(LIBD)
	$(CC) -o $(OBJ) -c $(SRC) $(INCS) $(DEF)

##########################################################################
#	End of File (Make_PZ_m.mk)
##########################################################################
