/*************************************************************************
    File        : . lat_trace.h
    Comment     : . order latency trace (F1 order-latency-metrics)
                  . call sites compile to nothing unless -DLAT_TRACE
                  . output line: <epoch_usec>|<proc>|<point>|<key>
*************************************************************************/
#ifndef _LAT_TRACE_H
#define _LAT_TRACE_H

extern  void    Lat_Init(const char *p_proc);
extern  void    Lat_Point(const char *p_point, const char *p_key, int p_klen);
extern  void    Lat_Close(void);

#ifdef LAT_TRACE
#define LAT_INIT(p)             Lat_Init(p)
#define LAT_POINT(pt, k, kl)    Lat_Point(pt, k, kl)
#define LAT_CLOSE()             Lat_Close()
#else
#define LAT_INIT(p)
#define LAT_POINT(pt, k, kl)
#define LAT_CLOSE()
#endif

#endif  /* _LAT_TRACE_H */
