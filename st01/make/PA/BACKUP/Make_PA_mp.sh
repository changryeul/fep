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
esac

for i in 1201 2201 1401 2401 6001 9001
do
	case $i in
		6001)
			SRC_TMP=pa_6000_mp.c;;
		1201|2201)
			SRC_TMP=pa_1200_mp.c;;
		1401|2401)
			SRC_TMP=pa_1400_mp.c;;
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

for i in 5011 5012 5013 5014 5015 5016 5021 5022 5023 5024 5025 5026\
		5031 5032 5033 5034 5035 5036 5041 5042 5043 5044 5045 5046\
		5051 5052 5053 5054 5055 5056 5061 5062 5063 5064 5065 5066\
		5071 5072 5073 5074 5075 5076 5081 5082 5083 5084 5085 5086\
		5091 5092 5093 5094 5095 5096 5101 5102 5103 5104 5105 5106
do
	case	$i in
		5011|5012|5013|5014|5015|5016)
			SRC_TMP=pa_5010_mp.c;;
		5021|5022|5023|5024|5025|5026)
			SRC_TMP=pa_5020_mp.c;;
		5031|5032|5033|5034|5035|5036)
			SRC_TMP=pa_5030_mp.c;;
		5041|5042|5043|5044|5045|5046)
			SRC_TMP=pa_5040_mp.c;;
		5051|5052|5053|5054|5055|5056)
			SRC_TMP=pa_5050_mp.c;;
		5061|5062|5063|5064|5065|5066)
			SRC_TMP=pa_5060_mp.c;;
		5071|5072|5073|5074|5075|5076)
			SRC_TMP=pa_5070_mp.c;;
		5081|5082|5083|5084|5085|5086)
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

##########################################################################
#	End of File (Make_PA_mp.sh)
##########################################################################
