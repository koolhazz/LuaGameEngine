#ifndef BOYAA_PROTOCAL_H_20120207
#define BOYAA_PROTOCAL_H_20120207

extern "C"
{
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}
#include <string.h>
#include <map>
using namespace std;

typedef int (*CALLBACK_FUNC)(const char* format, ...);      //命令处理回调函数

typedef struct _message {
    int  cmd;               //命令字类型
    char format[128];       //格式
    char desc[256];         //描述
    char call_back[32];     //回调函数
    _message()
    {
        memset(this, 0, sizeof(_message));
    }
}Message;

typedef map<unsigned short, Message> MessageMap_t;
typedef MessageMap_t::iterator MessageMapItr_t;


class CProtocal
{
public:
    CProtocal();
    ~CProtocal();

public:
	static int init();
    static void trace_message();
    static Message get_message(unsigned short cmd);
public:
    static int message_count;
    static MessageMap_t message_table;
};

#endif

