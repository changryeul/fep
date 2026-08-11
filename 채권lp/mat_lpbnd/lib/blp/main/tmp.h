typedef struct
{
    int             total_item_cnt;         /* 종목수, seq 0에만 사용       */

    int             check_cnt;              /* 보유여부 확인(평가금액용)    */
                                            /* 해당종목을 보유한 관리계좌가 있는지, 평가금액 확인 때문에 */
    int             auto_use;               /* 자동전략기동시 해당종목 +- 설정    */

    int             CURR_Arry_Key;          /* 최근 시세내역 30개의 Key     */
    int             Befor_CURR_Arry_Key;    /* CURR_Arry_Key 직전값기억     */

    int             HogaLastGbn;            /* 0:체결(A3/G7), 1:호가(B6)    */

    CO_M401K        M4;                 /* 장운영정보, seq는 0만사용    */
    CO_A701K        A7;                 /* 장운영TS, 사용여부는 불투명  */

    CO_A001_RDS02   A0;                 /* 채권 종목배치(RDS)           */
    CO_A301K        A3;                 /* 채권 체결_A3                 */
    CO_G701K        G7;                 /* 채권 체결_G7                 */
    CO_B601K        B6;                 /* 채권 호가_B6                 */

    CO_G701K        CURR_Arry[30];      /* 최근 시세내역 30개 기억   */

}   SHM_NOTE;       /* 채권시세 Shared Memory */

