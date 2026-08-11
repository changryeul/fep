##########################################################################
#	Module	: make shell - PA - ts (TCP send)
#	File	: Make_PA_ts.sh
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
		LIB_TMP="-lnsl";    				export LIB_TMP;;
esac

for i in 1536 1537 1539\
	1231 1232 1233 1234 1235 1236 1237 1238 1239 1240\
	1241 1242 1243 1244 1245 1246 1247 1248 1249 1250\
	1431 1432 1433 1434 1435 1436 1437 1438 1439 1440\
	1441 1442 1443 1444 1445 1446 1447 1448 1449 1450\
	1031 1032 1033 1034 1035 1036 1037 1038 1039 1040\
	1041 1042 1043 1044 1045 1046 1047 1048 1049 1050\
	6101 6501\
	6310 6320 6330 6340 6350 6360 6370 6380 6390 6400\
	6410 6420 6430 6440 6450 6460 6470 6480 6490 6500
do
	case $i in
		1536|1537|1539)
			SRC_TMP=pw_3100_ts.c	# FEP 접속, Async, client
									# 1536(신규), 1537(취소,정정)
									# 1539(MC)
			DEF_TMP=$MDEF" -DA${i}";;
		6101|6501)
			SRC_TMP=pw_3600_ts.c	# FEP 접속, client
			DEF_TMP=$MDEF" -DSAM_USE -DA${i}";;
		1231|1232|1233|1234|1235|1236|1237|1238|1239|1240|\
		1241|1242|1243|1244|1245|1246|1247|1248|1249|1250|\
		1431|1432|1433|1434|1435|1436|1437|1438|1439|1440|\
		1441|1442|1443|1444|1445|1446|1447|1448|1449|1450|\
		1031|1032|1033|1034|1035|1036|1037|1038|1039|1040|\
		1041|1042|1043|1044|1045|1046|1047|1048|1049|1050|\
		6310|6320|6330|6340|6350|6360|6370|6380|6390|6400|\
		6410|6420|6430|6440|6450|6460|6470|6480|6490|6500)
			SRC_TMP=pw_4000_ts.c	# HATS-C 접속, server
			DEF_TMP=$MDEF" -DSAM_USE -DA${i}";;
	esac

	OBJ_TMP=pa_${i}_ts.o
	RUN_TMP=pa_${i}_ts
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_t.mk
done

# ELW 
for i in 1566 1567 1569\
	6610 6620 6630 6640 6650 6660 6670 6680 6690 6700
do
	case $i in
		1566|1567|1569)
			SRC_TMP=pw_3100_ts.c	# FEP 접속, Async, client
									# 1566(신규), 1567(취소,정정)
									# 1569(Client)
			DEF_TMP=$MDEF" -DA${i}";;
		6610|6620|6630|6640|6650|6660|6670|6680|6690|6700)
			SRC_TMP=pw_4000_ts.c	# HATS-C 접속, server
			DEF_TMP=$MDEF" -DSAM_USE -DA${i}";;
	esac

	OBJ_TMP=pa_${i}_ts.o
	RUN_TMP=pa_${i}_ts
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PA_MAKE}/Make_PA_t.mk
done

##########################################################################
#	End of File (Make_PA_ts.sh)
##########################################################################
