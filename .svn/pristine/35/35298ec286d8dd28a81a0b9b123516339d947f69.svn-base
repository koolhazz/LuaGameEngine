
#include <stdio.h>
#include "timer.h"
#include "llist.h"	


	#include "mtimer.c"
	#include "plex.h"
	plex_t timer_plex;


// 初始化定时器
void init_timer()
{
	stimer_init();
	plex_init(&timer_plex, sizeof(stimer_t));
}
// Linux定时器回调函数
void on_timer(void* ctx)
{
	struct time_ev* ev = ctx;
	//释放定时器
	stop_timer(ev);	    
	ev->callback(ev->ptr);
}
// 投递一个定时器
int start_timer(int sec, int usec, struct time_ev* ev)
{
	void* timer = 0;

	//timer malloc
	timer = plex_alloc(&timer_plex);
	if (timer == NULL) {
		if (0 != plex_expand(&timer_plex, 64, &sys_malloc, 0)) {
			return -1;
		}
		timer = plex_alloc(&timer_plex);
		assert(timer);
	}

	//add timer
	stimer_set(timer, sec, &on_timer, ev);
	if (0 != stimer_add(timer)) {
		plex_free(&timer_plex, timer);
		return -1;
	};
	ev->timer = timer; //定时器赋值
	return 0;
}
// 删除一个定时器
int stop_timer(struct time_ev* ev)
{
	if (ev->timer) {
		stimer_del(ev->timer);//无论是否成功，该定时器都不在链表中，故可以删除内存
		plex_free(&timer_plex, ev->timer);
		ev->timer = NULL;
	}
	return 0;
}
// 轮询定时器
void run_timer()
{
	stimer_collect();
}

//返回定时器剩余时间
int remain_timer(struct time_ev* ev)
{	
	stimer_t* timer = (stimer_t*)(ev->timer);

	if (NULL == timer)
	{
		return 0;
	}

	int remain = timer->expires - time(NULL); 

	return remain;
}


