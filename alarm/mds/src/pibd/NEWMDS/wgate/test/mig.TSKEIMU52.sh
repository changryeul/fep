if [ $# -ne 1 ]; then
	echo "$0 [save/load]"
	exit
fi

fname=TSKEIMU52.csv
fpath=$PWD/$fname
logfile=$PWD/mig.log

db2 CONNECT TO DSKEIT USER KEI000 USING \'kbEcm2021\!\'

if [ $1 = "save" ]; then

	echo "Save Data from ASIS Table"
	echo ""
######## Save Data from ASIS Table (TOBE 포맷으로 select 하면 LOAD가 간편함)

	db2 "EXPORT TO $fpath OF DEL
			MESSAGES $logfile
				SELECT	 GROUP_CO_CD
				--	 	,FX_PRDCT_DSTCD		-- DELETE
						,'1'				-- NEW : MRKUP_DSTCD
						,FX_PRDCT_CD
				--		,TNR_CD				-- DELETE
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
						,'090000000'		-- NEW : OPEN_PRC_TIME
						,'090000000'		-- NEW : HIGH_PRC_TIME
						,'090000000'		-- NEW : LOW_PRC_TIME
						,PRDAT_CNTST_PRC
						,UPDWN_RATO
						,SYS_LAST_PRCSS_YMS
						,SYS_LAST_UNO
				  FROM	INST1.TSKEIMU52
				 WHERE	REGI_YMD >= '20240101'
		"
	echo ""
	echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
	tail -n5 $logfile
	echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
	echo "data count : `wc $fpath | awk '{print $1}'`"
	echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
	echo ""

elif [ $1 = "load" ]; then

	echo "Load Data to TOBE Table"
	echo ""
######## Load Data to TOBE Table

	db2 "IMPORT FROM TSKEIMU52.csv OF DEL
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

	echo ""
	echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
	tail -n5 $logfile
	echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
	echo "target data count : `wc $fpath | awk '{print $1}'`"
	echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
	echo ""

else

	echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
	echo "wrong input"
	echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"

fi

db2 connect reset

