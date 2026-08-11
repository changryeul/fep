##########################################################################
#	Module	: make shell - PC (파생) - 개선 빌드모델 (1 소스 = 1 바이너리)
#	File	: Make_PC_all.sh
#
#	설계 §6: 시장/프로토콜 차이는 소스분리, 프로세스 인스턴스 차이는 런타임.
#	  - 소스당 -DPCxxxx 다중바이너리 폐기 → src/PC/*.c 하나가 바이너리 하나.
#	  - 프로세스ID/포트/기능코드/전문크기는 proc.ini·file.ini에서 런타임 로드.
#	  - 데몬이 exec -a p{c}_{NNNN}_{type} 로 argv[0] 부여, Init_Proc가 파싱.
#	  - 남는 -D는 플랫폼/기능 플래그(NO_INISAFE, HOLIDAY_*, osname)뿐.
#	src/PC 에 .c 가 없으면 조용히 종료(스캐폴딩 단계 no-op).
##########################################################################

FEP_SRC_TMP=$_PC_SRC;	export FEP_SRC_TMP
FEP_OBJ_TMP=$_PC_OBJ;	export FEP_OBJ_TMP

# --- 플랫폼별 컴파일/링크 플래그 (PB KRX-direct 송수신과 동일 계열) ---
case $osname in
	HP-UX)
		MDEF="-Ae +DAportable"
		LIB_TMP="-lxnet";							export LIB_TMP;;
	SunOS)
		MDEF=""
		LIB_TMP="-lsocket -lnsl -lxnet";			export LIB_TMP;;
	AIX)
		MDEF="-O -qcpluscmt"
		LIB_TMP="-lxnet";							export LIB_TMP;;
	Linux)
		MDEF="-D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE -D_REENTRANT"
		LIB_TMP="-ltirpc -lm";						export LIB_TMP;;
esac

# 기능 플래그(진짜 빌드타임 관심사만 유지). 필요 시 환경변수로 재정의.
FEATURE_DEF="${_PC_FEATURE_DEF:--DSAM_USE -DHOLIDAY_APPLY}"

# F1 order-latency-metrics: _LAT_TRACE=1 이면 계측 빌드 (기본 off)
if [ "${_LAT_TRACE}" = "1" ]; then
	MDEF="$MDEF -DLAT_TRACE"
fi

# --- src/PC/*.c → 동일 basename 바이너리 1:1 빌드 ---
built=0
for src in ${_PC_SRC}/*.c
do
	[ -e "$src" ] || continue          # glob 미매치(소스 없음) → no-op
	base=`basename "$src" .c`          # 예: pc_2100_ts.c → pc_2100_ts
	SRC_TMP=${base}.c
	OBJ_TMP=${base}.o
	RUN_TMP=${base}
	DEF_TMP="$MDEF $FEATURE_DEF"
	export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
	echo "$RUN_TMP"
	make -f ${_PC_MAKE}/Make_PC_c.mk
	built=`expr $built + 1`
done

if [ $built -eq 0 ]; then
	echo "PC: no sources in ${_PC_SRC} (scaffold no-op)"
fi

##########################################################################
#	End of File (Make_PC_all.sh)
##########################################################################
