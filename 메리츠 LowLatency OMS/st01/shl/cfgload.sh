# cfgload.sh
# ini.tmp -> ini (after edition of configuration file)

cd $_P_BIN

if [ $# = 1 ] && [ $1 != "?" ]; then
	px_cfgload_mp $1
else
	echo "=========================================================="
	echo "[ini.tmp -> ini (after edition of configuration file)]"
	echo ""
	echo "Usage: cfgload.sh <ini name|\"all\">"
	echo ""
	echo "  e.g. 1) cfgload.sh tcp1"
	echo "       2) cfgload.sh all"
	echo "=========================================================="
fi
