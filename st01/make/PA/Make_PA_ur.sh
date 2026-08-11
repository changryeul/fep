#!/bin/sh
##########################################################################
#	Module	: make shell - PA - ur (UDP receive)
#	File	: Make_PA_ur.sh
##########################################################################

FEP_SRC_TMP=$_PA_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PA_OBJ;	export FEP_OBJ_TMP

case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP="-lxnet";					export LIB_TMP;;
	SunOS)
		MDEF=""
		LIB_TMP="-lsocket -lnsl -lxnet";	export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt"
		LIB_TMP="-lxnet";					export LIB_TMP;;
	Linux)
		MDEF=""
		LIB_TMP="-ltirpc";    				export LIB_TMP;;
		#LIB_TMP="-ltirpc";    				export LIB_TMP;;
esac

#for i in 6101 6301 6401 6501\
#		6102 6103 6104 6105 6106 6107\
#		6202 6302 6402 6502
#do
#	case $i in
#		6101|6301|6401|6501|6102|6103|6104|6105|6106|6107|6202|6302|6402|6502)
#			SRC_TMP=pa_6100_ur.c
#			DEF_TMP=$MDEF" -DA${i} -DHOLIDAY_CHECK";;
#	esac
#
#	OBJ_TMP=pa_${i}_ur.o
#	RUN_TMP=pa_${i}_ur
#	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
#	echo $RUN_TMP
#	make -f ${_PA_MAKE}/Make_PA_u.mk
#done

#for i in 7101 7201 7301 7401 7801 7901 7902 7905 7906\
#		7111 7211 7212 7311 7411 7412 7811 7911 7912 7915 7916\
#		7001 7501 7601\
#		7511 7512 7521 7522 7611 7612 7621 7622\
#		7711 7712 7721 7722\
#		7191 7192 7193 7291 7292 7293 7391 7491 7891 7991 7992 6591 6691
#do
#	case $i in
#		7101|7201|7301|7401|7801|7901|7902|7905|7906|7111|7211|7212|7311|7411|7412|7811|7911|7912|7915|7916)
#			SRC_TMP=pa_7100_ur.c;;
#		7001|7501|7601|7511|7512|7521|7522|7611|7612|7621|7622|7711|7712|7721|7722)
#			SRC_TMP=pa_7500_ur.c;;
## 파생 내부포트
#		7191|7192|7193|7291|7292|7293|7391|7491|7891|7991|7992|6591|6691)
#			SRC_TMP=pa_7111_ur.c;;
#	esac
#
#	DEF_TMP=$MDEF" -DA${i}"
#	OBJ_TMP=pa_${i}_ur.o
#	RUN_TMP=pa_${i}_ur
#	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
#	echo $RUN_TMP
#	make -f ${_PA_MAKE}/Make_PA_u.mk
#done

for i in 7102 7103 7201 7202 7203 7291
do
	case $i in
		7102|7201|7202|7291)
			SRC_TMP=pa_7100_ur.c;;
	esac
	DEF_TMP=$MDEF" -DA${i}"
	OBJ_TMP=pa_${i}_ur.o
	RUN_TMP=pa_${i}_ur
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_u.mk
done

##########################################################################
#	End of File (Make_PA_ur.sh)
##########################################################################
