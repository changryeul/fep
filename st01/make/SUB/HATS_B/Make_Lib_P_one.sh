##########################################################################
#	Module	: make shell - library - make only one program
#	File	: Make_Lib_P_one.sh
##########################################################################

case $osname in
	HP-UX)
		DEF_TMP="-Ae +DAportable -D_PSTAT64";	export DEF_TMP;;
	SunOS)
		DEF_TMP="-lnsl -lsocket";	export DEF_TMP;;
	AIX)
		DEF_TMP="-O -lnsl -lsocket -qcpluscmt";	export DEF_TMP;;
esac

cd ${_P_SUB}
LIB_TMP=libfepP.a;	export LIB_TMP
echo $1.c
SRC_TMP=$1.c;	export SRC_TMP
make -f ${_PSUB_MAKE}/Make_Lib_P_c.mk

##########################################################################
#	End of File (Make_Lib_P_one.sh)
##########################################################################
