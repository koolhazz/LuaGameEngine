#ifndef __TIME_WHEEL_H_
#define __TIME_WHEEL_H_

#include <set>

using namespace std;

class SocketHandler;

#ifndef TIME_WHEEL_SIZE 
#define TIME_WHEEL_SIZE 		15 /* 轮盘的刻度大小 */
#endif

typedef std::set<SocketHandler*> 	time_scale_t;
typedef time_scale_t::iterator 		time_scale_itr_t;
typedef time_scale_t* 				time_wheel_t[15];

extern int
tw_create(time_wheel_t** t, int len);

/* 返回需要关闭的time_scale_t */
extern time_scale_t*
tw_wheeling(time_wheel_t* t, int len);

extern int
tw_insert(time_wheel_t* t, int len, SocketHandler* s);



#endif 
