##########################################################################
#	Module	: makefile - PB (선물옵션) - d (divide)
#	File	: Make_PB_d.mk
##########################################################################

.SUFFIXES: .c .o

LIBD= $(_P_LIB)/libfepP.a

LIBS= -L$(_P_LIB) -lfepP $(LIB_TMP)

INCD= $(_P_INC)/fep_sub.h\
	$(_P_INC)/fep_file.h\
	$(_P_INC)/fep_fepp.h\
	$(_P_INC)/def_error.h\
	$(_P_INC)/shm_memory.h\
	$(_P_INC)/fep_interface.h\
	$(_P_INC)/fep_tcpip.h\
	$(_P_INC)/pb_struct.h\
	$(_P_INC)/buf_struct.h

INCS= -I$(_P_INC)
SRC= $(FEP_SRC_TMP)/$(SRC_TMP)
OBJ= $(FEP_OBJ_TMP)/$(OBJ_TMP)
RUN= $(_P_BIN)/$(RUN_TMP)
DEF= $(DEF_TMP)

all: $(RUN)

$(RUN): $(SRC) $(LIBD) $(INCD)
	$(CC) -o $(OBJ) -c $(SRC) $(LIBS) $(INCS) $(DEF)
	$(CC) -o $@ $(OBJ) $(LIBS)

##########################################################################
#	End of File (Make_PB_d.mk)
##########################################################################
