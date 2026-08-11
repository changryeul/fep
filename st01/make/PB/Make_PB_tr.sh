##########################################################################
#   Module  : make shell - PB - tr (TCP receive)
#   File    : Make_PB_tr.sh
##########################################################################

FEP_SRC_TMP=$_PB_SRC;   export FEP_SRC_TMP
FEP_OBJ_TMP=$_PB_OBJ;   export FEP_OBJ_TMP

case $osname in
    HP-UX)
        MDEF="-Ae +DAportable"
        LIB_TMP="-lxnet";                       export LIB_TMP;;
    SunOS)
        MDEF=""
        LIB_TMP="-lsocket -lnsl -lxnet";        export LIB_TMP;;
    AIX)
        MDEF="-O -qcpluscmt"
        LIB_TMP="-lxnet";                       export LIB_TMP;;
    Linux)
        MDEF="-DUSEFX"
        #LIB_TMP="-lnsl -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE"; export LIB_TMP;;
        LIB_TMP="-ltirpc -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE";    export LIB_TMP;;
        #LIB_TMP="-ltirpc -D_DEFAULT_SOURCE";       export LIB_TMP;;
        #LIB_TMP="-ltirpc -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE";       export LIB_TMP;;
esac

# F1 order-latency-metrics: _LAT_TRACE=1 이면 계측 빌드 (기본 off)
if [ "${_LAT_TRACE}" = "1" ]; then
    MDEF="$MDEF -DLAT_TRACE"
fi

for i in 1201 1301 1601 7801 8211
do
    case $i in
        1201|1601)
            SRC_TMP=pb_1200_tr.c;;  # FEP 접속, client, async, 채권(KRX직접접속)
        7801)
            SRC_TMP=pb_7800_tr.c;;  # FEP 접속, client, symc, KRX일괄송신(RDS) KRX직접
        8211)
            SRC_TMP=pb_8200_tr.c;;  # Recv From Client
        1301)
            SRC_TMP=pb_7200_tr.c;;  # Recv From OMS
    esac

    DEF_TMP=$MDEF" -DSAM_USE -DB${i} -DHOLIDAY_CHECK"

    OBJ_TMP=pb_${i}_tr.o
    RUN_TMP=pb_${i}_tr
    export SRC_TMP DEF_TMP OBJ_TMP RUN_TMP
    echo $RUN_TMP
    make -f ${_PB_MAKE}/Make_PB_t.mk
done

##########################################################################
#   End of File (Make_PB_tr.sh)
##########################################################################
