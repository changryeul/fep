# ftp.sh

FTP_Processing()
{
cd $SOUR_DIRECTORY
ftp -n $MACHINE <<EndFTP
user $USERNAME $PASSWORD
$SENDMODE
prompt
hash
cd $DEST_DIRECTORY
mput $DEST_FILE
EndFTP
}

function Send_Common
{
	SOUR_DIRECTORY=${1}
	DEST_DIRECTORY=${2}
	FTP_Processing
}

function Send_OutOfRange
{
	SOUR_DIRECTORY=$SELECTION
	DEST_DIRECTORY=${REMOTE_DIR}
	FTP_Processing
}

#	Send_Main
function Send_Main
{
	case $SELECTION in
		pbin) 	Send_Common $_P_BIN 		"bin";;
		tpbin) 	Send_Common $APPDIR 		$TPDIR;;
		penv) 	Send_Common $_P_ENV 		"env";;
		pshl) 	Send_Common $_P_SHL 		"shl";;
		pcfg) 	Send_Common $_P_CFG 		"cfg";;
		pinc) 	Send_Common $_P_INC 		"inc";;
		psub) 	Send_Common $_P_SUB 		"sub";;
		plib) 	Send_Common $_P_LIB 		"lib";;

		pmake) 	Send_Common $_P_MAKE 		"make";;
		pamake)	Send_Common $_PA_MAKE 		"make/PA";;
		pwmake)	Send_Common $_PW_MAKE 		"make/PW";;
		pxmake)	Send_Common $_PX_MAKE 		"make/PX";;
		pymake)	Send_Common $_PY_MAKE 		"make/PY";;
		pzmake)	Send_Common $_PZ_MAKE 		"make/PZ";;
		pmakes)	Send_Common $_PSUB_MAKE		"make/SUB";;

		pasrc) 	Send_Common $_PA_SRC 		"src/PA";;
		pwsrc) 	Send_Common $_PW_SRC 		"src/PW";;
		pxsrc) 	Send_Common $_PX_SRC 		"src/PX";;
		pysrc) 	Send_Common $_PY_SRC 		"src/PY";;
		pzsrc) 	Send_Common $_PZ_SRC 		"src/PZ";;

		*) echo "***** Out of Part (${SELECTION}) *****" 
			Send_OutOfRange;;
	esac
}

#	Main Procedure
function Usage
{
	echo ''
	echo 'Check your arguments !!!'
	echo ''
	echo '*******************************************************************************'
	echo 'Usage'
	echo '*******************************************************************************'
	echo '  1) ftp.sh <Mode> <HostName> <DirAlias> <File> <UserName> <Password>'
	echo '    e.g. ftp.sh asc ap54 pshl "*" rfepp pw'
	echo ''
	echo '  2) ftp.sh <Mode> <HostName> <SrcDir> <SrcFile> <TargetDir> <UserName> <Passwd>'
	echo '    e.g. ftp.sh bin ap54 /etc "*" /tmp rfepp pw'
	echo '         ftp.sh asc ap54 /etc checklist /tmp rfepp pw'
	echo ''
	echo '  * DirAlias: pbin, pshl, pcfg, psub, ...'
	echo '*******************************************************************************'
	exit
}

if [ $# -lt 6 ]; then
	Usage
fi

SENDMODE=$1
MACHINE=$2
SELECTION=$3
DEST_FILE=$4

if [ $# -eq 6 ]; then
	USERNAME=$5
	PASSWORD=$6
else 
	if [ $# -eq 7 ]; then
		REMOTE_DIR=$5
		USERNAME=$6
		PASSWORD=$7
	else
		exit
	fi
fi

Send_Main
