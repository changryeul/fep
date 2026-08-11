#include "all_inc.h"

char    Serr_fname[256];
char    Shome_dir[256];
struct  SHM *shmdp;


#ifdef NO_USE
#define      LOGINIDSEND    "run/data/mastsvr/loginidsend"
#define      MAX_LOGIN       50

struct  LOGIN_ENTRY  {
   int     Iusr_pid;
   int     Iusr_memoff;
   int     Ims_memoff;
   char    Sname_unq[15];
   char    Cuserflag;
};

struct  _LOGIN_CHEF  {
   uchar   Sid[USR_ID_LEN];
   int     login_cnt;
   struct  LOGIN_ENTRY  le[50];
};
#define SZ__LOGIN_CHEF   sizeof(struct   _LOGIN_CHEF)

#endif

usage()
{
   printf("********************************************************\n");
   printf("* >>> 삭제하고자 하는 ID정보를 입력하세요               \n");
   printf("********************************************************\n");
   printf("Usage uid_delete    ID    삭제범위(A/P/D)               \n");
   printf("      uid_delete ansi3000 A(모든   Login정보 삭제)      \n");
   printf("      uid_delete ansi3000 P(서버별 Login정보 삭제)      \n");
   printf("      uid_delete ansi3000 D(Login정보 Display)          \n");
   printf("********************************************************\n");
}

main(argc, argv)
int      argc;
char   *argv[];
{
    int cnt;

    _SetUpUnixEnv(WORKHOMED,Shome_dir);

    if(argc < 3) {
       usage();
       return 0;
    }

    if     (argv[2][0] == 'A') {
       printf("********************************************************\n");
       printf(">>> 해당ID 의 모든 Login정보 삭제                       \n");
       uid_delete(argv[1]);
       printf("********************************************************\n");
    }
    else if(argv[2][0] == 'P') {
       printf("********************************************************\n");
       printf(">>> 해당ID 의 서버별 Login정보 삭제                     \n");
       uid_rewrite(argv[1]);
       printf("********************************************************\n");
    }
    else if(argv[2][0] == 'D') {
       printf("********************************************************\n");
       printf(">>> 해당ID 의 Login정보 Disp                            \n");
       uid_disp(argv[1]);
       printf("********************************************************\n");
    }
    else {
       usage();
       return 0;
    }
}

int uid_delete(char *xid)
{
   int     F_fd,i,cnt;
   char    filename[128];
   struct  _LOGIN_CHEF  chesnd;

   sprintf(filename,"run/data/mastsvr/loginidsend");
   if( 0> (F_fd = isopen(filename, ISINOUT + ISMANULOCK)) ) {
      printf("search [%s] open error \n", filename );
      return(-1);
   }

   memset(chesnd.Sid,0x20,12);
   memcpy(chesnd.Sid,xid,strlen(xid));
   if( 0 > isread(F_fd, (char *)&chesnd , ISEQUAL)) {
      printf("search [%s] no fetch \n", xid);
      isclose(F_fd);
      return(-1);
   }
   cnt = chesnd.login_cnt;
   printf(" [%.10s] [%d] \n", chesnd.Sid, cnt );
   printf("--------------------------------------------------------\n");
   for(i=0; i<cnt; i++) {
      printf(" [%d] [%d] [%d] [%.15s] [%c]\n",
               chesnd.le[i].Iusr_pid, chesnd.le[i].Iusr_memoff, chesnd.le[i].Ims_memoff,
               chesnd.le[i].Sname_unq, chesnd.le[i].Cuserflag);
   }
   printf("--------------------------------------------------------\n");
   printf(" Enter를 3번 치면 삭제가 수행됩니다.                    \n");
   getchar();
   getchar();
   getchar();

   if(isdelete(F_fd, (char *)&chesnd) < 0) 
      printf("All Delete error [%d] \n",iserrno );
   else
      printf("All Delete OK \n");

   isclose(F_fd);
   return(cnt);
}

