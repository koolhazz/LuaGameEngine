#include "net.h"
#include "timer.h"
#include "timer_event.h"
#include "log.h"
#include "protocal.h"
#include "getopt.h"
#include "daemonize.h"
#include "lua_interface.h"
#include "mysql_part.h"
#include "redis.h"
#include "interface_c.h"
#include "timewheel.h"

#include <unistd.h>
#include <sys/types.h>
#include <sched.h>
#include <signal.h>


#define LUA_GAME_ENGINE_VERSION 		"1.4.2b201312101700"
#define LUA_GAME_ENGINE_VERSION_MAJOR 	1
#define LUA_GAME_ENGINE_VERSION_MINOR 	4
#define LUA_GAME_ENGINE_VERSION_BUGFIX 	2

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_interface_open (lua_State* tolua_S);

#define CPU_NUM sysconf(_SC_NPROCESSORS_CONF)

static const char* PidPath = "./server.pid";

static int
__cpus_nums()
{
	return sysconf(_SC_NPROCESSORS_CONF);
}

static void 
__binding_cpu(void)
{
	int curr_cpu_max = __cpus_nums();
	
	srand(time(NULL));
	
	int num = rand() % curr_cpu_max;

	while(!num) {
		num = rand() % curr_cpu_max;
	}

	log_debug("CPU: %d\n", num);
	
	cpu_set_t mask;

	__CPU_ZERO(&mask);

	__CPU_SET(num, &mask);

	sched_setaffinity(0,sizeof(cpu_set_t),&mask);	
}

static void
__reload_config(int signo)
{
	int ret, rvalue;
	
	ret = call_lua("reload_config", ">d", &rvalue);

	if (ret != 0) {
		log_debug("reload_config failed.");
	} else {
		log_debug("reload_config success.");
	}
}

static void
__signal_binding()
{
	struct sigaction act;
	struct sigaction act2;
	
	sigemptyset(&act.sa_mask);
	act.sa_handler = SIG_IGN;

	//sigaction(SIGHUP, &act, NULL);
	sigaction(SIGPIPE, &act, NULL);

	sigemptyset(&act2.sa_mask);
	act2.sa_handler = __reload_config;

	sigaction(SIGHUP, &act2, NULL);
}

static void
__version()
{
	log_debug("%s", LUA_GAME_ENGINE_VERSION);
}

static void
__pidfile()
{
	int fd = open(PidPath, O_CREAT | O_RDWR | O_TRUNC, 00666);

	if (fd <= 0) {
		log_error("create pidfile failed.");
		return;
	}

	char temp[64] = {0};

	snprintf(temp, 64, "%d", getpid());

	int ret = write(fd, temp, strlen(temp));

	if (ret <= 0) {
		log_error("write pid failed. %d %d", ret, errno);
	}
}

lua_State* 		L;
CMysql 			mysql_handle;
CRedis 			redis_handle;
bool 			is_daemon = false;

int				now; /* 当前系统时间 */
time_wheel_t	*g_tw; 

#ifdef BeginHandlerNums
Net net(BeginHandlerNums);
#else
Net net(0);
#endif

int 
main(int argc, char ** argv)
{
	pid_t 	pid;
	char 	log_prefix[50] = {0};

	now = time(NULL);

    L = lua_open();     /* initialize Lua */
    luaL_openlibs(L);   /* load Lua base libraries */
    tolua_interface_open(L);
    luaL_dofile (L, "script/server.lua");    /* load the script */
 
    // 创建一个新的表    
    lua_newtable(L);    
    int result;
    char* option = "a:b:c:de:f:g:h:i:j:k:l:m:n:o:p:q:r:s:t:u:v:w:x:y:z:";
    while((result = getopt(argc, argv, option)) != -1) {
        if(result >= 'a' && result <= 'z') {
            if(result == 'd') {
                is_daemon = true;
            } else {
                char s[2]={0};
                sprintf(s, "%c", result);
                lua_pushstring(L, s);    
                lua_pushstring(L, optarg);    
                lua_rawset(L, -3);
            }
        }
    }   
    
    lua_setglobal(L, COMMAND_ARGS);    

    //初始化日志文件


    //log_info("Initing\n");

	if (is_daemon) {
        if ( -1 == daemon(1, 0)) {
            fprintf(stdout, "%s\n", "daemon error.");
        }
    }
   
   	pid = getpid();
	snprintf(log_prefix, sizeof(log_prefix), "Log_%d", pid); 
    init_log(log_prefix, "./");

	init_timer();
	__binding_cpu();
	__signal_binding();

	__pidfile();
	__version();

	CProtocal::init();

    if(!net.init()) {
        log_error("init server failed");
        return -1;
    }

    int ret = -1;
    if(call_lua("handle_init", ">d", &ret)) {
        return -1;
    }

    if(ret < 0) {
        log_error("call lua function handle_init failed");
        return -1;
    }
	
    if(!net.start_server()) {
        log_error("start server failed");
        return -1;
    }

    return 0;
}
