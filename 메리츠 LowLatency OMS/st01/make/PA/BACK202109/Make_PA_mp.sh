##########################################################################
#	Module	: make shell - PA - mp (manager)
#	File	: Make_PA_mp.sh
##########################################################################

FEP_SRC_TMP=$_PW_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PA_OBJ;	export FEP_OBJ_TMP

case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP="-lpthread";		export LIB_TMP;;
	SunOS)
		MDEF="-D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS"
		LIB_TMP="-lrt -lpthread";	export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt -D_THREAD_SAFE_ERRNO"
		LIB_TMP="-lpthreads";		export LIB_TMP;;
	Linux)
		MDEF="-D_THREAD_SAFE_ERRNO -D_REENTRANT"
		LIB_TMP="-lpthread";    	export LIB_TMP;;
esac

for i in 1001
do
	SRC_TMP=pw_1000_mp.c
	DEF_TMP=$MDEF
	OBJ_TMP=pa_${i}_mp.o
	RUN_TMP=pa_${i}_mp
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_m.mk
done

case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP="";	export LIB_TMP;;
	SunOS)
		MDEF=""
		LIB_TMP="";	export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt"
		LIB_TMP="";	export LIB_TMP;;
	Linux)
        MDEF=""
        LIB_TMP=""; export LIB_TMP;;
esac

for i in 2001
do
	SRC_TMP=pw_2000_mp.c
	DEF_TMP=$MDEF
	OBJ_TMP=pa_${i}_mp.o
	RUN_TMP=pa_${i}_mp
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_m.mk
done

FEP_SRC_TMP=$_PA_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PA_OBJ;	export FEP_OBJ_TMP

case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP=""; export LIB_TMP;;
	SunOS)
		MDEF=""
		LIB_TMP=""; export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt"
		LIB_TMP=""; export LIB_TMP;;
	Linux)
        MDEF=""
        LIB_TMP=""; export LIB_TMP;;
esac

for i in 1201 1291 1401 1491 6001 7001 7002 7003 7004 9001
do
	case $i in
		1201)
			SRC_TMP=pa_1200_mp.c;;
		1291)
			SRC_TMP=pa_1290_mp.c;;
		1401)
			SRC_TMP=pa_1400_mp.c;;
		1491)
			SRC_TMP=pa_1490_mp.c;;
		6001)
			SRC_TMP=pa_6000_mp.c;;
		7001|7002|7003|7004)
			SRC_TMP=pa_7000_mp.c;;
		9001)
			SRC_TMP=pa_9000_mp.c;;
	esac

	DEF_TMP=$MDEF" -DA${i}"
	OBJ_TMP=pa_${i}_mp.o
	RUN_TMP=pa_${i}_mp
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_m.mk
done

for i in 5011 5021 5022 5023 5024 5025 5026\
		5031 5032 5041 5042\
		5051 5052 5053 5054 5055 5056 5061 5062\
		5071 5081\
		5091 5092 5093 5094 5095 5096 5101 5102 5103 5104 5105 5106
do
	case	$i in
		5011)
			SRC_TMP=pa_5010_mp.c;;
		5021|5022|5023|5024|5025|5026)
			SRC_TMP=pa_5020_mp.c;;
		5031|5032)
			SRC_TMP=pa_5030_mp.c;;
		5041|5042)
			SRC_TMP=pa_5040_mp.c;;
		5051|5052|5053|5054|5055|5056)
			SRC_TMP=pa_5050_mp.c;;
		5061|5062)
			SRC_TMP=pa_5060_mp.c;;
		5071)
			SRC_TMP=pa_5070_mp.c;;
		5081)
			SRC_TMP=pa_5080_mp.c;;
		5091|5092|5093|5094|5095|5096)
			SRC_TMP=pa_5090_mp.c;;
		5101|5102|5103|5104|5105|5106)
			SRC_TMP=pa_5100_mp.c;;
	esac

	DEF_TMP=$MDEF" -DA${i}"
	OBJ_TMP=pa_${i}_mp.o
	RUN_TMP=pa_${i}_mp
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_m.mk
done

# ELW Ã³¸®
for i in 3201 3291 3401 3491 6301 9301
do
	case $i in
		3201)
			SRC_TMP=pa_3200_mp.c;;
		3291)
			SRC_TMP=pa_3290_mp.c;;
		3401)
			SRC_TMP=pa_3400_mp.c;;
		3491)
			SRC_TMP=pa_3490_mp.c;;
		6301)
			SRC_TMP=pa_6300_mp.c;;
		9301)
			SRC_TMP=pa_9300_mp.c;;
	esac

	DEF_TMP=$MDEF" -DA${i}"
	OBJ_TMP=pa_${i}_mp.o
	RUN_TMP=pa_${i}_mp
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_m.mk
done

for i in 5611 5621 5622
do
	case	$i in
		5611)
			SRC_TMP=pa_5610_mp.c;;
		5621|5622)
			SRC_TMP=pa_5620_mp.c;;
	esac

	DEF_TMP=$MDEF" -DA${i}"
	OBJ_TMP=pa_${i}_mp.o
	RUN_TMP=pa_${i}_mp
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_m.mk
done

##########################################################################
#	End of File (Make_PA_mp.sh)
##########################################################################
