##########################################################################
#	System	: PARK SANG HOON HATS_H System
#	Module	: makefile - PB (선물옵션_주문)
#	File	: Make_PB.mk
##########################################################################

all: TR TS DD MP UR END

TR:
	sh ${_PB_MAKE}/Make_PB_tr.sh

TS:
	sh ${_PB_MAKE}/Make_PB_ts.sh

DD:
	sh ${_PB_MAKE}/Make_PB_dd.sh

MP:
	sh ${_PB_MAKE}/Make_PB_mp.sh

UR:
	sh ${_PB_MAKE}/Make_PB_ur.sh

END:
	sleep 1
	echo "FEPp PB SRC Make End"

##########################################################################
#	End of File (Make_PB.mk)
##########################################################################
