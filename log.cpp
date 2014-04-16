#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

#include "log.h"

#ifndef PLAT_WIN32
#include <unistd.h>

#define LOGSIZE         1 * 1024 * 1024
#define MAX_PATH_LEN    256

int __log_level__ = 6;

static char log_dir[128] = "../log";
static char appname[32];
static int  max_file_num    = DEFAULT_FILE_NUM;
static int  max_file_size   = DEFAULT_FILE_SIZE;
static int  cur_file_pos    = 0;

extern bool is_daemon;
void init_log (const char *app, const char *dir, int max_num, int max_size)
{
    strncpy(appname, app, sizeof(appname)-1);

    if(dir) {
        strncpy (log_dir, dir, sizeof (log_dir) - 1);
    }
    mkdir(log_dir, 0777);
    if(access(log_dir, W_OK|X_OK) < 0)
    {
        log_error("logdir(%s): Not writable", log_dir);
    }

    if(max_num > 0)
        max_file_num = max_num;
    
    if(max_size > 0)
        max_file_size = max_size;
}

void set_log_level(int l)
{
    if(l>=0)
        __log_level__ = l > 4 ? l : 4;
}

void write_access(int access, const char* rsp_buf, const char* fmt, ...)
{
    if (0 == access)
        return;

    char  rspinfo[MAX_PATH_LEN] = {'\0'};
    int   rsplen                = 0;
    char* checkpoint            = strstr ((char *)rsp_buf, "\r\n");

    if (NULL == checkpoint)
        return;

    rsplen = checkpoint - rsp_buf;
    if(rsplen > MAX_PATH_LEN - 1)
        rsplen = MAX_PATH_LEN - 1;

    memcpy (rspinfo, rsp_buf, rsplen);
    rspinfo[rsplen] = 0x00;

    // save errno
    int savedErrNo = errno;
    int off = 0;
    char buf[LOGSIZE];
    char logfile[MAX_PATH_LEN];

    struct tm tm;
    time_t now = time (NULL);
    localtime_r(&now, &tm);
    off = snprintf (buf, LOGSIZE - 1, "[%02d:%02d:%02d] : response info[%s] ", tm.tm_hour, tm.tm_min, tm.tm_sec, rspinfo);

    snprintf (logfile, MAX_PATH_LEN - 1, "%s/%s.access%04d%02d%02d.log",
            log_dir, appname,
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    va_list ap;
    va_start(ap, fmt);
    // restore errno
    errno = savedErrNo;
    off += vsnprintf(buf+off, LOGSIZE-off-2, fmt, ap);
    va_end(ap);

    if(buf[off-1] != '\n')
        buf[off++] = '\n';

    if(appname[0])
    {
        int fd = open (logfile, O_CREAT | O_LARGEFILE | O_APPEND |O_WRONLY, 0644);

        if (fd >= 0)
        {
            write(fd, buf, off);
            close (fd);
        }
    }
}

void write_log (int level, const char *filename, const char *funcname, int lineno, const char *format, ...)
{
    if(is_daemon && level > __log_level__)  //守护进程并且该level不需要记录，则跳过
    {
        return;
    }
    // save errno
    int 		savedErrNo = errno;
    int 		off = 0;
    int 		n_write;
    char 		buf[LOGSIZE];
    char 		logfile[512];
    int 		fd = -1;
    struct 		stat stBuf;
    bool 		btruncate = false;
    struct tm 	tm;
    // time_t now = time (NULL);

	struct timeval tv;
	
    gettimeofday(&tv, NULL);

    //localtime_r(&now, &tm);

	localtime_r(&tv.tv_sec, &tm);
    
    filename = basename(filename);
    off = snprintf (buf, LOGSIZE-1,
            "<%d>[%04d%02d%02d-%02d:%02d:%02d.%03ld] pid[%d]: %s(%d)[%s]: ",
            level,
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec / 1000,
            getpid(),
            filename, lineno, funcname
            );

    va_list ap;
    va_start(ap, format);
    // restore errno
    errno = savedErrNo;
    n_write= vsnprintf(buf+off, LOGSIZE-off-2, format, ap);
    va_end(ap);

    if(n_write>=LOGSIZE-off-2)
        off=LOGSIZE-2;
    else
        off+=n_write;
    
    if(buf[off-1] != '\n')
        buf[off++] = '\n';

    if (!is_daemon)
        fwrite(buf, 1, off, stderr);

    do
    {
        if(!appname[0])
        {
            fprintf(stderr, "ATTENTION: the prefix of the log file is not set\n");
            return;
        }
        
        snprintf (logfile, MAX_PATH_LEN-1, "%s/%s_%d.log",
                log_dir, appname,
                cur_file_pos);

        int flags = O_CREAT | O_LARGEFILE | O_APPEND | O_WRONLY;
        if(btruncate)
            flags |= O_TRUNC;

        fd = open (logfile, flags, 0644);
        if (fd < 0)
        {
            fprintf(stderr, "ATTENTION: open log file failed, dir[%s] file[%s], error[%m]\n", log_dir, appname);
            return;
        }

        if(-1 == fstat(fd, &stBuf))
        {
            fprintf(stderr, "ATTENTION: stat log file failed, dir[%s] file[%s], error[%m]\n", log_dir, appname);
            return;
        }

        if((int)stBuf.st_size >= max_file_size)
        {
            cur_file_pos = (cur_file_pos + 1)%max_file_num;
            close(fd);
            fd = -1;
            btruncate = true;
            continue;
        }
        else
        {
            break;
        }

    }while(true);

    write(fd, buf, off);
    close (fd);
}

void
__time_mark()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	log_debug("Mark: %lu:%lu", tv.tv_sec, tv.tv_usec);
}

#else
//windows版本直接打印到控制台
void init_log (const char *app, const char *dir, int max_num, int max_size){}
void set_log_level(int){}
void write_log (int, const char*, const char *, int, const char *, ...) {}

void log_boot(const char* format, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, format);
	int nWrittenBytes = vfprintf(stderr, format, arg_ptr);
	va_end(arg_ptr);
}

#endif

