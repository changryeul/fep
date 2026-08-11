/*****************************************************************************
 *  Components  : paho8000.h
 *  Description : FTS 8000 Cilent TRANSACTION  @|9. A$@G
 *  Rev. History: Ver   Date    Description
 *                ----  ------- ----------------------------------------------
 *                1.0   2017-05 Initial version
 ******************************************************************************/
#ifndef svctrn_h
#define svctrn_h

#define	MTSENV_FILE	"MTSENV.cfg"
#define	HOST_SVCIP	"211.175.18.243"

/*----------------------------------------------------------------------------
 * 상품정보관리 (조회)
 *----------------------------------------------------------------------------*/
typedef struct {
	char	rcnt	[6];
	struct	{
		char	comd[10];		/* 상품코드				*/
		char	name[30];		/* 상품명				*/
		char	exch[10];		/* 거래소코드			*/
		char	fuop[1 ];		/* 선물옵션구분			*/ /* F: 선물	O: 옵션	*/
		char	tgub[1 ];		/* 거래여부				*/
		char	pind[1 ];		/* 가격표시				*/
		char	tsiz[12];		/* 호가단위				*/
		char	tval[12];		/* 가격변동폭			*/
		char	aval[12];		/* 가격조정계수			*/
		char	csiz[12];		/* 계약당금액			*/
		char	cmon[3 ];		/* 상장개월수			*/
		char	msec[1 ];		/* MARKET구분			*/
		char	csec[3 ];		/* Commodity구분		*/
		char	isec[3 ];		/* Instrument구분		*/
		char	esec[1 ];		/* E-mini구분			*/
		char	high[6 ];		/* 자체 상한가			*/
		char	lowx[6 ];		/* 자체 하한가			*/
		char	maxq[6 ];		/* 주문한도수량			*/
		char	infosys_commd_sect [3 ];		/* 정보계용상품구분			*/
	} orec[1];
} GSBK100002_O ;
#define GSBK100002_O_SZ	(sizeof(GSBK100002_O))

/*----------------------------------------------------------------------------
 * CabSize (조회)
 *----------------------------------------------------------------------------*/
typedef struct {
	char	commd_cd[10];		/* 상품코드		*/
} GSBK100005_I ;
#define GSBK100005_I_SZ	(sizeof(GSBK100005_I))
typedef struct {
	struct	{
	char	commd_cd	[10];		/* 상품코드           */
	char	start_price	[20];		/* 구간 시작 가격     */
	char	end_price	[20];		/* 구간 종료 가격     */
	char	tick_size	[20];		/* 적용 TICK_SIZE     */
	char	ipad 		[15];		/* 작업자IP           */
	char	user		[30];		/* 작업자             */
	char	time		[23];		/* 작업일시           */
	char	conv_start_price[20];		/* 10진법_구간 시작 가격     */
	char	conv_end_price	[20];		/* 10진법_구간 종료 가격     */
	char	conv_tick_size	[20];		/* 10진법_적용 TICK_SIZE     */
	} orec[1];
} GSBK100005_O ;
#define GSBK100005_O_SZ (sizeof(GSBK100005_O))

/*----------------------------------------------------------------------------
 * Symbol Information Output Format
 *----------------------------------------------------------------------------*/
