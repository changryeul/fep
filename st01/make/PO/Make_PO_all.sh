##########################################################################
#	Module	: make shell - PO (OMS 코어) - 개선 빌드모델 (1 소스 = 1 바이너리)
#	File	: Make_PO_all.sh
#
#	설계 §6: 프로세스 인스턴스 차이는 런타임(proc.ini + argv[0]), -D 다중바이너리 폐기.
#	PO는 시장 무관 크로스커팅(전략/한도/접속/분배/원장). KRX 대면 아님이 기본이나
#	접속서버 등 TCP 사용 → 공통 링크 플래그 유지. src/PO 에 .c 없으면 no-op.
##########################################################################

FEP_SRC_TMP=$_PO_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PO_OBJ;	export FEP_OBJ_TMP

case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP="-lxnet -lpthread";					export LIB_TMP;;
	SunOS)
		MDEF="-D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS"
		LIB_TMP="-lsocket -lnsl -lxnet -lrt -lpthread";	export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt -D_THREAD_SAFE_ERRNO"
		LIB_TMP="-lxnet -lpthreads";				export LIB_TMP;;
	Linux)
		MDEF="-D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE -D_REENTRANT"
		LIB_TMP="-ltirpc -lm";						export LIB_TMP;;
esac

# PO 기능 플래그는 기본 없음(순수 크로스커팅). 필요 시 _PO_FEATURE_DEF로 재정의.
FEATURE_DEF="${_PO_FEATURE_DEF:-}"

if [ "${_LAT_TRACE}" = "1" ]; then
	MDEF="$MDEF -DLAT_TRACE"
fi

built=0
for src in ${_PO_SRC}/*.c
do
	[ -e "$src" ] || continue
	base=`basename "$src" .c`
	SRC_TMP=${base}.c
	OBJ_TMP=${base}.o
	RUN_TMP=${base}
	DEF_TMP="$MDEF $FEATURE_DEF"
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo "$RUN_TMP"
	make -f ${_PO_MAKE}/Make_PO_c.mk
	built=`expr $built + 1`
done

if [ $built -eq 0 ]; then
	echo "PO: no sources in ${_PO_SRC} (scaffold no-op)"
fi

##########################################################################
#	End of File (Make_PO_all.sh)
##########################################################################
