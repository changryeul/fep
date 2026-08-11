##########################################################################
#	Module	: makefile - PY (¿î¿µ) - c (main)
#	File	: Make_PY_c.mk
##########################################################################

.SUFFIXES: .c .o

INCD= $(INC_TMP)
INCS= -I$(_P_INC)

SRC= $(MAIN_TMP).c
OBJ= $(_PY_OBJ)/$(MAIN_TMP).o
RUN= $(_P_BIN)/$(MAIN_TMP)
DEF= $(DEF_TMP)
LIBS= $(LIB_TMP)

all: $(RUN)

$(RUN): $(SRC) $(INCD) $(OBJ_TMP)
	$(CC) -o $(OBJ) -c $(SRC) $(LIBS) $(INCS) $(DEF)
	$(CC) -o $@ $(OBJ) $(OBJ_TMP) $(LIBS) $(DEF)

##########################################################################
#	End of File (Make_PY_c.mk)
##########################################################################
