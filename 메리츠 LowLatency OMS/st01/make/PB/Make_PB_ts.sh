# !/bin/sh
##########################################################################
#	System	: P.S.H HATS system
#	Module	: make shell - PB (急拱可记) - ts (TCP send)
#	File	: Make_PB_ts.sh
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

for i in 1252 1293 1493 1532\
		1031 1032 1033 1034 1035 1036 1037 1038 1039 1040\
		1041 1042 1043 1044 1045 1046 1047 1048 1049 1050\
		1452
do
	case $i in
		1532)
			SRC_TMP=pw_3100_ts.c		# FEP 立加, Async, client
										# 1532(秒家,沥沥)
			DEF_TMP=$MDEF" -DB${i}";;
		*)
			SRC_TMP=pw_4000_ts.c		# PK-C 立加, server
			DEF_TMP=$MDEF" -DSAM_USE -DB${i}";;
	esac

	OBJ_TMP=pb_${i}_ts.o
	RUN_TMP=pb_${i}_ts
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PB_MAKE}/Make_PB_t.mk
done

FEP_SRC_TMP=$_PB_SRC;   export FEP_SRC_TMP

for i in 1113 1114 1115 1116 1117 1118
do
	SRC_TMP=pb_1100_ts.c

	#DEF_TMP=$MDEF" -DB${i} -DHOLIDAY_TEST -DHATS"
	DEF_TMP=$MDEF" -DB${i} -DHATS"
	OBJ_TMP=pb_${i}_ts.o
	RUN_TMP=pb_${i}_ts
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PB_MAKE}/Make_PB_t.mk
done

##########################################################################
#	End of File (Make_PB_ts.sh)
##########################################################################
