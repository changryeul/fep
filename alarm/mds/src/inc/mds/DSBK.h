#ifndef __DSBK_H__
#define __DSBK_H__

struct {
	char	bsns_dt[8];		/* 영업일  */
} DSBK1200B_I ;
#define	DSBK1200B_I_SZ	sizeof(DSBK1200B_I)
typedef	struct {
	char	for_int1	[10];	/* 해외 금리 1개월(%)	*/
	char	for_int3	[10];	/* 해외 금리 3개월(%)	*/
	char	for_int6	[10];	/* 해외 금리 6개월(%)	*/
	char	for_int9	[10];	/* 해외 금리 9개월(%)	*/
	char	for_int12	[10];	/* 해외 금리 12개월(%)	*/
	char	dom_call	[10];	/* call overnight금리(%)*/
	char	dom_cd3mon	[10];	/* 3개월 금리(%)		*/
	char	dom_dv1year	[10];	/* 1년만기 채권금리(%)	*/
	char	dom_msb2year[10];	/* 2년만기 금리(%)		*/
	char	year2		[3];	/* 영업일 + 24개월 - 영업일		*/
	char	gold_store	[10];	/* 금보관료				*/
	char	gold_mask	[15];	/* 31.10379973 금 단위 변환 mask	*/
} DSBK1200B_O ;
#define	DSBK1200B_O_SZ	sizeof(DSBK1200B_O)


typedef	struct {
	char	bsns_dt[8];		/* 영업일  */
}DSBK1200C_I ;
#define	DSBK1200C_I_SZ	sizeof(DSBK1200C_I)
typedef	struct {
	char	rcnt[6];
	struct	{
		char	bond_cd		[12];	/* 채권 코드                    */
		char	issue_p		[18];	/* 발행금액                     */
		char	issue_dt	[ 8];	/* 발행일                       */
		char	end_dt		[ 8];	/* 만기일                       */
		char	int_term	[ 8];	/* 이표지급 간격(6개월)         */
		char	bef_intdt	[ 8];	/* 직전 이표지급일              */
		char	aft_intdt	[ 8];	/* 차기 이표지급일              */
		char	aft_bef_cnt	[ 8];	/* 차기 이표일 - 직전 이표일    */
		char	aft_remncnt	[ 8];	/* 차기 이표지급일까지 잔존일수 */
		char	surf_r		[18];	/* 표면금리 (%)                 */
		char	remn_intcnt	[ 8];	/* 잔여이표지급횟수             */
		char	last_p		[18];	/* B08최근체결가 : 현물수익률   */
		char	spot_p		[18];	/* 국채현물가                   */
		char	bond_tp		[ 1];	/* 채권 종류 : 3년 or 5년       */
	} orec[0];
} DSBK1200C_O ;
#define	DSBK1200C_O_SZ	sizeof(DSBK1200C_O)


typedef	struct {
	char	bsns_dt[8];		/* 영업일  */
} DSBK1200D_I ;
#define	DSBK1200D_I_SZ	sizeof(DSBK1200D_I)
typedef	struct {
	char	rcnt[6];
	struct	{
		char	series			[32];	/* 채권선물 종목명                      */
		char	bond_cd			[12];	/* 채권 코드                            */
		char	expr_dt			[ 8];	/* 선물 만기일                          */
		char	exp_remnintcnt	[ 3];	/* 선물만기일까지잔여이표지급횟수       */
		char	exp_aft_intdt	[ 8];	/* 만기일 시점에서 차기 이표일          */
		char	exp_bef_intdt	[ 8];	/* 만기일 시점에서 직전 이표일          */
		char	exp_aft_bef_cnt	[ 3];	/* 만기일 시점에서 차기 이표일 - 만기일 시점에서 이전 이표일 */
		char	exp_aft_exp_cnt	[ 3];	/* 만기일 시점에서 차기 이표일 - 만기일 */
		char	fexpr_r			[ 7];	/* 선물만기일까지의 기회비용(%)         */
		char	ipo_r			[ 7];	/* 이표금액할인금리(%)                  */
		char	sundo_p			[ 7];	/* 선도가격                             */
		char	sundo_r			[ 7];	/* 선도수익률(%)                        */
		char	thi_p			[ 7];	/* 선물 이론가                          */
		/* TABLE 이동으로 master 필요데이타 증가로 인해 추가 */
		char	bond_nm			[30];	/* 채권명                               */
		char	surf_r			[10];	/* 표면이자률                           */
		char	issue_dt		[ 8];	/* 채권발행일                           */
		char	end_dt			[ 8];	/* 채권만기일                           */
		char	int_term		[ 2];	/* 이표지급방법                         */
		char	aft_intdt		[ 8];	/* 차기이표지급일                       */
		char	remn_intcnt		[ 3];	/* 잔여이표지급횟수                     */
	} orec[1];
} DSBK1200D_O ;
#define	DSBK1200D_O_SZ	sizeof(DSBK1200D_O)

typedef	struct {
	char	bsns_dt[8];		/* 영업일  */
} DSBK1200E_I ;
#define	DSBK1200E_I_SZ	sizeof(DSBK1200E_I)
typedef	struct {
	char	rcnt[6];
	struct	{
		char	undr_cd		[ 2];		/* 상품현물코드     */
		char	pdy_undr_p	[14];		/* 전일현물가       */
	} orec[1];
} DSBK1200E_O ;
#define	DSBK1200E_O_SZ	sizeof(DSBK1200E_O)


#endif
