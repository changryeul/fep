# !/bin/sh
##########################################################################
#   Module  : make shell - PA (HATS System) - dd (divide)
#   File    : Make_PA_dd.sh
##########################################################################

FEP_SRC_TMP=$_PA_SRC;   export FEP_SRC_TMP
FEP_OBJ_TMP=$_PA_OBJ;   export FEP_OBJ_TMP

case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP="";     	export LIB_TMP;;
	SunOS)
		MDEF=""
		LIB_TMP="-lrt";		export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt"
		LIB_TMP="";     	export LIB_TMP;;
	Linux)
		MDEF=""
		LIB_TMP="-lrt";		export LIB_TMP;;
esac

for i in 1537 1539 3567 6101 6201 6301\
	7101 7102 7201 7202 7203\
	7501 7502 7503 7504\
	7520 7521 7522 7523 7524\
	7530 7531 7532 7533 7534\
	7540 7541 7542 7543 7544\
	7620 7621 7630 7631 7640 7641
do
	case $i in
		6101|6301)
			SRC_TMP=pa_6100_dd.c;;		# 조회분배, 6301(ELW)
		6201)
			SRC_TMP=pa_6200_dd.c;;		# 조회처리결과분배
		1537|1539|3567)
			SRC_TMP=pa_1500_dd.c;;		# 자동주문 Client송신분배, 3567(ELW)
		7101|7102|7201|7202|7203)
			SRC_TMP=pa_7100_dd.c;;
		7501|7502|7504 | 7520|7521|7522|7523|7524 | 7530|7531|7532|7533|7534 | 7540|7541|7542|7543|7544 | 7620|7621|7630|7631|7640|7641)
			SRC_TMP=pa_7500_dd.c;;
	esac

	DEF_TMP=$MDEF" -DA${i}"
	OBJ_TMP=pa_${i}_dd.o
	RUN_TMP=pa_${i}_dd
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_d.mk
done

##########################################################################
#   End of File (Make_PA_dd.sh)
##########################################################################
