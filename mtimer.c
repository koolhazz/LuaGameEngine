/*
 * modified by winktzhong@gmail.com 
 * source from linux 3.0.8 kernel/timer.c
 */

#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "mtimer.h"
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define spin_lock(x)
#define spin_unlock(x)

#define spin_lock_irqrestore(x, y)
#define spin_unlock_irqrestore(x, y)

#define spin_lock_init(x)

#define spin_lock_irqsave(x, y)
#define spin_unlock_irqsave(x, y)

#define spin_lock_irq(x)
#define spin_unlock_irq(x)

#ifndef EXPORT
#define EXPORT static inline
#endif

/*
 * per-CPU timer vector definitions:
 */
#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)

struct tvec {
	struct list_head vec[TVN_SIZE];
};

struct tvec_root {
	struct list_head vec[TVR_SIZE];
};

struct tvec_base {
	int lock;
	struct timer_list *running_timer;
	unsigned long timer_jiffies;
	unsigned long next_timer;
	struct tvec_root tv1;
	struct tvec tv2;
	struct tvec tv3;
	struct tvec tv4;
	struct tvec tv5;
} ____cacheline_aligned;

static struct tvec_base* t_base = 0;
struct list_head *entry;

static inline void
init_stimer_vecs(timer_base_t* base, unsigned long jiffies)
{
    int j;
	spin_lock_init(&base->lock);

	for (j = 0; j < TVN_SIZE; j++) {
		INIT_LIST_HEAD(base->tv5.vec + j);
		INIT_LIST_HEAD(base->tv4.vec + j);
		INIT_LIST_HEAD(base->tv3.vec + j);
		INIT_LIST_HEAD(base->tv2.vec + j);
	}
	for (j = 0; j < TVR_SIZE; j++)
		INIT_LIST_HEAD(base->tv1.vec + j);

	base->timer_jiffies = jiffies;
	base->next_timer = base->timer_jiffies;
}

