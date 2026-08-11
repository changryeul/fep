# !/bin/sh
##########################################################################
#	System	: P.S.H HATS system
#	Module	: make shell - PB (急拱可记) - tr (TCP receive)
#	File	: Make_PB_tr.sh
##########################################################################

FEP_SRC_TMP=$_PW_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PB_OBJ;	export FEP_OBJ_TMP

case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP="-lxnet";	export LIB_TMP;;
	SunOS)
		MDEF=""
		LIB_TMP="-lsocket -lnsl -lxnet";	export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt"
		LIB_TMP="-lxnet";	export LIB_TMP;;
esac

for i in 1131 1132 1133 1134 1135 1136 1137 1138 1139 1140\
		1141 1142 1143 1144 1145 1146 1147 1148 1149 1150\
		1536 1537\
		5310 5320 5330 5340 5350 5360 5370 5380 5390 5400\
		5410 5420 5430 5440 5450 5460 5470 5480 5490 5500\
		6501
do
	case $i in
		1536|1537)
			SRC_TMP=pw_3030_tr.c;;	# PK (NO-ACK)
		5310|5320|5330|5340|5350|5360|5370|5380|5390|5400|\
		5410|5420|5430|5440|5450|5460|5470|5480|5490|5500|\
		6501)
			SRC_TMP=pw_3000_tr.c;;
		*)
			SRC_TMP=pw_3010_tr.c;;	# HATS-Client(林巩立加)
	esac

	case $i in
		5310|5320|5330|5340|5350|5360|5370|5380|5390|5400|\
		5410|5420|5430|5440|5450|5460|5470|5480|5490|5500|\
		6501)
			DEF_TMP=$MDEF" -DB${i} -DPROC_MAX=0 -DCHK_TS";;
		*)
			DEF_TMP=$MDEF" -DB${i} -DPROC_MAX=1 -DMEM_USE -DCHK_TS -DKRX_USE";;
	esac

	OBJ_TMP=pb_${i}_tr.o
	RUN_TMP=pb_${i}_tr
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PB_MAKE}/Make_PB_t.mk
done

FEP_SRC_TMP=$_PB_SRC;   export FEP_SRC_TMP

for i in 1413 1414 1415 1416 1417 1418
do
	SRC_TMP=pb_1400_tr.c

	#DEF_TMP=$MDEF" -DB${i} -DHOLIDAY_TEST -DHATS"
	DEF_TMP=$MDEF" -DB${i} -DHATS"
	OBJ_TMP=pb_${i}_tr.o
	RUN_TMP=pb_${i}_tr
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PB_MAKE}/Make_PB_t.mk
done

##########################################################################
#	End of File (Make_PB_tr.sh)
##########################################################################
