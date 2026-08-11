##########################################################################
#   Module  : make default directories
#   File    : pkg_mkdir.sh
#   Notes   : fepp1, fepp2, fepp3가 생성되어 있고 fepp1/.profile과
#               fepp1/env/pkg_env.sh이 준비된 상황에서 실행할 것
##########################################################################

for s in $_FEP_SYSTEM
do
    typeset -u us=${s}
    typeset -l ls=${s}

    for h in 1 2 3
    do
        export _FEP_MKDIR=$_FEP_HOME/st0${h}
        mkdir $_FEP_MKDIR > /dev/null 2> /dev/null

        case ${h} in
            1)
                for d in $_FEP_DIR1 $_FEP_DIR11
                do
                    export _FEP_MKDIR1=$_FEP_MKDIR/${d}
                    mkdir $_FEP_MKDIR1 > /dev/null 2> /dev/null
                    case ${d} in
                        bin)
                            export _FEP_MKDIR2=$_FEP_MKDIR1/bak
                            mkdir $_FEP_MKDIR2 > /dev/null 2> /dev/null;;
                        obj)
                            export _FEP_MKDIR2=$_FEP_MKDIR1/SUB
                            mkdir $_FEP_MKDIR2 > /dev/null 2> /dev/null

                            for sd in $_FEP_SUBDIR
                            do
                                export _FEP_MKDIR2=$_FEP_MKDIR1/${us}${sd}
                                mkdir $_FEP_MKDIR2 > /dev/null 2> /dev/null
                            done;;
                        src)
                            for sd in $_FEP_SUBDIR
                            do
                                export _FEP_MKDIR2=$_FEP_MKDIR1/${us}${sd}
                                mkdir $_FEP_MKDIR2 > /dev/null 2> /dev/null
                            done;;
                        make)
                            export _FEP_MKDIR2=$_FEP_MKDIR1/SUB
                            mkdir $_FEP_MKDIR2 > /dev/null 2> /dev/null

                            for sd in $_FEP_SUBDIR
                            do
                                export _FEP_MKDIR2=$_FEP_MKDIR1/${us}${sd}
                                mkdir $_FEP_MKDIR2 > /dev/null 2> /dev/null
                            done;;
                    esac
                done;;
            2)
                for d in $_FEP_DIR2
                do
                    export _FEP_MKDIR1=$_FEP_MKDIR/${d}
                    mkdir $_FEP_MKDIR1 > /dev/null 2> /dev/null

                    for sd in $_FEP_SUBDIR
                    do
                        export _FEP_MKDIR2=$_FEP_MKDIR1/${us}${sd}
                        mkdir $_FEP_MKDIR2 > /dev/null 2> /dev/null
                    done
                done;;
            3)
                for d in $_FEP_DIR3
                do
                    export _FEP_MKDIR1=$_FEP_MKDIR/${d}
                    mkdir $_FEP_MKDIR1 > /dev/null 2> /dev/null

                    for sd in $_FEP_SUBDIR
                    do
                        export _FEP_MKDIR2=$_FEP_MKDIR1/${us}${sd}
                        mkdir $_FEP_MKDIR2 > /dev/null 2> /dev/null
                    done
                done

                export _FEP_MKDIR1=$_FEP_MKDIR/SEQ
                mkdir $_FEP_MKDIR1 > /dev/null 2> /dev/null
                export _FEP_MKDIR2=$_FEP_MKDIR1/BATCH
                mkdir $_FEP_MKDIR2 > /dev/null 2> /dev/null
                export _FEP_MKDIR2=$_FEP_MKDIR1/DISC
                mkdir $_FEP_MKDIR2 > /dev/null 2> /dev/null
                export _FEP_MKDIR2=$_FEP_MKDIR1/TOTAL
                mkdir $_FEP_MKDIR2 > /dev/null 2> /dev/null;;
        esac
    done
done

##########################################################################
#   End of File (pkg_mkdir.sh)
##########################################################################
