#include "redis.h"
#include "interface_c.h"
#include "log.h"

#include <string.h>

extern "C" 
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}


extern lua_State* L;

CRedis::CRedis()
	:m_redis(NULL),
	m_reply(NULL)
{

}

CRedis::~CRedis()
{
	if (m_reply) freeReplyObject(m_reply);

	if (m_redis) redisFree(m_redis);
}

int 
CRedis::connect_redis(const char* host, 
					  unsigned int port, 
					  unsigned short second)
{
	if(m_reply) {
		freeReplyObject(m_reply);
		m_reply = NULL;
	}

	if (m_redis) {
		redisFree(m_redis);
		m_redis = NULL;
	}

	struct timeval tv;
	tv.tv_sec = second;
	tv.tv_usec = 0;

    m_redis = redisConnectWithTimeout(host, port, tv);
	
    if(m_redis->err) {
		redisFree(m_redis);
		m_redis = NULL;
		
		return -1;
    }
	
    return 0;
}

int CRedis::get_value(const char* key)
{
    m_reply = reinterpret_cast<redisReply*>(redisCommand(m_redis, "GET %s", key));
    if(m_reply)
    {
        int result = 0;

		//log_debug("m_reply->type:%d\n", m_reply->type);
			
        if(1 == m_reply->type)
        {
           lua_pushstring(L, m_reply->str); 
           lua_setglobal(L, REDIS_RESULT); 
        }
        freeReplyObject(m_reply);
		m_reply = NULL;
		
        return result;
    }
    return 0;
}

int CRedis::set_value(const char* key, const char* value)
{
    m_reply = reinterpret_cast<redisReply*>(redisCommand(m_redis, "SET %s %s", key, value));
    if(m_reply)
    {
        int result = (m_reply->type==4) ? -1:0;

		freeReplyObject(m_reply);
		m_reply = NULL;
		
		return result;
    }
    return 0;
}

int CRedis::set_expire(const char* key, int expire)
{
    m_reply = reinterpret_cast<redisReply*>(redisCommand(m_redis, "EXPIRE %s %d", key, expire));
    if(m_reply)
    {
        int result = (m_reply->type==4) ? -1:0;

		freeReplyObject(m_reply);
		m_reply = NULL;

		return result;
    }
    return -1;
}

int 
CRedis::Enqueue(const char* queue, const char* value)
{
	m_reply = static_cast<redisReply*>(redisCommand(m_redis, "rpush %s %s", queue, value));

	if (m_reply)
	{
		int result = (m_reply->type == 4) ? -1 : 0;

		freeReplyObject(m_reply);
		m_reply = NULL;

		return result;
	}

	return -1;
}


int
CRedis::Dequeue(const char* queue)
{
	m_reply = static_cast<redisReply*>(redisCommand(m_redis, "lpop %s", queue));

	if (m_reply)
	{
		int result = 0;

		if (m_reply->type == 1)
		{
			lua_pushstring(L, m_reply->str);
			lua_setglobal(L, REDIS_RESULT);
		}

		freeReplyObject(m_reply);
		m_reply = NULL;

		return result;
	}

	return -1;
}


bool
CRedis::IsActived()
{
	if (m_redis == NULL)
	{
		return false;
	}

	m_reply = static_cast<redisReply*>(redisCommand(m_redis, "ping"));

	if (m_reply)
	{
		//log_debug("m_reply->type:%d\n", m_reply->type);
		
		if (m_reply->type == 5)
		{
			//log_debug("m_reply->str:%s\n", m_reply->str);
			
			if(strcmp("PONG", m_reply->str) == 0)
			{
				freeReplyObject(m_reply);
				m_reply = NULL;
				
				return true;
			}
		}

		// 2012.12.10
		freeReplyObject(m_reply);
		m_reply = NULL;
	}

	return false;
}

int
CRedis::S_IsMember(const char* key, const char* value)
{
	m_reply = static_cast<redisReply*>(redisCommand(m_redis, "sismember %s %s", key, value));

	if (m_reply)
	{
		int result = 0;

		if (m_reply->type == 3)
		{
			result = m_reply->integer == 1 ? 1 : 0;
		}

		freeReplyObject(m_reply);
		m_reply = NULL;

		return result;
	}

	return -1;
}

int
CRedis::S_IsMember(const char* key, const int value)
{
	m_reply = static_cast<redisReply*>(redisCommand(m_redis, "sismember %s %d", key, value));

	if (m_reply)
	{
		int result = 0;

		if (m_reply->type == 3)
		{
			result = m_reply->integer == 1 ? 1 : 0;
		}

		freeReplyObject(m_reply);
		m_reply = NULL;

		return result;
	}

	return -1;	
}

int CRedis::HashSetRedisValue(const char* key,const int field, const char* value)
{
    m_reply = reinterpret_cast<redisReply*>(redisCommand(m_redis, "HSET %s %d %s", key,field,value));

    if(m_reply) {
        log_debug("HashSetRedisValue:m_reply->type:%d\n", m_reply->type);

		freeReplyObject(m_reply);
		m_reply = NULL;
		
		return 1;
    }
    return 0;
}

int CRedis::HashGetRedisValue(const char* key, const int field)
{
    m_reply = reinterpret_cast<redisReply*>(redisCommand(m_redis, "HGET %s %d ", key, field));
    int result = -1;
    
    if(m_reply) {
        if(m_reply->type == 1) {
			lua_pushstring(L, m_reply->str);
			lua_setglobal(L, REDIS_RESULT);
			result = 0;
        } else {
        	lua_pushstring(L, "");
        	lua_setglobal(L,REDIS_RESULT);
        }
        
		freeReplyObject(m_reply);
		m_reply = NULL;

		return result;
    }
    
    return -1;
}

int CRedis::DelRedisValue(const char* key)
{
    m_reply = reinterpret_cast<redisReply*>(redisCommand(m_redis, "DEL %s ", key));
    if(m_reply) {

		freeReplyObject(m_reply);
		m_reply = NULL;
		
		return 1;
    }
    
    return 0;
}

int CRedis::DelRedisHashValue(const char* key,const int field)
{
    m_reply = reinterpret_cast<redisReply*>(redisCommand(m_redis, "HDEL %s %d ", key,field));
    if(m_reply)
    {
        log_debug("DelRedisHashValue:m_reply->type:%d\n", m_reply->type);
		freeReplyObject(m_reply);
		m_reply = NULL;
		
		return 1;
    }
    return 0;
}

