##########################################################################
#	Module	: makefile - PB
#	File	: Make_PB.mk
##########################################################################

#all: BR QR QS TR TS UR DD MP END
all: TS TR UR MP END

#BR:
#	sh ${_PB_MAKE}/Make_PB_br.sh

TS:
	sh ${_PB_MAKE}/Make_PB_ts.sh

TR:
	sh ${_PB_MAKE}/Make_PB_tr.sh

UR:
	sh ${_PB_MAKE}/Make_PB_ur.sh

MP:
	sh ${_PB_MAKE}/Make_PB_mp.sh

END:
	sleep 1
	echo "FEPp PB SRC Make End"

##########################################################################
#	End of File (Make_PB.mk)
##########################################################################
