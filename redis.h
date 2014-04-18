#ifndef __CREDIS_H__
#define __CREDIS_H__

#include "hiredis.h"

class CRedis {
public:
    CRedis();
    ~CRedis();

public:
    int connect_redis(const char* host, unsigned int port, unsigned short second = 0);
    int get_value(const char* key);
    int set_value(const char* key, const char* value);
    int set_expire(const char* key, int expire);

	int Enqueue(const char* queue, const char* value);
	int Dequeue(const char* queue);

	int S_IsMember(const char* key, const char* value);
	int S_IsMember(const char* key, const int value);
    int HSet(const char* key, const int field, const char* value);
    int HGet(const char* key, const int field);
    int Del(const char* key);
    int HDel(const char* key, const int field);
    
	bool IsActived();
private:
    redisContext*	m_redis;	
    redisReply*		m_reply;	

};

#endif
