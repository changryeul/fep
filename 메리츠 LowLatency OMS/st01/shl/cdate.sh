#!/bin/sh
########################################
#@(#)Program    : cdate.sh
#@(#)Purpose    :
#@(#)Release    : 1.06 / Thu Jun 10 08:25:01 METDST 1999
#@(#)UnixPerm   : 755.0.3
#@(#)Author     : JPvdGiessen IT Consultancy (jpvdgiessen@gelrevision.nl)
#@(#)Called by  : prompt / jobmanager
#@(#)Remarks    :
########################################
        CatType=5
      #  . ${JPGBV:-/usr/jpg}/etc/funcmain.lib
        # Usage: cdate.sh  [yyyy mm dd] [{+|-}ndays]
        if [ $# -eq 0 ]
        then
            Y=`date "+%Y"`
            M=`date "+%m"`
            D=`date "+%d"`
            nb=-1
        elif [ $# -eq 1 ]
        then
            Y=`date "+%Y"`;
            M=`date "+%m"`;
            D=`date "+%d"`;
            nb=$1;
        elif [ $# -eq 3 ]
        then
            Y=$1; M=$2; D=$3; nb=-1;
        else
            Y=$1; M=$2; D=$3; nb=$4;
        fi
        nawk -v TYear=$Y -v TMnth=$M -v TDay=$D -v NBefore=$nb '
        function IsLeap(yy)
        {
            if ((yy%400)==0) {return 1;}
            if ((yy%100)==0) {return 0;}
            if ((yy%4)==0)   {return 1;}
            return 0;
        }
        BEGIN {
            Mds="31:28:31:30:31:30:31:31:30:31:30:31";
            split(Mds,MDA,":");
            # Find out the day of yesterday.
            Year = TYear+0;
            Mnth = TMnth+0;
            Day = TDay+0;
            Day += NBefore;
            while( (Day <= 0) || (Day > MDA[Mnth]) ) {
                if(Day <= 0) {
                    Mnth -= 1;
                    if(Mnth == 0) {
                        Mnth = 12;
                        Year--;
                        MDA[2] = IsLeap(Year) ? 29:28;
                    } else if( Mnth == 2 && IsLeap(Year)) {
                        MDA[2]=29;
                    }
                    Day += MDA[Mnth];
                    } else if(Day > MDA[Mnth] ) {
                    if( Mnth == 2 && IsLeap(Year)) {
                        MDA[2]=29;
                    }
                    Day -= MDA[Mnth];
                    Mnth += 1;
                    if(Mnth == 13 ) {
                        Mnth = 1;
                        Year++;
                        MDA[2] = IsLeap(Year) ? 29:28;
                    }
                }
            }
            #print Mnth,Day,Hour
            printf "%04d%02d%02d\n",Year,Mnth,Day;
        } ' -


