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

#for i in 7101 7201 7301 7401 7801 7901 7902 7905 7906\
#	7111 7211 7212 7311 7411 7412 7811 7911 7912 7915 7916\
#	7001 7501 7502 7601 7511 7512 7521 7522 7611 7612 7621 7622 7711 7712 7721 7722\
#	7191 7192 7193 7291 7292 7293 7391 7491 7891 7991 7992 7392 7492 7892 7996 7997 6591 6691 6592 6692
#do
#	case $i in
#		7101|7201|7301|7401|7801|7901|7902|7905|7906|7111|7211|7212|7311|7411|7412|7811|7911|7912|7915|7916)
#			SRC_TMP=pa_7100_dd.c;;
#		7001|7501|7502|7601|7511|7512|7521|7522|7611|7612|7621|7622|7711|7712|7721|7722)
#			SRC_TMP=pa_7500_dd.c;;
## 파생 내부포트
#		7191|7192|7193|7291|7292|7293|7391|7491|7891|7991|7992|7392|7492|7892|7996|7997|6591|6691|6592|6692)
#			SRC_TMP=pa_7111_dd.c;;
#	esac
#
#	DEF_TMP=$MDEF" -DA${i}"
#	OBJ_TMP=pa_${i}_dd.o
#	RUN_TMP=pa_${i}_dd
#	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
#	echo $RUN_TMP
#	make -f ${_PA_MAKE}/Make_PA_d.mk
#done

# 7801 7802 => 7181 7182
for i in 7102 7103 7201 7202 7203 7801 7802 7803 7181 7182
do
	case $i in
		7181|7182|7102|7103|7201|7202|7203)
			SRC_TMP=pa_7100_dd.c;;
		7801|7802|7803)
			SRC_TMP=pa_7800_dd.c;;
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
