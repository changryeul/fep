# cfgback.sh
# ini -> ini.tmp (to edit configuration file)

cd $_P_BIN

if [ $# = 1 ] && [ $1 != "?" ]; then
	px_cfgback_mp $1
else
	echo "=========================================================="
	echo "[ini -> ini.tmp (to edit configuration file)]"
	echo ""
	echo "Usage: cfgback.sh <ini name|\"all\">"
	echo ""
	echo "  e.g. 1) cfgback.sh tcp1"
	echo "       2) cfgback.sh all"
	echo "=========================================================="
fi
