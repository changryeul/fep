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

usage() {
    printf("********************************************************\n");
    printf("* >>> �����ϰ��� �ϴ� ID������ �Է��ϼ���               \n");
    printf("********************************************************\n");
    printf("Usage uid_delete    ID    ��������(A/P/D)               \n");
    printf("      uid_delete ansi3000 A(���   Login���� ����)      \n");
    printf("      uid_delete ansi3000 P(������ Login���� ����)      \n");
    printf("      uid_delete ansi3000 D(Login���� Display)          \n");
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
        printf(">>> �ش�ID �� ��� Login���� ����                       \n");
        uid_delete(argv[1]);
        printf("********************************************************\n");
    }
    else if(argv[2][0] == 'P') {
        printf("********************************************************\n");
        printf(">>> �ش�ID �� ������ Login���� ����                     \n");
        uid_rewrite(argv[1]);
        printf("********************************************************\n");
    }
    else if(argv[2][0] == 'D') {
        printf("********************************************************\n");
        printf(">>> �ش�ID �� Login���� Disp                            \n");
        uid_disp(argv[1]);
        printf("********************************************************\n");
    }
    else {
        usage();
        return 0;
    }
}

int uid_delete(char *xid) {
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
    printf(" Enter�� 3�� ġ�� ������ ����˴ϴ�.                    \n");
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

int uid_rewrite(char *xid) {
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
    printf(" Enter�� 3�� ġ�� ���� �۾���  ����˴ϴ�.              \n");
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
            printf(" ���̻� Login������ �����ϴ�. [%s] no fetch \n", xid);
            isclose(F_fd);
            break;
            return(-1);
        }
        cnt = chesnd.login_cnt;
        printf("\n �� ���� Login���� [Sid:%.10s][Login Cnt:%d] \n", chesnd.Sid, cnt);
        for(i=0; i<cnt;i++) {
            printf(" [%d] [%d] [%d] [%.15s] [%c]\n",
                    chesnd.le[i].Iusr_pid, chesnd.le[i].Iusr_memoff,
                    chesnd.le[i].Ims_memoff, chesnd.le[i].Sname_unq, chesnd.le[i].Cuserflag);

            SET_NULL(Sinput);
            printf("    ���� �����Ͻðڽ��ϱ�? (Y/y) ==>");
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
                printf("    �ش� ���´� Skip!! Bye!! \n");
                continue;
            }
            break;
        }
        isclose(F_fd);

        SET_NULL(Sinput);
        printf("\n �� ��� �����Ͻðڽ��ϱ�? (Y/y) ==>");
        gets(Sinput);

        if((Sinput[0] != 'Y')&&(Sinput[0] != 'y')) {
            break;
        }
    }
    return(cnt);
}

int uid_disp(char *xid) {
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