int uid_rewrite(char *xid)
{
   int     F_fd,i,x,cnt,pos=0;
   char    filename[128];
   char    Sinput[2];
   struct  _LOGIN_CHEF  chesnd;
   struct  _LOGIN_CHEF  tmp;

   sprintf(filename,"run/data/mastsvr/loginidsend");
   if( 0> (F_fd = isopen(filename, ISINOUT + ISMANULOCK)) ) {
      printf("search [%s] open error \n", filename ); 
      return(-1);
   }
    
   memset(chesnd.Sid,0x20,12);
   memcpy(chesnd.Sid,xid,strlen(xid));
   if( 0 > isread(F_fd, (char *)&chesnd , ISEQUAL)) {
      printf("search [%s] no fetch \n", xid);
      isclose(F_fd);
      return(-1);
   }
   isclose(F_fd);

   cnt = chesnd.login_cnt;
   printf(" [%.10s] [%d] \n", chesnd.Sid, cnt );
   printf("--------------------------------------------------------\n");
   for(i=0; i < cnt ;i++) {
      printf(" [%d] [%d] [%d] [%.15s] [%c]\n", 
               chesnd.le[i].Iusr_pid, chesnd.le[i].Iusr_memoff, chesnd.le[i].Ims_memoff,
               chesnd.le[i].Sname_unq, chesnd.le[i].Cuserflag);
   }
   printf("--------------------------------------------------------\n");
   printf(" Enter를 3번 치면 다음 작업이  수행됩니다.              \n");
   getchar();
   getchar();
   getchar();

   while(1) {
      if( 0> (F_fd = isopen(filename, ISINOUT + ISMANULOCK)) ) {
         printf("while search [%s] open error \n", filename );
         return(-1);
      }

      memset(chesnd.Sid,0x20,12);
      memcpy(chesnd.Sid,xid,strlen(xid));
      if( 0 > isread(F_fd, (char *)&chesnd , ISEQUAL)) {
         printf(" 더이상 Login정보가 없습니다. [%s] no fetch \n", xid);
         isclose(F_fd);
         break;
         return(-1);
      }
      cnt = chesnd.login_cnt;
      printf("\n ※ 현재 Login정보 [Sid:%.10s][Login Cnt:%d] \n", chesnd.Sid, cnt);
      for(i=0; i<cnt;i++) {
         printf(" [%d] [%d] [%d] [%.15s] [%c]\n",
               chesnd.le[i].Iusr_pid, chesnd.le[i].Iusr_memoff,
               chesnd.le[i].Ims_memoff, chesnd.le[i].Sname_unq, chesnd.le[i].Cuserflag);

         SET_NULL(Sinput);
         printf("    정말 삭제하시겠습니까? (Y/y) ==>");
         gets(Sinput);

         if((Sinput[0] == 'Y')||(Sinput[0] == 'y')) {
            SET_NULL(tmp);
            pos=0;
            memcpy(tmp.Sid,chesnd.Sid,USR_ID_LEN);
            for(x=0; x<cnt; x++) {   /* entry rearrange    */
               if(0 == memcmp(chesnd.le[x].Sname_unq, chesnd.le[i].Sname_unq, sizeof(chesnd.le[x].Sname_unq)) ) {
               }
               else {
                  tmp.le[pos] = chesnd.le[x];
                  pos++;
               }
            }
            tmp.login_cnt = pos;
            if(tmp.login_cnt == 0) {    /* empty entry */
               if(isdelete(F_fd,(char *)&chesnd) < 0)
                  printf("    delete error [%d] \n",iserrno );
               else
                  printf("    delete OK \n");
            }
            else  {
               if(isrewrite(F_fd, (char *)&tmp) < 0)
                  printf("    delete error [%d][%.15s]\n",iserrno, chesnd.le[i].Sname_unq);
               else
                  printf("    delete OK [%.15s]\n", chesnd.le[i].Sname_unq);
            }
         }
         else {
            printf("    해당 계좌는 Skip!! Bye!! \n");
            continue;
         }
         break;
      }
      isclose(F_fd);

      SET_NULL(Sinput);
      printf("\n ▶ 계속 진행하시겠습니까? (Y/y) ==>");
      gets(Sinput);

      if((Sinput[0] != 'Y')&&(Sinput[0] != 'y')) {
         break;
      }
   }
   return(cnt);  
}

int uid_disp(char *xid)
{
   int     F_fd,i,cnt;
   char    filename[128];
   struct  _LOGIN_CHEF  chesnd;

   sprintf(filename,"run/data/mastsvr/loginidsend");
   if( 0> (F_fd = isopen(filename, ISINOUT + ISMANULOCK)) ) {
      printf("search [%s] open error \n", filename );
      return(-1);
   }

   memset(chesnd.Sid,0x20,12);
   memcpy(chesnd.Sid,xid,strlen(xid));
   if( 0 > isread(F_fd, (char *)&chesnd , ISEQUAL)) {
      printf("search [%s] no fetch \n", xid);
      isclose(F_fd);
      return(-1);
   }
   cnt = chesnd.login_cnt;
   printf(" [%.10s] [%d] \n", chesnd.Sid, cnt );
   printf("--------------------------------------------------------\n");
   for(i=0; i<cnt; i++) {
      printf(" [%d] [%d] [%d] [%.15s] [%c]\n",
               chesnd.le[i].Iusr_pid, chesnd.le[i].Iusr_memoff, chesnd.le[i].Ims_memoff,
               chesnd.le[i].Sname_unq, chesnd.le[i].Cuserflag);
   }

   isclose(F_fd);
   return(cnt);
}
