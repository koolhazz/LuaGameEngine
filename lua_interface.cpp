#include <stdarg.h>
#include <string.h>
#include "lua_interface.h"
#include "wtypedef.h"
#include "log.h"


extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

extern lua_State* L;

int 
call_lua(const char *func, const char *sig, ...) 
{
    va_list vl;
    int narg, nres;   /* number of arguments and results */
 
    va_start(vl, sig);
    lua_getglobal(L, func);  /* get function */
 
    /* push arguments */
    narg = 0;
    while (*sig) {    /* push arguments */
		switch (*sig++) {
 
		case 'f':  /* double argument */
			lua_pushnumber(L, va_arg(vl, double));
			break;
 
		case 'd':  /* int argument */
			lua_pushnumber(L, va_arg(vl, int));
            break;
 
        case 's':  /* string argument */
            lua_pushstring(L, va_arg(vl, char *));
            break;
	
        case '>':
            goto endwhile;
 
        default:
            {
                log_error("invalid option (%c)", *(sig - 1));
                return -1;
            }
        }
        narg++;
        luaL_checkstack(L, 1, "too many arguments");
    } endwhile:
 
    /* do the call */
    nres = strlen(sig);  /* number of expected results */
    if (lua_pcall(L, narg, nres, 0) != 0)  /* do the call */
    {
        log_error( "error running function `%s': %s", func, lua_tostring(L, -1));
        return -1;
    }   
 
    /* retrieve results */
    nres = -nres;     /* stack index of first result */
    while (*sig) {    /* get results */
        switch (*sig++) {
 
        case 'f':  /* double result */
            if (!lua_isnumber(L, nres))
            {
                log_error( "wrong result type");
                return -1;
            }
            *va_arg(vl, double *) = lua_tonumber(L, nres);
            break;
 
        case 'd':  /* int result */
            if (!lua_isnumber(L, nres))
            {
                log_error("wrong result type");
                return -1;
            }
              
           *va_arg(vl, int *) = (int)lua_tonumber(L, nres);
           break;
 
        case 's':  /* string result */
            if (!lua_isstring(L, nres)) {
                log_error("wrong result type");
                return -1;
            }   
           *va_arg(vl, const char **) = lua_tostring(L, nres);
           break;
 
       default:
           {
               log_error("invalid option (%c)", *(sig - 1));
               return -1;
           }
       }
       nres++;
    }
    va_end(vl);
    return 0;
}

