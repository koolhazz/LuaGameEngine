#ifndef MOD_TIMER_H
#define MOD_TIMER_H

#define __KERNEL__
#include <stddef.h>
#include <sys/times.h>
#include <timer.h>
#include "clist.h"

#define time_after(a,b) ((long)(b) - (long)(a) < 0)
#define time_before(a,b) time_after(b,a)

#define time_after_eq(a,b) ((long)(a) - (long)(b) >= 0)
#define time_before_eq(a,b) time_after_eq(b,a) 

typedef struct tvec_base timer_base_t;

typedef struct timer_list {
	/*
	 * All fields that change during normal runtime grouped to the
	 * same cacheline
	 */
	struct list_head entry;
	unsigned long expires;
    void* ctx;
	void (*function)(void*);
	
    timer_base_t *base;

	int slack;
} stimer_t;

/**
 * 设置定时�?
 * @param expires 过期时间，单位：微秒。注意是相对时间 
 * 接口函数
 */

static timer_base_t* stimer_init();
static void stimer_set(stimer_t* timer, unsigned long expires, void (*fn) (void*), void* ctx);
static int  stimer_add(stimer_t * timer);
static int  stimer_del(stimer_t * timer);
static void stimer_mod(stimer_t * timer, unsigned long expires);
static void stimer_collect();
static void stimer_base_cleanup();

static void stimer_cleanup(timer_base_t*);
static void stimer_base_set(stimer_t* timer, timer_base_t*);
static void stimer_base_collect(timer_base_t*);

static inline int stimer_valid(const stimer_t * timer)
{
	return timer->entry.next != NULL;
}
#define stimer_pending stimer_valid

#endif
