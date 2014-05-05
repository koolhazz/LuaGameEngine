#ifndef __PROTOCOL_H_
#define __PROTOCOL_H_

extern "C" 
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <string.h>
#include <map>
using namespace std;

typedef struct message_s message_t;
struct message_s {
    unsigned short  cmd;               //命令字类型
    char 			format[64];       //格式
    char 			desc[128];        //描述
    char 			handler[64];     //回调函数
};

typedef map<unsigned short, message_t*> message_map_t;
typedef message_map_t::iterator 		message_map_itr_t;

extern int
message_init();

extern void
message_trace();

extern message_t* 
message_get(unsigned short cmd);

extern message_map_t* messages;

#endif

