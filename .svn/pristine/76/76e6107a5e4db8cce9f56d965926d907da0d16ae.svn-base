#ifndef __TTCLOG_H__
#define __TTCLOG_H__

#ifndef PLAT_WIN32

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <stdio.h>

#define DEFAULT_FILE_NUM    10
#define DEFAULT_FILE_SIZE   50*1024*1024

#if CLIENTAPI
#define __log_level__ 4
#else
extern int __log_level__;
#endif
#define log_generic(lvl, fmt, args...)	 write_log(lvl, __FILE__, __FUNCTION__, __LINE__ , fmt, ##args)
#define log_error(fmt, args...)		log_generic(1, fmt, ##args)
#define log_info(fmt, args...)		log_generic(2, fmt, ##args) //do{ if(__log_level__>=2)log_generic(2, fmt, ##args); } while(0)
#define log_debug(fmt, args...)		log_generic(3, fmt, ##args) //do{ if(__log_level__>=3)log_generic(3, fmt, ##args); } while(0)


#if CLIENTAPI
#if __cplusplus
static inline void init_log (const char *app, const char *dir = NULL, int max_num = -1, int max_size = -1) {}
#else
static inline void init_log (const char *app, const char *dir, int max_num, int max_size) {}
#endif
static inline void set_log_level(int n){}
static inline void write_log (int n, const char *a, const char *b, int c, const char *d, ...) {}
#else
#if __cplusplus
extern void init_log (const char *app, const char *dir = NULL, int max_num = -1, int max_size = -1);
#else
extern void init_log (const char *app, const char *dir, int max_num, int max_size);
#endif
extern void set_log_level(int);
extern void write_log (int, const char*, const char *, int, const char *, ...) __attribute__((format(__printf__,5,6)));
extern void write_access(int access, const char* rsp_buf, const char* fmt, ...) __attribute__((format(__printf__,3,4)));

extern void __time_mark();

#endif

__END_DECLS



#else
void init_log (const char *app, const char *dir = NULL, int max_num = -1, int max_size = -1);
void set_log_level(int);
void write_log (int, const char*, const char *, int, const char *, ...);
void log_boot(const char* fmt, ...);

#define log_info  log_boot
#define	log_error log_boot
#define	log_debug log_boot

#endif

#define	TRACE printf

#endif
