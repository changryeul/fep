/*------------------------------------------------------------------------
#	Module	: INISAFE-Net encryption functions for PB processes
#	File	: fep_encrypt.h
------------------------------------------------------------------------*/
#ifndef _FEP_ENCRYPT_H_
#define _FEP_ENCRYPT_H_

#ifdef NO_INISAFE
/*------------------------------------------------------------------------
	INISAFE disabled — stub declarations only
------------------------------------------------------------------------*/
typedef void *net_ctx;          /* dummy type for compilation */

extern void     Free_All (void *);
extern int      Handshake (void);

#else
/*------------------------------------------------------------------------
	INISAFE enabled — real declarations
------------------------------------------------------------------------*/
#include "INISAFENet.h"

extern net_ctx         *EnCtx;
extern char             KRX_INITECH_CONF_PATH[];
extern unsigned char   *cinitout;
extern unsigned char   *cupdateout;
extern unsigned char   *cfinalout;
extern unsigned char   *sinitout;
extern unsigned char   *supdateout;
extern int              cinitoutl;
extern int              cupdateoutl;
extern int              cfinaloutl;

extern void     Free_All (void *);
extern int      Handshake (void);

#endif /* NO_INISAFE */

#endif /* _FEP_ENCRYPT_H_ */
