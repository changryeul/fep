fname=TSKEIMU52.csv
fpath=$PWD/$fname
logfile=$PWD/mig.log

sqlquery="
INSERT INTO INST1.TSKEIMU52
(	  GROUP_CO_CD
	 ,FX_PRDCT_CD
	 ,MRKUP_DSTCD
	 ,REGI_YMD
	 ,RGST_TIME
	 ,BID_LAST_PRC
	 ,OFFER_LAST_PRC
	 ,MID_LAST_PRC
	 ,BID_OPEN_PRC
	 ,BID_HIGH_PRC
	 ,BID_LOW_PRC
	 ,OFFER_OPEN_PRC
	 ,OFFER_HIGH_PRC
	 ,OFFER_LOW_PRC
	 ,MID_OPEN_PRC
	 ,MID_HIGH_PRC
	 ,MID_LOW_PRC
	 ,OPEN_PRC_TIME
	 ,HIGH_PRC_TIME
	 ,LOW_PRC_TIME
	 ,PRDAT_CNTST_PRC
	 ,UPDWN_RATO
	 ,SYS_LAST_PRCSS_YMS
	 ,SYS_LAST_UNO
) values (
"
echo "Load Data to TOBE Table"
echo ""
######## Load Data to TOBE Table

cat $fpath | while read line
do
db2 connect to DSKEIT user KEI000 using \'kbEcm2021\!\'
	sql="$sqlquery $line);"
	echo $sql
	db2 -t "$sql"
db2 quit
done
