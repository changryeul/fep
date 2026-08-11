/*------------------------------------------------------------------------
#	Module	: Common event-loop functions for PA/PB processes
#	File	: fep_common.h
------------------------------------------------------------------------*/
#ifndef _FEP_COMMON_H_
#define _FEP_COMMON_H_

/*------------------------------------------------------------------------
	Extern declarations for process-level globals
	(defined in each process .c, resolved by linker)
------------------------------------------------------------------------*/
extern int		Sockfd;
extern int		SendLen;
extern int		RecvLen;
extern int		FirstSeq;
extern double	SendMsec;
extern char		LogOnFlag;
extern char		OpenFlag;
extern char		DataBuff[];
extern char		IpAddr[];
extern char		NoTime[];
extern void	   *FmtPtr;
extern int		ConnectRetryCnt;
extern struct pollfd	Poll[];

/*------------------------------------------------------------------------
	Common function prototypes
	(implemented in sub/fep_common.c)
------------------------------------------------------------------------*/
extern void		Get_Msec (double *);
extern void		Line_Change (void);
extern int		Device_Read (void);
extern void		Device_Write (void);
extern void		Device_Close_Base (void);
extern void		Err_Msg (void);
extern int		Log_Out_Base (void);
extern int		Device_Open_Logon (int, int);
extern void		Time_Out_Disconnect (const char *);
extern void		Fifo_Event_Rtn (void);
extern void		Set_Socket_Linger (int);

/*------------------------------------------------------------------------
	Shared utility function prototypes
------------------------------------------------------------------------*/
extern void	format_ip_addr(const char *packed_ip, char *dotted_ip, int buf_size);
extern int	init_udp_socket(struct sockaddr_in *svr_addr, struct sockaddr_in *clnt_addr,
				const char *svr_ip, int svr_port,
				int sndbuf_size, int rcvbuf_size, int broadcast);
extern int	detect_poll_event(struct pollfd *poll_arr, int poll_cnt,
				int socket_event_idx);

/*------------------------------------------------------------------------
	Process-specific function prototypes
	(implemented in each process file, called by common functions)
------------------------------------------------------------------------*/
extern void		Log_Out (void);
extern void		Device_Close (void);
extern int		Handshake (void);

#endif /* _FEP_COMMON_H_ */