static inline void
internal_add_timer(timer_base_t* base, stimer_t * timer)
{
	unsigned long expires = timer->expires;
	unsigned long idx = expires - base->timer_jiffies;
	struct list_head *vec;

	if (idx < TVR_SIZE) {
		int i = expires & TVR_MASK;
		vec = base->tv1.vec + i;
	} else if (idx < 1 << (TVR_BITS + TVN_BITS)) {
		int i = (expires >> TVR_BITS) & TVN_MASK;
		vec = base->tv2.vec + i;
	} else if (idx < 1 << (TVR_BITS + 2 * TVN_BITS)) {
		int i = (expires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
		vec = base->tv3.vec + i;
	} else if (idx < 1 << (TVR_BITS + 3 * TVN_BITS)) {
		int i = (expires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK;
		vec = base->tv4.vec + i;
	} else if ((signed long) idx < 0) {
		/*
		 * Can happen if you add a timer with expires == jiffies,
		 * or you set a timer to go off in the past
		 */
		vec = base->tv1.vec + (base->timer_jiffies & TVR_MASK);
	} else {
		int i;
		/* If the timeout is larger than 0xffffffff on 64-bit
		 * architectures then we use the maximum timeout:
		 */
		if (idx > 0xffffffffUL) {
			idx = 0xffffffffUL;
			expires = idx + base->timer_jiffies;
		}
		i = (expires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK;
		vec = base->tv5.vec + i;
	}
	/*
	 * Timers are FIFO:
	 */
	list_add_tail(&timer->entry, vec);
}

/**
 * add_timer_on - start a timer on a particular CPU
 * @timer: the timer to be added
 * @cpu: the CPU to start it on
 *
 * This is not very scalable on SMP. Double adds are not possible.
 */
EXPORT int
stimer_add(stimer_t * timer)
{
	struct tvec_base *base = t_base;

	spin_lock_irqsave(&base->lock, 0);
	if (time_before(timer->expires, base->next_timer))
		base->next_timer = timer->expires;
	internal_add_timer(base, timer);
	spin_unlock_irqrestore(&base->lock, 1);
    
    return 0;
}

static inline int
detach_slowtimer(stimer_t * timer)
{
    if (!stimer_pending(timer)) {
        errno = ENOENT; //链表中根本就不存在该�?
        return -1;
    }
    list_del(&timer->entry);
    return 0;
}

/**
 * del_timer - deactive a timer.
 * @timer: the timer to be deactivated
 *
 * del_timer() deactivates a timer - this works on both active and inactive
 * timers.
 *
 * The function returns whether it has deactivated a pending timer or not.
 * (ie. del_timer() of an inactive timer returns 0, del_timer() of an
 * active timer returns 1.)
 */
EXPORT int
stimer_del(stimer_t * timer)
{
	struct tvec_base *base = t_base;
	int ret = 0;

	if (stimer_pending(timer)) {
	    spin_lock_irqsave(&base->lock, 0);
		if (stimer_pending(timer)) {
			detach_slowtimer(timer);
			if (timer->expires == base->next_timer)
				base->next_timer = base->timer_jiffies;
			ret = 1;
		}
		spin_unlock_irqrestore(&base->lock, 1);
	}

	return ret;
}

/**
 * mod_timer - modify a timer's timeout
 * @timer: the timer to be modified
 * @expires: new timeout in jiffies
 *
 * mod_timer() is a more efficient way to update the expire field of an
 * active timer (if the timer is inactive it will be activated)
 *
 * mod_timer(timer, expires) is equivalent to:
 *
 *     del_timer(timer); timer->expires = expires; add_timer(timer);
 *
 * Note that if there are multiple unserialized concurrent users of the
 * same timer, then mod_timer() is the only safe way to modify the timeout,
 * since add_timer() cannot modify an already running timer.
 *
 * The function returns whether it has modified a pending timer or not.
 * (ie. mod_timer() of an inactive timer returns 0, mod_timer() of an
 * active timer returns 1.)
 */
EXPORT void
stimer_mod(stimer_t * timer, unsigned long expires)
{
    int ret;

    spin_lock(&tcp_vs_stimer_lock);
    timer->expires = expires;
    ret = stimer_del(timer);
    internal_add_timer(timer->base, timer);
    spin_unlock(&tcp_vs_stimer_lock);
}

static int
cascade(struct tvec_base *base, struct tvec *tv, int index)
{
	/* cascade all the timers from tv up one level */
	struct timer_list *timer, *tmp;
	struct list_head tv_list;

	list_replace_init(tv->vec + index, &tv_list);

	/*
	 * We are removing _all_ timers from the list, so we
	 * don't have to detach them individually.
	 */
	list_for_each_entry_safe(timer, tmp, &tv_list, entry) {
		internal_add_timer(base, timer);
	}

	return index;
}

static void call_timer_fn(struct timer_list *timer, void (*fn)(void*),
			  void* data)
{
	fn(data);
}

#define INDEX(N) ((base->timer_jiffies >> (TVR_BITS + (N) * TVN_BITS)) & TVN_MASK)

static inline void
run_stimer_list(timer_base_t* base, unsigned long jiffies)
{
	struct timer_list *timer;

	spin_lock_irq(&base->lock);
	while (time_after_eq(jiffies, base->timer_jiffies)) {
		struct list_head work_list;
		struct list_head *head = &work_list;
		int index = base->timer_jiffies & TVR_MASK;

		/*
		 * Cascade timers:
		 */
		if (!index &&
			(!cascade(base, &base->tv2, INDEX(0))) &&
				(!cascade(base, &base->tv3, INDEX(1))) &&
					!cascade(base, &base->tv4, INDEX(2)))
			cascade(base, &base->tv5, INDEX(3));
		++base->timer_jiffies;
		list_replace_init(base->tv1.vec + index, &work_list);
		while (!list_empty(head)) {
			void (*fn)(void*);
			void* ctx;

			timer = list_first_entry(head, struct timer_list,entry);
			fn = timer->function;
			ctx = timer->ctx;

			base->running_timer = timer;
			detach_slowtimer(timer);

			spin_unlock_irq(&base->lock);
			call_timer_fn(timer, fn, ctx);
			spin_lock_irq(&base->lock);
		}
	}
	base->running_timer = NULL;
	spin_unlock_irq(&base->lock);
}

EXPORT timer_base_t* stimer_init()
{
    timer_base_t* base = malloc(sizeof(timer_base_t));
    if (!base) 
        return 0;
    /* initialize the slowtimer vectors */
    init_stimer_vecs(base, time(NULL));
    t_base = base;
    return base;
}

EXPORT void stimer_cleanup(timer_base_t* base)
{
    if (!base)
        base = t_base;
    free(base);
}

EXPORT void stimer_base_cleanup()
{
    if (!t_base)
        free(t_base);
}
/**
 * 设置定时�?
 * @param expires 过期时间，单位：微秒。注意是相对时间
 */
EXPORT void stimer_set(stimer_t* timer, unsigned long expires, void (*fn) (void*), void* ctx)
{
	timer->entry.next = timer->entry.prev = NULL;
    timer->expires = time(NULL)+ expires;
    timer->ctx = ctx;
    timer->function = fn;
    timer->base = t_base;
    entry = &timer->entry;
}

EXPORT void stimer_base_set(stimer_t* timer, timer_base_t* base)
{
    timer->base = base;
}

/*
 * The function to collect stale timers must be activated from the
 * outside periodically (such as every second).
 */
EXPORT void stimer_collect()
{
    run_stimer_list(t_base, time(NULL));
}
EXPORT void stimer_base_collect(timer_base_t* base)
{
    run_stimer_list(base, time(NULL));
}


