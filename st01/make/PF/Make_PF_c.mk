##########################################################################
#   Module  : makefile - PF - c (generic compile+link, 개선 빌드모델)
#   File    : Make_PF_c.mk
#
#   Make_PF_c.mk 와 동일 규칙. 컴파일/링크 분리, INISAFE는 NO_INISAFE 가드.
##########################################################################

.SUFFIXES: .c .o

LIBD= $(_P_LIB)/libfepP.a
HOME_DIR := $(HOME)

ifdef NO_INISAFE
INI_LIB_PATH =
INI_LIBS =
INI_INCS =
INI_DEF = -DNO_INISAFE
else
INI_LIB_PATH = $(HOME_DIR)/krx/INISAFE_Net_for_C_v7.2.47_64/lib
INI_LIBS = -L$(INI_LIB_PATH) -linisafeNet -liniCore -liniPKI -liniCrypto -liniPaccel
INI_INCS = -I$(HOME_DIR)/krx/INISAFE_Net_for_C_v7.2.47_64/include
INI_DEF =
endif

LIBS= -L${_P_LIB} -lfepP $(INI_LIBS) $(LIB_TMP) -ldl -lpthread

INCS= -I${_P_INC} $(INI_INCS)
SRC= $(FEP_SRC_TMP)/$(SRC_TMP)
OBJ= $(FEP_OBJ_TMP)/$(OBJ_TMP)
RUN= $(_P_BIN)/$(RUN_TMP)
DEF= $(DEF_TMP) $(INI_DEF)

all: $(RUN)

$(RUN): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LIBS)
	@echo "Linking Complete: $@"

$(OBJ): $(SRC) $(LIBD)
	$(CC) -o $@ -c $(SRC) $(INCS) $(DEF)
	@echo "Compilation Complete: $@"

clean:
	rm -f $(OBJ) $(RUN)

##########################################################################
#	End of File (Make_PF_c.mk)
##########################################################################
