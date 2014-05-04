#include "protocal.h"
#include "log.h"

/* the Lua interpreter */
extern lua_State* L;

static inline message_t*
__new_message()
{
	message_t* p = (message_t*)malloc(sizeof *p);

	if (p) {
		memset(p, 0, sizeof *p);
	}

	return p;
}

CProtocal::CProtocal()
{
}

CProtocal::~CProtocal()
{
}

message_map_t CProtocal::messages = message_map_t();

int 
CProtocal::init()
{
	message_t *m;

    luaL_openlibs(L);
    // Load file.
    if(luaL_dofile(L, "script/protocal.lua")) {
        log_error("Cannot run file:%s\n",lua_tostring(L,-1));
        return -1;
    }

    lua_getglobal(L, "protocal_define_table");
    lua_pushnil(L);
    while(lua_next(L, -2) != 0) {
		m = __new_message();

		if (m == NULL) {
			log_error("new message failed.");
			return -1;
		}
		
        lua_pushnil(L);
        lua_next(L, -2);    //cmd
        m->cmd = (unsigned short)lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_next(L, -2);    //format
        strcpy(m->format, lua_tostring(L, -1));
        lua_pop(L, 1);
        
        lua_next(L, -2);    //desc
        strcpy(m->desc, lua_tostring(L, -1));
        lua_pop(L, 1);
        
        lua_next(L, -2);    //callback
        strcpy(m->handler, lua_tostring(L, -1));
        lua_pop(L, 1);

        lua_next(L, -2);
        lua_pop(L, 1);
        
        messages[m->cmd] = m;
    }

	trace_message();

	return 0;
}

void 
CProtocal::trace_message()
{
    message_map_itr_t iter;
	
    for(iter = messages.begin(); iter != messages.end(); iter++) {
        log_debug("cmd: [%#x] Format: [%s] desc: [%s] callback: [%s]", 
			iter->second->cmd, iter->second->format, iter->second->desc, iter->second->handler);
    }

}

message_t* 
CProtocal::get_message(unsigned short cmd)
{
    message_map_itr_t iter = messages.find(cmd);
    if(iter != messages.end()) {
        return iter->second;
    }
	
	return NULL;     //return default message
}

