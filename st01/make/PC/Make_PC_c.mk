##########################################################################
#   Module  : makefile - PC - c (generic compile+link, 개선 빌드모델)
#   File    : Make_PC_c.mk
#
#   Make_PB_t.mk 계열 규칙 재사용. 컴파일과 링크 분리, INISAFE는 NO_INISAFE 가드.
##########################################################################

.SUFFIXES: .c .o

LIBD= $(_P_LIB)/libfepP.a
HOME_DIR := $(HOME)

# NO_INISAFE=1 이면 INISAFE 라이브러리 제외 (개발 기본값)
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

# 최종 라이브러리 결합 (순서: FEP -> Initech -> System)
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
#	End of File (Make_PC_c.mk)
##########################################################################
