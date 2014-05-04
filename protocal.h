#ifndef __PROTOCAL_H_
#define __PROTOCAL_H_

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <string.h>
#include <map>
using namespace std;

typedef int (*CALLBACK_FUNC)(const char* format, ...);      //命令处理回调函数

typedef struct message_s message_t;
struct message_s {
    unsigned short  cmd;               //命令字类型
    char 			format[64];       //格式
    char 			desc[128];        //描述
    char 			handler[64];     //回调函数
};

typedef map<unsigned short, message_t*> message_map_t;
typedef message_map_t::iterator 		message_map_itr_t;


class CProtocal {
public:
    CProtocal();
    ~CProtocal();

public:
	static int init();
    static void trace_message();
    static message_t* get_message(unsigned short cmd);
public:
    static message_map_t messages;
};

#endif

