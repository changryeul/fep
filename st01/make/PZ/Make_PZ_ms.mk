##########################################################################
#	Module	: makefile - PZ (����)
#	File	: Make_PZ_ms.mk
##########################################################################

.SUFFIXES: .c .o

LIBD= $(_P_LIB)/libfepP.a

LIBS= -L$(_P_LIB) -lfepP -lsqlite3

INCD= $(_P_INC)/fep_sub.h\
	$(_P_INC)/fep_file.h\
	$(_P_INC)/fep_fepp.h\
	$(_P_INC)/def_error.h\
	$(_P_INC)/shm_memory.h\
	$(_P_INC)/fep_interface.h\
	$(_P_INC)/buf_struct.h

INCS= -I$(_P_INC)
OBJ= $(FEP_OBJ_TMP)/$(RUN_TMP:_mp=.o)
RUN= $(_P_BIN)/$(RUN_TMP)


all: $(RUN)

$(RUN): $(OBJ) $(LIBD) $(INCD)
	$(CC) -o $@ $(OBJ) $(LIBS) 

##########################################################################
#	End of File (Make_PZ_ms.mk)
##########################################################################
