#!/usr/bin/sh

export  PATH=$PATH:/usr/local/bin:/usr/sbin:/etc:.:$HOME/shl
umask 003

. $HOME/env/fep_env.sh

cd /rfepp/fepp1/utl/shpark/

`/rfepp/fepp1/utl/shpark/mastermaker OPTION > /rfepp/fepp1/utl/cli/bin/data/omast.dat`
`/rfepp/fepp1/utl/shpark/mastermaker FUTURE > /rfepp/fepp1/utl/cli/bin/data/fmast.dat`

