##########################################################################
#   Module  : environment variables and aliases
#   File    : pkg_env.sh
##########################################################################

export  host_name=`hostname`
if [ $host_name = "podm12" ]; then
    export  _FEP_DIV="REAL2"
elif [ $host_name = "podm11" ]; then
    export  _FEP_DIV="REAL1"
else
    export  _FEP_DIV="TEST"
fi
export  _FEP_HOME=$HOME
export  _FEP_SYSTEM="p"
export  _FEP_DIR1="bin cfg inc lib shl utl sub env"
export  _FEP_DIR11="src obj make"
export  _FEP_DIR2="DAT FIFO"
export  _FEP_DIR3="LOG"
export  _FEP_SUBDIR="A X Z"
export  _FEP_TMP=/tmp
export  osname=`uname -s 2>/dev/null`
export  subname="a x z"
export  LANG=C

case $osname in
    HP-UX)
        export  LC_CTYPE=C.iso88591
        ;;
    SunOS)
        export  LC_CTYPE=iso_8859_1
        ;;
    AIX)
        export  LC_CTYPE=en_US.ISO8859-1
        ;;
    Linux)
        export  LANG=en_US.UTF-8
        export  LC_CTYPE=en_US.UTF-8
#	   export  LC_CTYPE=en_US.iso_88591
esac

##########################################################################
# Environment Variables
##########################################################################
for s in $_FEP_SYSTEM
do
    typeset -u us=${s}

    for h in 1 2 3
    do
        case ${h} in
        1)
            for d in $_FEP_DIR1
            do
                typeset -u ud=${d}
                export _${us}_${ud}=$_FEP_HOME/st0${h}/${d}
            done
            for d in $_FEP_DIR11
            do
                typeset -u ud=${d}
                export _${us}_${ud}=$_FEP_HOME/st0${h}/${d}
                for sd in $_FEP_SUBDIR
                do
                    export _${us}${sd}_${ud}=$_FEP_HOME/st0${h}/${d}/${us}${sd}
                done
                case ${d} in
                    obj|make)
                        export _${us}SUB_${ud}=$_FEP_HOME/st0${h}/${d}/SUB;;
                esac
            done;;
        2)
            for d in $_FEP_DIR2
            do
                export _${us}_${d}=$_FEP_HOME/st0${h}/${d}
                for sd in $_FEP_SUBDIR
                do
                    export _${us}${sd}_${d}=$_FEP_HOME/st0${h}/${d}/${us}${sd}
                done
            done;;
        3)
            for d in $_FEP_DIR3
            do
                export _${us}_${d}=$_FEP_HOME/st0${h}/${d}
                for sd in $_FEP_SUBDIR
                do
                    export _${us}${sd}_${d}=$_FEP_HOME/st0${h}/${d}/${us}${sd}
                done
            done;;
        esac
    done
done

##########################################################################
# Aliases
##########################################################################
for s in $_FEP_SYSTEM
do
    typeset -u us=${s}
    typeset -l ls=${s}

    for d in $_FEP_DIR1 $_FEP_DIR11 $_FEP_DIR2 $_FEP_DIR3
    do
        typeset -u ud=${d}
        typeset -l ld=${d}
        aval="${ls}${ld}"
        alias $aval="cd ""$""_${us}_${ud}"
        case ${d} in
            obj|make)
                aval="${ls}${ld}s"
                alias $aval="cd ""$""_${us}_${ud}/SUB";;
        esac
    done
    for d in $_FEP_DIR11 $_FEP_DIR2 $_FEP_DIR3
    do
        typeset -u ud=${d}
        typeset -l ld=${d}
        for sd in $_FEP_SUBDIR
        do
            typeset -l lsd=${sd}
            aval="${ls}${lsd}${ld}"
            alias $aval="cd ""$""_${us}${sd}_${ud}"
        done
    done
    for d in $_FEP_DIR2 $_FEP_DIR3
    do
        typeset -u ud=${d}
        typeset -l ld=${d}
        for sd in $_FEP_SUBDIR
        do
            typeset -l lsd=${sd}
            aval="${ls}${lsd}d${ld}"
            if [ ${ld} = "log" ] ; then
                if [ ${lsd} = "w" ] || [ ${lsd} = "x" ] ||\
                    [ ${lsd} = "y" ] || [ ${lsd} = "z" ]
                then
                    alias $aval="cd ""$""_${us}${sd}_${ud}/"'`date +%Y%m%d`'
                else
                    alias $aval="cd ""$""_${us}${sd}_${ud}/00000000"
                fi
            elif [ ${ld} = "dat" ] ; then
                if [ ${lsd} = "w" ] || [ ${lsd} = "x" ] || [ ${lsd} = "y" ]
                then
                    alias $aval="cd ""$""_${us}${sd}_${ud}/"'`date +%Y%m%d`'
                else
                    alias $aval="cd ""$""_${us}${sd}_${ud}/00000000"
                fi
            else
                alias $aval="cd ""$""_${us}${sd}_${ud}"
            fi
        done
    done
done

alias   cls="clear"
alias   ll="ls -al"
alias   rm="rm -i"
alias   h='history'
alias   who='who -T'
alias   chkcron='cat /var/spool/cron/crontabs/*fepp'

alias   ${ls}seq="cd "${_FEP_HOME}"/st03/SEQ"
alias   ${ls}seqb="cd "${_FEP_HOME}"/st03/SEQ/BATCH"
alias   ${ls}seqd="cd "${_FEP_HOME}"/st03/SEQ/DISC"
alias   ${ls}seqt="cd "${_FEP_HOME}"/st03/SEQ/TOTAL"

for s in $subname
do
    alias   ${ls}${s}all="ps.sh "${ls}${s}
    alias   ${ls}${s}cnt="chkcnt.sh "${ls}${s}
    alias   ${ls}${s}p="chkproc.sh "${ls}${s}
    alias   ${ls}${s}t="chktcp1.sh "${ls}${s}
    alias   ${ls}${s}t2="chktcp2.sh "${ls}${s}
    alias   ${ls}${s}u="chkudp.sh "${ls}${s}
done

alias   ${ls}all=ps.sh
alias   ${ls}yall="ps.sh "${ls}y
alias   ${ls}zall="ps.sh "${ls}"z;ps.sh|grep daemon"

##########################################################################
#   End of File (pkg_env.sh)
##########################################################################
