##########################################################################
#	Module	: makefile - PA
#	File	: Make_PA.mk
##########################################################################

#all: BR QR QS TR TS UR DD MP END
all: QR QS TR TS UR DD MP END

#BR:
#	sh ${_PA_MAKE}/Make_PA_br.sh

QR:
	sh ${_PA_MAKE}/Make_PA_qr.sh

QS:
	sh ${_PA_MAKE}/Make_PA_qs.sh

TR:
	sh ${_PA_MAKE}/Make_PA_tr.sh

TS:
	sh ${_PA_MAKE}/Make_PA_ts.sh

UR:
	sh ${_PA_MAKE}/Make_PA_ur.sh

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
