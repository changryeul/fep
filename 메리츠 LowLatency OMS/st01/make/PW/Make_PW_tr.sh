##########################################################################
#	Module	: make shell - PW (TCP업무접속) - tr (TCP receive)
#	File	: Make_PW_tr.sh
##########################################################################

FEP_SRC_TMP=$_PW_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PW_OBJ;	export FEP_OBJ_TMP

case $osname in
	HP-UX)
		LIB_TMP="-lxnet";					export LIB_TMP
		DEF_TMP="-Ae +DAportable";			export DEF_TMP;;
	SunOS)
		LIB_TMP="-lsocket -lnsl -lxnet";	export LIB_TMP
		DEF_TMP="";							export DEF_TMP;;
	AIX)
		LIB_TMP="-O -lxnet";				export LIB_TMP
		DEF_TMP="-qcpluscmt";				export DEF_TMP;;
	Linux)
		LIB_TMP="-lnsl";    				export LIB_TMP
		DEF_TMP="";         				export DEF_TMP;;
esac

for i in 1000 1001 1002 1003 1004 1005 1006 1007 1008 1009\
	2000 2001 2002 2003 2004 2005 2006 2007 2008 2009
do
	case $i in
		1000|2000)
			SRC_TMP=pw_1000_tr.c;;
		*)
			SRC_TMP=pw_2000_tr.c;;
	esac

	OBJ_TMP=pw_${i}_tr.o
	RUN_TMP=pw_${i}_tr
	export SRC_TMP OBJ_TMP RUN_TMP
	echo $RUN_TMP
	make -f ${_PW_MAKE}/Make_PW_t.mk
done

##########################################################################
#	End of File (Make_PW_tr.sh)
##########################################################################