typedef struct	{
	char	symb				[32];		/* code 			*/
} GSBK100003_I;
typedef struct	{
	char	f_exch_cd           [10];		/* FUT_거래소      */
	char	f_product_group_nm  [20];		/* FUT_상품구분    */
	char	f_prev_end_price    [15];		/* FUT_전일종가    */
	char	f_start_date        [8 ];		/* FUT_상장일      */
	char	f_exp_day_cnt       [5 ];		/* FUT_잔존일수    */
	char	f_trust_margin      [15];		/* FUT_위탁증거금  */
	char	f_maint_margin      [15];		/* FUT_유지증거금  */
	char	f_open_price        [15];		/* FUT_시가        */
	char	f_high_price        [15];		/* FUT_고가        */
	char	f_low_price         [15];		/* FUT_저가        */
//	char	f_last_trade        [8 ];		/* FUT_최종거래일  */
//	char	f_fnd_date          [8 ];		/* FUT_최초통보일  */
	char	f_exp_date          [8 ];		/* FUT_최종거래일  */
	char	f_fnd_date          [8 ];		/* FUT_FND		   */
	char	f_last_date         [8 ];		/* FUT_신규제한일  */
	char	f_clear_type_nm     [20];		/* FUT_만기결제    */
	char	f_trad_state_nm     [20];		/* FUT_거래여부    */
	char	f_contract_size     [15];		/* FUT_계약크기    */
	char	f_curr_cd           [3 ];		/* FUT_거래통화    */
	char	f_tick_size         [15];		/* FUT_틱Size      */
	char	f_tick_value        [15];		/* FUT_틱가치      */

    struct {
        char    open            [12];       /* FUT_장개시      */
        char    clos            [12];       /* FUT_장마감      */
    } f_mkt_tm[3];

	char	o_instr_cd          [8 ];		/* OPT_기초자산    */
	char	o_opt_type_nm       [20];		/* OPT_옵션type    */
	char	o_clear_type_nm     [20];		/* OPT_만기결제    */
	char	o_curr_cd           [3 ];		/* OPT_거래통화    */
	char	o_contract_size     [15];		/* OPT_승수        */
	char	o_tick_size         [15];		/* OPT_틱Size      */
	char	o_tick_value        [15];		/* OPT_틱가치      */
	char	o_trust_margin      [15];		/* OPT_위탁증거금  */
	char	o_last_trade        [8 ];		/* OPT_최종거래일  */
	char	o_h_volat           [15];		/* OPT_역사성변동성*/
	char	o_volat             [15];		/* OPT_내재변동성  */
	char	o_value             [15];		/* OPT_내재가치    */
	char	o_t_value           [15];		/* OPT_시간가치    */
	char	o_delta             [15];		/* OPT_델타        */
	char	o_gamma             [15];		/* OPT_감마        */
	char	o_theta             [15];		/* OPT_세타        */
	char	o_vega              [15];		/* OPT_베가        */
	char	o_rho               [15];		/* OPT_로우        */
	char	o_eth_ctime         [12];		/* OPT_장마감      */
	char	o_exp_day_cnt       [5 ];		/* OPT_잔존일수    */
    struct {
        char    open            [12];       /* OPT_장개시      */
        char    clos            [12];       /* OPT_장마감      */
    } o_mkt_tm[3];

} GSBK100003_O;

/*----------------------------------------------------------------------------
 * 상품별 증거금 관리 (조회)
 *----------------------------------------------------------------------------*/
typedef struct {
	char	code[10];		/* 상품코드		*/
	char	date[8 ];		/* 조회일자		*/
	char	chck[1 ];		/* 전체일자조회		*/
	char	hegt[1 ];		/* 증거금유형		*/
} GSBK100004_I ;
#define GSBK100004_I_SZ	(sizeof(GSBK100004_I))
typedef struct {
	struct	{
		char	code[10];		/* 상품코드		*/
		char	cdnm[30];		/* 상품명		*/
		char	hegt[1 ];		/* 증거금유형		*/
		char	hdes[8 ];		/* 증거금유형명		*/
		char	gubn[1 ];		/* 증거금구분 	(1:정액 2:정율)	*/
		char	mrgn[12];		/* 적용증거금		*/
		char	mrgn_updn[12];	/* 증거금증감		*/
		char	frdt[8 ];		/* 적용시작일		*/
		char	todt[8 ];		/* 적용종료일		*/
		char	uemp[30];		/* 작업자		*/
		char	udat[14];		/* 작업시간		*/
		char	wkip[15];		/* 작업자IP		*/
		char	buy_opt_exe_reg_margin_rate[12];	/*매수옵션행사예약증거금비율*/
	} orec[1];
} GSBK100004_O ;
#define GSBK100004_O_SZ (sizeof(GSBK100004_O))
#endif
