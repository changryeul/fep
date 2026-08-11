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
setpstat.sh pa_7101_dd 1
setfcnt.sh pa_7101_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7101_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7101_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_7201_dd 1
setfcnt.sh pa_7201_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7201_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7201_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_7301_dd 1
setfcnt.sh pa_7301_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7301_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7301_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_7401_dd 1
setfcnt.sh pa_7401_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7401_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7401_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_7801_dd 1
setfcnt.sh pa_7801_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7801_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7801_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_7901_dd 1
setfcnt.sh pa_7901_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7901_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7901_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_7902_dd 1
setfcnt.sh pa_7902_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7902_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7902_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_7905_dd 1
setfcnt.sh pa_7905_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7905_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7905_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
	kill -15 $pno
fi
setpstat.sh pa_7906_dd 1
setfcnt.sh pa_7906_dd R1 0
pno=`ps -ef|grep $LOGNAME|grep pa_7906_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'|wc -l`
if [ $pno = 1 ]; then
	pno=`ps -ef|grep $LOGNAME|grep pa_7906_dd|grep -v psk.sh|grep -v "ps -e"|grep -v vi|grep -v tail|grep -v more|grep -v cat|grep -v grep|awk '{print $2}'`
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
