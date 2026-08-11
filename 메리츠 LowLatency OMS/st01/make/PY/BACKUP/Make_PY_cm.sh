#!/bin/sh
##########################################################################
#	Module	: make shell - PY (¿î¿µ) - cm (manager)
#	File	: Make_PY_cm.sh
##########################################################################

cd $_PY_SRC

INC_TMP="$""(_P_INC)/shm_memory.h ""$""(_P_INC)/py_cm.h";	export INC_TMP
LIB_TMP="-lcurses";		export LIB_TMP
MAIN_TMP=py_main_cm;	export MAIN_TMP

case $osname in
	HP-UX)
		DEF_TMP="-Ae +DAportable -D_PSTAT64";	export DEF_TMP;;
	SunOS)
		DEF_TMP="-lsocket -lnsl";				export DEF_TMP;;
	AIX)
		DEF_TMP="-O -qcpluscmt";				export DEF_TMP;;
esac

for i in *.c
do
	if [ $i = "py_main_cm.c" ]; then
		continue
	fi

	echo ${i}
	SRC_TMP=${i};	export SRC_TMP
	make -f $_PY_MAKE/Make_PY_o.mk
done

sleep 1

for i in *.c
do
	if [ $i = "py_main_cm.c" ]; then
		continue
	fi

	oname=`echo $i|cut -c1-10 2>/dev/null`
	OBJ_TMP="${OBJ_TMP} ${_PY_OBJ}/${oname}.o"
done

export OBJ_TMP

make -f $_PY_MAKE/Make_PY_c.mk

##########################################################################
#	End of File (Make_PY_cm.sh)
##########################################################################
