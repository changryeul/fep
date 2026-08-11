##########################################################################
#	Module	: makefile - library
#	File	: Make_Lib_P_c.mk
##########################################################################

.SUFFIXES: .c .o

LIB= $(_P_LIB)/$(LIB_TMP)

INCD= $(_P_INC)/fep_sub.h\
	$(_P_INC)/fep_file.h\
	$(_P_INC)/fep_fepp.h\
	$(_P_INC)/def_error.h\
	$(_P_INC)/shm_memory.h\
	$(_P_INC)/fep_interface.h\
	$(_P_INC)/fep_tcpip.h\
	$(_P_INC)/buf_struct.h

INCS= -I$(_P_INC)
SRC= $(_P_SUB)/$(SRC_TMP)
OBJ= $(_PSUB_OBJ)/$(SRC_TMP:.c=.o)
DEF= $(DEF_TMP)

all: $(LIB)
	$(SLEEP_TMP)

$(LIB): $(OBJ)
	ar -rv $(LIB) $(OBJ)

$(OBJ): $(SRC) $(INCD)
	$(CC) -o $(OBJ) -c $(SRC) $(DEF) $(INCS)

##########################################################################
#	End of File (Make_Lib_P_c.mk)
##########################################################################
