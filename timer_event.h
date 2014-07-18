#ifndef BOYAA_TIMER_EVENT_H_20110313
#define BOYAA_TIMER_EVENT_H_20110313

#include "timer.h"
#include <stdio.h>
#include <map>
#include <tr1/unordered_map>
using namespace std;
using namespace tr1;

class TimerEvent;

typedef tr1::unordered_map<unsigned long, TimerEvent*> 	timer_list_t;
typedef timer_list_t::iterator							timer_list_itr_t;

class TimerEvent {
public:
	TimerEvent();
	~TimerEvent(void);
	void SetTimerId(unsigned long timer_id);
	void StartTimer(int sec, int usec = 0);
	void StopTimer();
	void ResetTimer();

	int GetRemain(); // add austinch at 2012/06/13

public:
	void OnTimer(unsigned long timer_id);

public:
	time_ev 				m_ev;
	int 					m_timeout;
	unsigned long 			m_guid;			//每个TimerEvent的唯一id
	static unsigned long 	m_increase_id;	
};

#endif
