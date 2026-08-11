##########################################################################
#	Project	: TLFEP system
#	Author	: Trust Line Information & Communications Co., Ltd.
#	Module	: DB environment
#	File	: pkg_db.sh
##########################################################################
export	ORACLE_BASE=/oracle
export	ORACLE_HOME=$ORACLE_BASE/product/8.1.7
export	ORACLE_SID=TEST
export	ORACLE_OWNER=oracle
export	NLS_LANG=American_Korea.KO16KSC5601
export	PATH=$PATH:$ORACLE_HOME/bin
export	_DB_PRECOMP_INC=$ORACLE_HOME/precomp/public
export	_DB_PLSQL_MAKE=$ORACLE_HOME/precomp/lib/env_precomp.mk

if [ $HOME = "/rfepj/fepj1" ]; then
	export	TNS_ADMIN=$ORACLE_HOME/network/admin
else
	export	TNS_ADMIN=$ORACLE_HOME/network_test/admin
fi

##########################################################################
#	End of File (pkg_db.sh)
##########################################################################
