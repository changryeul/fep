fname=TSKEIMU52.csv
fpath=$PWD/$fname
logfile=$PWD/mig.log

echo "Load Data to TOBE Table"
echo ""
######## Load Data to TOBE Table

db2 CONNECT TO DSKEIT USER KEI000 USING \'kbEcm2021\!\'

db2 "LOAD FROM $fpath OF DEL
		MESSAGES $logfile
		INSERT INTO KEI000.TSKEIMU52
		(	  GROUP_CO_CD
			 ,MRKUP_DSTCD
			 ,FX_PRDCT_CD
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
		)
"
db2 "COMMIT"

db2 connect reset

