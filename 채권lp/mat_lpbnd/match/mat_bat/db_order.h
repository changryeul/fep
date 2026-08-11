#include <stdio.h>

typedef struct _db_order_
{
	char	ORDN_NO							[30  +1];			/* VARCHAR2	    주문번호                         */
	char	ORTR_ORDN_NO					[30  +1];			/* VARCHAR2	    원거래주문번호                   */
	char	OUST_ORDN_NO					[30  +1];			/* VARCHAR2	    당초주문번호                     */
	char	GRP_ORDN_NO						[30  +1];			/* VARCHAR2	    그룹주문번호                     */
	char	ORDN_YMD						[8   +1];			/* VARCHAR2	    주문년월일                       */
	char	ORDN_DCD						[1   +1];			/* CHAR		    주문구분코드                     */
	char	PRAS_ID							[60  +1];			/* VARCHAR2	    호가ID                           */
	char	FDM_ID							[10  +1];			/* VARCHAR2	    외환중개사ID                     */
	char	ORDN_ORGN_DCD					[1   +1];			/* CHAR		    주문원천구분코드                 */
	char	ORDN_ORGN_SUB_DCD				[1   +1];			/* CHAR		                                     */
	char	FX_TRN_TCD						[1   +1];			/* CHAR		    FX거래유형코드                   */
	char	AUON_DCD						[1   +1];			/* CHAR		    자동거래구분코드                 */
	char	ORDE_ID							[20  +1];			/* VARCHAR2	                                     */
	char	WBCS_RLNM_ALTR_NO				[16  +1];			/* VARCHAR2	    전행고객실명대체번호             */
	char	PU_CD							[5   +1];			/* CHAR		    PU코드                           */
	char	FXBK_ID							[20  +1];			/* VARCHAR2	    FX북ID                           */
	char	MNGM_BRCD						[6   +1];			/* CHAR		    관리부점코드                     */
	char	HDOM_APLY_GRP_ID				[3   +1];			/* VARCHAR2	    본점마진적용그룹ID               */
	char	FX_PDCD							[3   +1];			/* CHAR		    FX상품코드                       */
	char	CRNC_PAIR_ID					[7   +1];			/* VARCHAR2	    통화페어ID                       */
	char	TNR_PTRN_DCD					[1   +1];			/* CHAR		    TENOR유형구분코드                */
	char	TNR_ID							[3   +1];			/* CHAR		    TENORID                          */
	char	EXPI_STTG_YMD					[8   +1];			/* VARCHAR2	    만기종료년월일                   */
	char	EXPI_FNSH_YMD					[8   +1];			/* VARCHAR2	    만기시작년월일                   */
	char	FX_ITMS_ID						[30  +1];			/* VARCHAR2	    FX종목ID                         */
	char	BYSEL_DCD						[1   +1];			/* CHAR		    매입매도구분코드                 */
	char	CNTT_CNDT_DCD					[3   +1];			/* CHAR		    체결조건구분코드                 */
	char	ORDN_VALD_YMD					[8   +1];			/* VARCHAR2	    주문유효년월일                   */
	char	TXCR_DCD						[1   +1];			/* CHAR		    거래통화구분코드                 */
	char	BASE_CRCD						[3   +1];			/* CHAR		    기준통화코드                     */
	double	FX_SDCU_AMT						[22  +1];			/* NUMBER		FX기준통화금액                   */
	char	COCU_CD							[3   +1];			/* CHAR		    상대통화코드                     */
	double	FX_COCU_AMT						[22  +1];			/* NUMBER		FX상대통화금액                   */
	double	FX_ORDN_QTY						[22  +1];			/* NUMBER		FX주문수량                       */
	double	FX_CRCT_QTY						[22  +1];			/* NUMBER		FX정정수량                       */
	double	FX_CNCL_QTY						[22  +1];			/* NUMBER		FX취소수량                       */
	double	FX_CNTT_QTY						[22  +1];			/* NUMBER		FX체결수량                       */
	double	FX_ORDN_BALN_QTY				[22  +1];			/* NUMBER		FX주문잔고수량                   */
	double	FX_ORDN_PNTM_PRC				[22  +1];			/* NUMBER		FX주문시점가격                   */
	char	ORDN_PRC_CNCD					[1   +1];			/* CHAR		    주문가격조건코드                 */
	double	FX_ORDN_PRC						[22  +1];			/* NUMBER		FX주문가격                       */
	double	MRKT_SPOT_PRC					[22  +1];			/* NUMBER		시장SPOT가격                     */
	double	MRKT_SWAP_PRC					[22  +1];			/* NUMBER		시장스왑가격                     */
	double	FX_MRKT_PRC						[22  +1];			/* NUMBER		FX시장가격                       */
	double	CVR_SPR							[22  +1];			/* NUMBER		커버스프레드                     */
	double	SLS_SPR							[22  +1];			/* NUMBER		세일즈스프레드                   */
	double	FX_ORCY_PRC						[22  +1];			/* NUMBER		FX당사가격                       */
	char	SPR_UT_DCD						[1   +1];			/* CHAR		    스프레드단위구분코드             */
	double	CUS_SPR							[22  +1];			/* NUMBER		고객스프레드                     */
	double	SLPPG_PRC						[22  +1];			/* NUMBER		슬리피지가격                     */
	char	MNRC_CRCD						[3   +1];			/* CHAR		    입금통화코드                     */
	char	MNRC_CRNC_ACN					[20  +1];			/* VARCHAR2	    입금통화계좌번호                 */
	char	DROT_CRCD						[3   +1];			/* CHAR		    출금통화코드                     */
	char	DROT_CRNC_ACN					[20  +1];			/* VARCHAR2	    출금통화계좌번호                 */
	char	DASTL_YN						[1   +1];			/* CHAR		    차액결제여부                     */
	char	STLM_CRCD						[3   +1];			/* CHAR		    결제통화코드                     */
	char	FX_GRNY_PTRN_DCD				[1   +1];			/* CHAR		    FX보증유형구분코드               */
	double	GRMN_APLY_RT					[22  +1];			/* NUMBER		보증금적용율                     */
	char	NMNL_GRNY_CRCD					[3   +1];			/* CHAR		    명목보증통화코드                 */
	double	NMNL_GRNY_AMT					[22  +1];			/* NUMBER		명목보증금액                     */
	char	GRMN_ACN						[20  +1];			/* VARCHAR2	    보증금계좌번호                   */
	char	GRMN_CRCD						[3   +1];			/* CHAR		    보증금통화코드                   */
	double	GRMN_WTMN_AMT					[22  +1];			/* NUMBER		보증금인출금액                   */
	char	LMT_ACN							[30  +1];			/* VARCHAR2	    한도계좌번호                     */
	char	LMT_CRCD						[3   +1];			/* CHAR		    한도통화코드                     */
	double	FX_LMUS_AMT						[22  +1];			/* NUMBER		FX한도사용금액                   */
	double	NMNL_GRNY_CRNC_TLCH_SELL_RT		[22  +1];			/* NUMBER		명목보증통화전신환매도율         */
	double	NMNL_GRNY_CRNC_TSCN_RT			[22  +1];			/* NUMBER		명목보증통화대미환산율           */
	double	GRLM_CRNC_TSCN_RT				[22  +1];			/* NUMBER		보증한도통화대미환산율           */
	char	FWEX_PSRN_NO					[30  +1];			/* VARCHAR2	    선물환사전점검번호               */
	double	FWEX_PSRN_DTLS_NO				[22  +1];			/* NUMBER		선물환사전점검세부번호           */
	char	FX_CCTN_CHNL_DCD				[2   +1];			/* CHAR		    FX접속채널구분코드               */
	char	ORDN_TRMS_DCD					[3   +1];			/* CHAR		    주문전송구분코드                 */
	char	ORDN_STTS_DCD					[1   +1];			/* CHAR		    주문상태구분코드                 */
	char	ERCD_CONV_ID					[10  +1];			/* VARCHAR2	    오류코드변환ID                   */
	char	ERCD_CONV_CON					[200 +1];			/* VARCHAR2	    오류코드변환내용                 */
	char	ORDN_TRMS_PNTM_CON				[26  +1];			/* VARCHAR2	    주문전송시점내용                 */
	char	ORDN_RCIP_PNTM_CON				[26  +1];			/* VARCHAR2	    주문접수시점내용                 */
	char	LAST_CNTT_PNTM_CON				[26  +1];			/* VARCHAR2	    최종체결시점내용                 */
	char	FDM_ORDN_RCIP_PNTM_CON			[26  +1];			/* VARCHAR2	    외환중개사주문접수시점내용       */
	char	FX_EXTL_REF_NO					[60  +1];			/* VARCHAR2	    FX외부참조번호                   */
	char	ENAM_STFS_YN					[1   +1];			/* CHAR		    전액결제완료여부                 */
	char	EXTL_LNK_ID						[60  +1];			/* VARCHAR2	    외부연계ID                       */
	char	FX_MDIA_DCD						[2   +1];			/* CHAR		    FX매체구분코드                   */
	char	FX_SYS_MAIN_USER_ID				[30  +1];			/* VARCHAR2	    FX시스템메인사용자ID             */
	char	FX_SYS_SUB_USER_ID				[30  +1];			/* VARCHAR2	    FX시스템서브사용자ID             */
	char	ORDE_OFRC_IP					[40  +1];			/* VARCHAR2	    주문자공인IP                     */
	char	ORDE_INDV_IP					[40  +1];			/* VARCHAR2	    주문자개인IP                     */
	char	MAC_ADR							[100 +1];			/* VARCHAR2	    MAC주소                          */
	char	RULE_ID							[36  +1];			/* VARCHAR2	    RULEID                           */
	char	PCSN_BRCD						[4   +1];			/* CHAR		    처리부점코드                     */
	char	FDM_ORDN_NO						[50  +1];			/* VARCHAR2	    외환중개사주문번호               */
	char	ADD_CON							[500 +1];			/* VARCHAR2	                                     */
	char	BOB_EMN							[6   +1];			/* CHAR		                                     */
	char	IMDR_YN							[1   +1];			/* CHAR		                                     */
	char	USE_YN							[1   +1];			/* CHAR		    사용여부                         */
	char	FX_SYS_LSMD_ID					[30  +1];			/* VARCHAR2	    FX시스템최종변경ID               */
	char	FX_SYS_LSMD_TS					[7   +1];			/* DATE		    FX시스템최종변경일시             */
}	DB_ORDER;

