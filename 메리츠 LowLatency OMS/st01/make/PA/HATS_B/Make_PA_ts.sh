##########################################################################
#	Module	: make shell - PA - ts (TCP send)
#	File	: Make_PA_ts.sh
##########################################################################

FEP_SRC_TMP=$_PW_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PA_OBJ;	export FEP_OBJ_TMP

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

for i in 1531 1532 1533 1534 1535 1536 1537 1538 1539 1540\
	1541 1542 1543 1544 1545 1546 1547 1548 1549 1550\
	1310 1320 1330 1340 1350 1360 1370 1380 1390 1400 1410\
	1420 1430 1440 1450 1460 1470 1480 1490 1500\
	6101 6310 6320 6330 6340 6350 6360 6370 6380 6390 6400\
	6410 6420 6430 6440 6450 6460 6470 6480 6490 6500
do
	case $i in
		1531|1532|1533|1534|1535|1536|1537|1538|1539|1540|\
		1541|1542|1543|1544|1545|1546|1547|1548|1549|1550)
			SRC_TMP=pw_3100_ts.c	# FEP 立加, client
			DEF_TMP=$MDEF" -DA${i}";;
		6101)
			SRC_TMP=pw_3600_ts.c	# FEP 立加, client
			DEF_TMP=$MDEF" -DSAM_USE";;
		1310|1320|1330|1340|1350|1360|1370|1380|1390|1400|1410|\
		1420|1430|1440|1450|1460|1470|1480|1490|1500|\
		6310|6320|6330|6340|6350|6360|6370|6380|6390|6400|\
		6410|6420|6430|6440|6450|6460|6470|6480|6490|6500)
			SRC_TMP=pw_4000_ts.c	# PK-C 立加, server
			DEF_TMP=$MDEF" -DSAM_USE";;
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
