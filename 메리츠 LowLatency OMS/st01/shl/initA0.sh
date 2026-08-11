cd $_P_SHL

#현물
setpstat.sh pa_7501_dd 1
setfcnt.sh pa_7501_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7501_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7501_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_7502_dd 1
setfcnt.sh pa_7502_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7502_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7502_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_7601_dd 1
setfcnt.sh pa_7601_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7601_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7601_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi

#파생
setpstat.sh pa_7191_dd 1
setfcnt.sh pa_7191_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7191_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7191_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_7291_dd 1
setfcnt.sh pa_7291_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7291_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7291_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_7391_dd 1
setfcnt.sh pa_7391_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7391_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7391_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_7491_dd 1
setfcnt.sh pa_7491_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7491_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7491_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_7891_dd 1
setfcnt.sh pa_7891_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7891_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7891_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_7991_dd 1
setfcnt.sh pa_7991_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7991_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7991_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_7992_dd 1
setfcnt.sh pa_7992_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7992_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7992_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_6591_dd 1
setfcnt.sh pa_6591_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_6591_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_6591_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_6691_dd 1
setfcnt.sh pa_6691_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_6691_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_6691_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi

#해외
setpstat.sh pa_6101_dd 1
setfcnt.sh pa_6101_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_6101_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_6101_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_6301_dd 1
setfcnt.sh pa_6301_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_6301_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_6301_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_6401_dd 1
setfcnt.sh pa_6401_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_6401_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_6401_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_6501_dd 1
setfcnt.sh pa_6501_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_6501_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_6501_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
