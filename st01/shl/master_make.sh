#!/usr/bin/sh
#Client요청 Master Table 만들기

export  PATH=$PATH:/usr/local/bin:/usr/sbin:/etc:.:$HOME/shl
umask 003

. $HOME/env/fep_env.sh


cd $_P_DAT/PA/00000000/

frcnt=`cat pa_7101_dd| wc -l`
if [ $frcnt -gt 0 ]
then


fi


cat pa_7101_dd|cut -c64-69|while read x;do ((tot = $tot + $x));done;
