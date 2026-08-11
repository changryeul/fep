##########################################################################
#	Module	: make programs
#	File	: mk.sh
##########################################################################

if ( [ $# = 1 ] && [ $1 != "?" ] ) || [ $# = 2 ]; then
	typeset -u sub=$1
	if [ $sub = "ALL" ]; then
		\rm ${_P_LIB}/libfep*.a
		echo "rm ${_P_LIB}/libfep*.a"
		sleep 2
		\rm ${_P_OBJ}/*/*
		echo "rm ${_P_OBJ}/*/*"
		sleep 2
		echo "FEPp P LIB Make"
		make -f ${_PSUB_MAKE}/Make_Lib_P.mk
		sleep 2
		touch ${_P_LIB}/libfepP.a
		sleep 2

		for sd in ${_FEP_SUBDIR}
		do
			typeset -u us=${_FEP_SYSTEM}${sd}
			echo "FEPp ${us} SRC Make"
			make -f ${_P_MAKE}/${us}/Make_${us}.mk &
		done
	else
		if [ $sub = "SUB" ]; then
			echo "FEPp P LIB Make!!!!!!!!!!!"
			make -f ${_PSUB_MAKE}/Make_Lib_P.mk
		elif [ $sub = "SRC" ]; then
			for sd in ${_FEP_SUBDIR}
			do
				typeset -u us=${_FEP_SYSTEM}${sd}
				echo "FEPp ${us} SRC Make"
				make -f ${_P_MAKE}/${us}/Make_${us}.mk &
			done
		elif [ $sub = "TEST" ]; then
			echo "FEPp Unit Test"
			make -C ${_FEP_HOME}/st01/test/unit
		elif [ $sub = "INTEG" ]; then
			echo "FEPp Integration Test"
			make -C ${_FEP_HOME}/st01/test/integ
		else
			if [ $# = 2 ]; then
				echo "FEPp $sub $2 SRC Make!!!!!!!!!!!"
				sh ${_P_MAKE}/$sub/Make_${sub}_$2.sh
			else
				echo "FEPp $sub SRC Make!!!!!!!!!!!"
				make -f ${_P_MAKE}/$sub/Make_${sub}.mk
			fi
		fi
	fi
else
	echo "======================================"
	echo "Usage: mk.sh <option>"
	echo "--------------------------------------"
	echo "  e.g. 1) mk.sh all (��)"
	echo "       2) mk.sh sub (library)"
	echo "       3) mk.sh src (src ��)"
	echo "       4) mk.sh pa (PA src��)"
	echo "       5) mk.sh pa ts (PA src TS)"
	echo "       6) mk.sh test (unit test)"
	echo "       7) mk.sh integ (integration test)"
	echo "======================================"
fi

##########################################################################
#	End of File (mk.sh)
##########################################################################
