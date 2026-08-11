##########################################################################
#	Module	: makefile - PA
#	File	: Make_PA.mk
##########################################################################

all: TR TS UR US DD MP END

TR:
	sh ${_PA_MAKE}/Make_PA_tr.sh

TS:
	sh ${_PA_MAKE}/Make_PA_ts.sh

UR:
	sh ${_PA_MAKE}/Make_PA_ur.sh

US:
	sh ${_PA_MAKE}/Make_PA_us.sh

DD:
	sh ${_PA_MAKE}/Make_PA_dd.sh

MP:
	sh ${_PA_MAKE}/Make_PA_mp.sh

END:
	sleep 1
	echo "FEPp PA SRC Make End"

##########################################################################
#	End of File (Make_PA.mk)
##########################################################################
