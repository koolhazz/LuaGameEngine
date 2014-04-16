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
	
    int HashSetRedisValue(const char* key, const int field, const char* value);
    int HashGetRedisValue(const char* key, const int field);
    int DelRedisValue(const char* key);
    int DelRedisHashValue(const char* key, const int field);
    
	bool IsActived();
private:
    redisContext*	m_redis;	
    redisReply*		m_reply;	
};

#endif
