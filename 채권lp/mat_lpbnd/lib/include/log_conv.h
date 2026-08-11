
#ifndef LOG_CONV_H
#define	LOG_CONV_H	1
#if 0
#define LogErr( format, args...)        Log( USR_OK, "%s:%d " format, __FUNCTION__, __LINE__, ##args)
#define LogLib( format, args...)        Log( USR_OK, "%s:%d " format, __FUNCTION__, __LINE__, ##args)
#define LogCri( format, args...)        Log( USR_OK, "%s:%d " format, __FUNCTION__, __LINE__, ##args)
#define LogMsg( format, args...)        Log( USR_OK, "%s:%d " format, __FUNCTION__, __LINE__, ##args)
#define LogDbg( format, args...)        Log( USR_OK, "%s:%d " format, __FUNCTION__, __LINE__, ##args)
#define LogWar( format, args...)        Log( USR_OK, "%s:%d " format, __FUNCTION__, __LINE__, ##args)
#define LogRaw( format, args...)        Log( USR_OK, "%s:%d " format, __FUNCTION__, __LINE__, ##args)
#define LogDel( format, args...)
#else
#define LogErr( format, args...)        Log( USR_OK, format " [%s:%d]", ##args, __FUNCTION__, __LINE__)
#define LogLib( format, args...)        Log( USR_OK, format " [%s:%d]", ##args, __FUNCTION__, __LINE__)
#define LogCri( format, args...)        Log( USR_OK, format " [%s:%d]", ##args, __FUNCTION__, __LINE__)
#define LogMsg( format, args...)        Log( USR_OK, format " [%s:%d]", ##args, __FUNCTION__, __LINE__)
#define LogDbg( format, args...)        Log( USR_OK, format " [%s:%d]", ##args, __FUNCTION__, __LINE__)
#define LogWar( format, args...)        Log( USR_OK, format " [%s:%d]", ##args, __FUNCTION__, __LINE__)
#define LogRaw( format, args...)        Log( USR_OK, format " [%s:%d]", ##args, __FUNCTION__, __LINE__)
#define LogDel( format, args...)
#endif
#endif	/* LOG_CONV_H */

