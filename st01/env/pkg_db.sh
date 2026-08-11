##########################################################################
#	Project	: TLFEP system
#	Author	: Trust Line Information & Communications Co., Ltd.
#	Module	: DB environment
#	File	: pkg_db.sh
##########################################################################
#export	ORACLE_BASE=/oracle
#export	ORACLE_HOME=$ORACLE_BASE/product/8.1.7
#export	ORACLE_SID=TEST
#export	ORACLE_OWNER=oracle
#export	NLS_LANG=American_Korea.KO16KSC5601
#export	PATH=$PATH:$ORACLE_HOME/bin
#export	_DB_PRECOMP_INC=$ORACLE_HOME/precomp/public
#export	_DB_PLSQL_MAKE=$ORACLE_HOME/precomp/lib/env_precomp.mk
#
#if [ $HOME = "/rfepj/fepj1" ]; then
#	export	TNS_ADMIN=$ORACLE_HOME/network/admin
#else
#	export	TNS_ADMIN=$ORACLE_HOME/network_test/admin
#fi

##########################################################################################
# ORACLE
# $ORACLE_HOME/network/admin/tnsnames.ora
##########################################################################################
export  ORACLE_INC=$ORACLE_HOME/precomp/public
export  ORACLE_UID=FIXOMS
export  ORACLE_PWD=sjrnfl1!
#export  ORACLE_TNS=dmaltad
#export  ORACLE_TNS=DMALTAD
export  ORACLE_TNS=MALTDBD
export  SQL_VER=sql12
export  NLS_LANG=American_America.KO16KSC5601

export  ORACLE_BASE=/oracle12/app/oracle
export  ORACLE_HOME=$ORACLE_BASE/product/122
export  ORACLE_SID=MALTDBD
export  ORACLE_OWNER=oracle
#export  PATH=$PATH:$ORACLE_HOME/bin
export  PATH=$PATH:$ORACLE_HOME/bin:$ORACLE_HOME/OPatch:$ORACLE_HOME/Apache/perl/bin:/usr/vac/bin:/usr/local/bin:$PATH:.
export  _DB_PRECOMP_INC=$ORACLE_HOME/precomp/public
export  _DB_PLSQL_MAKE=$ORACLE_HOME/precomp/lib/env_precomp.mk
export  LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ORACLE_HOME/lib:$ORACLE_HOME/network/lib:/lib:/usr/lib

export  TNS_ADMIN=$ORACLE_HOME/network/admin
##########################################################################
#	End of File (pkg_db.sh)
##########################################################################
