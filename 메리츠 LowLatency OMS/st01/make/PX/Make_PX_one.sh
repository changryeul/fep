##########################################################################
#	Module	: make shell - PX (Utility) - make only one program
#	File	: Make_PX_one.sh
##########################################################################

FEP_SRC_TMP=$_PX_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PX_OBJ;	export FEP_OBJ_TMP

case $osname in
	HP-UX)
		DEF_TMP="-Ae +DAportable";			export DEF_TMP;;
	SunOS)
		DEF_TMP="-lsocket -lnsl -lxnet";	export DEF_TMP;;
	AIX)
		DEF_TMP="-O -lxnet -qcpluscmt";		export DEF_TMP;;
	Linux)
		DEF_TMP="-lnsl";                    export DEF_TMP;;
esac

cd $_PX_SRC

SRC_TMP=$1.c;	export SRC_TMP
echo $SRC_TMP
make -f ${_PX_MAKE}/Make_PX_m.mk

##########################################################################
#	End of File (Make_PX_one.sh)
##########################################################################
