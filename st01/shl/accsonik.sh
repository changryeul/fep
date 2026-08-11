#!/bin/sh

export  PATH=$PATH:/usr/local/bin:/usr/sbin:/etc:.:$HOME/shl
umask 003

. $HOME/env/fep_env.sh

clear
date +%Y/%m/%d-%H:%M:%S

wfile=/rfepp/fepp3/SEQ/TOTAL/sonik_`date +%Y%m%d`

cd $_P_BIN

px_accsonik_mp > $wfile

