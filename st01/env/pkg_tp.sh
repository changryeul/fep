##########################################################################
#	Project	: TLFEP system
#	Author	: Trust Line Information & Communications Co., Ltd.
#	Module	: tuxedo environment
#	File	: fep_tp.sh
##########################################################################
export	TUXDIR=/hws01/tuxedo

if [ $HOME = "/rfepj/fepj1" ]; then
	export	TUXHOME=$TUXDIR/hwsc
	export	PATH=$PATH:$TUXDIR/bin:/hws01/hwsc/shl/A
else
	export	TUXHOME=$TUXDIR/test
	export	PATH=$PATH:$TUXDIR/bin:/hws11/test/shl/A
fi

case $host_name in
	at04)
        export  WSNADDR=//201.21.204.130:8800;;
	ap53)
		export	WSNADDR=//201.21.204.153:9200;;
	ap63)
		export	WSNADDR=//201.21.204.163:9200;;
esac

export	APPDIR=$TUXHOME/bin
export	TUXCONFIG=$TUXHOME/cfg/tuxconfig
export	ULOGPFX=$TUXHOME/logs/ULOG

export	SHLIB_PATH=$TUXDIR/lib:$SHLIB_PATH
export	LIBPATH=$TUXDIR/lib:$LIBPATH
export	LD_LIBRARY_PATH=$TUXDIR/lib:$LD_LIBRARY_PATH
export	LC_MESSAGES=en_US

alias	cdd='cd $TUXDIR;pwd'
alias	cda='cd $TUXHOME/bin;pwd'
alias	cdf='cd $TUXHOME/cfg;pwd'
alias	cdl='cd $TUXHOME/logs;pwd'

alias	psr='echo psr|tmadmin -r'	# printserver
alias	psc='echo psc|tmadmin -r'	# printservice
alias	pclt='echo pclt|tmadmin -r'	# printclient

alias	tphome='cd $TUXDIR'
alias	tpbin='cd $TUXHOME/bin'
alias	tpcfg='cd $TUXHOME/cfg'
alias	tplog='cd $TUXHOME/logs'
alias	tpinc='cd $TUXDIR/include'
alias	tpsrc='cd $TUXDIR/samples/atmi'

##########################################################################
#	End of File (fep_tp.sh)
##########################################################################
