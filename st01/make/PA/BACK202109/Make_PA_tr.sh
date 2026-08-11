##########################################################################
#	Module	: make shell - PA - tr (TCP receive)
#	File	: Make_PA_tr.sh
##########################################################################

FEP_SRC_TMP=$_PW_SRC;	export FEP_SRC_TMP
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
		LIB_TMP="-lnsl";					export LIB_TMP;;
esac

for i in 1150 1251 1252 1451 1452 6201\
	5310 5320 5330 5340 5350 5360 5370 5380 5390 5400\
	5410 5420 5430 5440 5450 5460 5470 5480 5490\
	6310 6320 6330 6340 6350 6360 6370 6380 6390 6400\
	6410 6420 6430 6440 6450 6460 6470 6480 6490 6500
do
	case $i in
		1251|1252|1451|1452|6201)
			SRC_TMP=pw_4100_tr.c;;	# FEP 접속, client
		1150|5310|5320|5330|5340|5350|5360|5370|5380|5390|5400|\
		5410|5420|5430|5440|5450|5460|5470|5480|5490|\
		6310|6320|6330|6340|6350|6360|6370|6380|6390|6400|\
		6410|6420|6430|6440|6450|6460|6470|6480|6490|6500)
			SRC_TMP=pw_3000_tr.c;;	# HATS-C 접속, server, 조회(분기됨)
	esac

	case $i in
		1150)
			DEF_TMP=$MDEF" -DMEM_USE -DA${i}";;
		*)
			DEF_TMP=$MDEF" -DSAM_USE -DA${i}";;
	esac

	OBJ_TMP=pa_${i}_tr.o
	RUN_TMP=pa_${i}_tr
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_t.mk
done

# ELW 처리 추가
for i in 1261 1461\
	 3161 3162 3163 3164 3165 3166 3167 3168 3169 3170\
	 6610 6620 6630 6640 6650 6660 6670 6680 6690 6700
do
	case $i in
		1261|1461)
			SRC_TMP=pw_4100_tr.c	# 
			DEF_TMP=$MDEF" -DSAM_USE -DA${i}";;
		3161|3162|3163|3164|3165|3166|3167|3168|3169|3170)
			SRC_TMP=pw_3000_tr.c	# HATS-C 접속, server, 조회(분기됨)
			DEF_TMP=$MDEF" -DMEM_USE -DA${i}";;
		6610|6620|6630|6640|6650|6660|6670|6680|6690|6700)
			SRC_TMP=pw_3000_tr.c	# HATS-C 접속, server, 조회(분기됨)
			DEF_TMP=$MDEF" -DSAM_USE -DA${i}";;
	esac

	OBJ_TMP=pa_${i}_tr.o
	RUN_TMP=pa_${i}_tr
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_t.mk
done

##########################################################################
#	End of File (Make_PA_tr.sh)
##########################################################################
