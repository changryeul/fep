##########################################################################
#   Module  : makefile - PB - t (TCP)
#   File    : Make_PB_t.mk
##########################################################################

.SUFFIXES: .c .o

LIBD= $(_P_LIB)/libfepP.a
HOME_DIR := $(HOME)

# NO_INISAFE=1 이면 INISAFE 라이브러리 제외
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

INCD= $(_P_INC)/fep_sub.h\
    $(_P_INC)/fep_file.h\
    $(_P_INC)/fep_fepp.h\
    $(_P_INC)/def_error.h\
    $(_P_INC)/shm_memory.h\
    $(_P_INC)/fep_interface.h\
    $(_P_INC)/fep_tcpip.h\
    $(_P_INC)/buf_struct.h

INCS= -I${_P_INC} $(INI_INCS)
SRC= $(FEP_SRC_TMP)/$(SRC_TMP)
OBJ= $(FEP_OBJ_TMP)/$(OBJ_TMP)
RUN= $(_P_BIN)/$(RUN_TMP)
DEF= $(DEF_TMP) $(INI_DEF)

all: $(RUN)

# 컴파일과 링크를 명확히 분리
$(RUN): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LIBS)
	@echo "Linking Complete: $@"

$(OBJ): $(SRC) $(LIBD) $(INCD)
	$(CC) -o $@ -c $(SRC) $(INCS) $(DEF)
	@echo "Compilation Complete: $@"

clean:
	rm -f $(OBJ) $(RUN)

##########################################################################
#	End of File (Make_PB_t.mk)
##########################################################################
