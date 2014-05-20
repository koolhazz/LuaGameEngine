#include "net.h"
#include "timer.h"
#include "llist.h"
#include "log.h"
#include "lua_interface.h"
#include "timewheel.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <time.h>

#define EVENT_TOTAL_COUNT 256	
#define CHECK_POINT_TIMES 10

extern int 				now;  /* 缓存当前系统时间，减少time的调用 */
extern time_wheel_t		*g_tw;

static inline int 
__set_linger_socket(int fd) 
{
	struct linger so_linger;
	
	so_linger.l_onoff = 1;
	so_linger.l_linger = 0; //强制关闭
	
	int ret = setsockopt(fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof so_linger);
		
	return ret;
}

static inline int  
__set_non_block(int fd)
{
	unsigned long argp = 1;
	ioctl(fd, FIONBIO, &argp);

	int flag = 1;
	int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));	
	return ret;
}

static inline int
__set_defer_accept(int fd) 
{
	unsigned int timeout = 10;

	int ret = setsockopt(fd, IPPROTO_TCP, TCP_DEFER_ACCEPT, (char*)&timeout, sizeof timeout);

	return ret;
}

/* socket 注册事件 */
/*static inline void
__add_event(int efd, int fd) 
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN | EPOLLET;

	epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev);	
}*/

static inline void
__add_event(int efd, SocketHandler* s) 
{
	struct epoll_event 	ev;
	int 				fd;
	unsigned int 		instance;

	instance = s->readev()->instance;
		
	s->readev()->active = 1;
	s->readev()->instance = !instance;
	s->writeev()->instance = !instance;	

	fd = s->fd();
	ev.data.ptr = (void*)((uintptr_t)s | s->readev()->instance);
	ev.events = EPOLLIN | EPOLLET;

	epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev);	
}

/* socket 添加读事�?*/
static inline void
__add_read_event(int efd, int fd)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN | EPOLLET;

	epoll_ctl(efd, EPOLL_CTL_MOD, fd, &ev);
}

/* socket 添加写事�?*/
static inline void
__add_write_event(int efd, int fd)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLOUT | EPOLLET;

	epoll_ctl(efd, EPOLL_CTL_MOD, fd, &ev);	
}

/* socket 删除监听 */
static inline void
__del_event(int efd, int fd)
{
	epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
}

#define FreeHandler(h, b) 	\
	(h)->clean(b)

#define FreeStaleSocket(h, f) 	\
	if (f != (h)->fd()) { 		\
		::close((h)->fd());  	\
	}

#define sinsert(e) 		m_HandlerCloseSet.insert(e)
#define sbegin 			m_HandlerCloseSet.begin()
#define send 			m_HandlerCloseSet.end()
#define serase(e)		m_HandlerCloseSet.erase(e)

#define hclosed(h)		h->SetClosed(true)

#define merase(k)				\
	m_Handlers.erase(k);		\
	_stat_hdr_sub()
	
#define minsert(fd, p)			\
	m_Handlers[(fd)] = (p);		\
	_stat_hdr_inc()

#define lpop(p)										\
	p = *(m_FreeHandlers.begin()); 					\
	m_FreeHandlers.erase(m_FreeHandlers.begin()); 	\
	_stat_hdr_free_sub()
	
#define lpush(p)				\
	m_FreeHandlers.insert(p);	\
	_stat_hdr_free_inc()

#define lfactor			10
#define lincream()	do {																	\
		for (int i = 0; i < lfactor; i++) {													\
			SocketHandler* p = new SocketHandler(-1, true, true, CONNECTION_TYPE_CLIENT); 	\
			lpush(p);																		\
		} 																					\
	} while(0)

#define lshink(l) { free_handler_set_t(*l).swap(*l); }

#define HANDLER(p) (SocketHandler*)((uintptr_t)(p) & (uintptr_t)~1)
#define INSTANCE(p) (uintptr_t)(p) & 1

Net::Net(unsigned int _nums)
	:m_listen_fd(0),
	m_Handlers(handler_map_t(0)),
	m_FreeHandlers(free_handler_set_t(0)),
	m_running(false),
	m_epoll_fd(0),
	m_epev_arr(0)
{
	lincream();	
}

Net::~Net() 
{
	if (m_epev_arr) free(m_epev_arr);
}

bool 
Net::init()
{
	struct rlimit rl;
	int nfiles = 65535;

	if (getrlimit(RLIMIT_NOFILE, &rl) == 0 && rl.rlim_cur != RLIM_INFINITY) {
		nfiles = rl.rlim_cur - 1;
	}
	
	if (-1 == (m_epoll_fd = epoll_create(nfiles))) {
		return false;
	}
	
	m_epev_arr = (struct epoll_event*)malloc(EVENT_TOTAL_COUNT * sizeof(struct epoll_event));

	if (!m_epev_arr) return false;

	return true;
}

bool
Net::start_server()
{
	m_running = true;

	return _start();
}

bool 
Net::_start()
{
    static int 			check_times = 0;
    SocketHandler		*s;
    lgs_event_t 		*rev, *wev;
    unsigned int 		instance;
    time_scale_t		*sc;
    time_scale_itr_t 	itr;

    tw_create(&g_tw, TIME_WHEEL_SIZE);

	while (m_running) {
		int res = epoll_wait(m_epoll_fd, m_epev_arr, EVENT_TOTAL_COUNT, 100);

		if (res == -1) {
			if (EINTR == errno) continue;
			
			TRACE("epoll_wait return false, errno = %d\n", errno);
			break;
		}
		
		for (int i = 0; i < res; i++) {
			if (m_epev_arr[i].data.fd == m_listen_fd) {
				handle_accept();
            	continue;
			}
		
			s = HANDLER(m_epev_arr[i].data.ptr);
			instance = INSTANCE(m_epev_arr[i].data.ptr);

			rev = s->readev();
			wev = s->writeev();

			//log_debug("Handler: %p", s);
			
			if (s->fd() == -1 || instance != rev->instance) {
				log_error("stale event.");
				continue;
			}

			//if (m_epev_arr[i].events & (EPOLLHUP | EPOLLERR)) { /* epoll_wait 出现错误 */
			//	//log_error("EPOLLHUP | EPOLLERR");
			//	//handle_close(s);
			//	//continue;	
			//}
			
            if (m_epev_arr[i].events & EPOLLIN && rev->active) {
				if (s->handle_read() == -1) {
					handle_close(s);
				}
			}

			if (m_epev_arr[i].events & EPOLLOUT && wev->active) {
				if (s->handle_output() == -1) {
					handle_close(s);
				}
			}
		}

        if (check_times++ >= CHECK_POINT_TIMES) {
			now = time(NULL);
            run_timer();
            check_times = 0;

			sc = tw_wheeling(g_tw, TIME_WHEEL_SIZE); /* 返回到期的连接集合 */
			for (itr = sc->begin(); itr != sc->end();) {
				SocketHandler* p = *itr;

				int live = p->GetLiveTime();
				
				int val = now - live; 

				if (live == 0 && p->fd() == -1) {
					sc->erase(itr++);
					log_debug("closed: %p", p);
				} else if (val >= 0 && val <= TIME_WHEEL_SIZE) {
					itr++;
				} else {
					sc->erase(itr++);
					log_debug("expire[%d]: %d ---> %p", p->fd(), val, p);
					handle_close(p);
				}
			}
		
			_log_stat(); /* 记录服务器状态 */
       	}
	}

	int ret = 0;
	call_lua("handle_fini", ">d", &ret);

	return true;
}

void 
Net::stop_server()
{
	m_running = false;
}

int 
Net::handle_accept()
{
	int conn_fd;
	int ret;
	
	while(1) {
		if ((conn_fd = accept(m_listen_fd, NULL, NULL)) == -1) {
			return -1;
		}

		if (__set_non_block(conn_fd) < 0) {
			log_error("set nonblock fd error. %d", errno);
		}

		// if (_set_linger_socket(conn_fd) < 0) {
		// return -1;
	// }
		
		SocketHandler* s = _new_handler(conn_fd);
		if (s == NULL) {
			log_debug("_new_handler error.");
			::close(conn_fd);
			continue;
		}
		
		__add_event(m_epoll_fd, s);

		tw_insert(g_tw, TIME_WHEEL_SIZE, s);

		if (call_lua("handle_accept", "d>d", conn_fd, &ret) == -1) {
			log_error("call lua->handle_accpet error.");
			continue;
		}			
	}

    return ret;
}

void 
Net::handle_close(SocketHandler* s)
{
	if (s == NULL) return;

	int fd = s->fd();

	merase(fd);

	s->handle_close(); // call lua->handle_client_socket_close or -> handle_server_socket_close

	FreeHandler(s, true);

	lpush(s);
}

SocketHandler* 
Net::_new_handler(SOCKET sock_fd, 
				  bool parse_protocal, 
				  bool encrypt, 
				  int conn_type) 
{
	SocketHandler* sh;

	if (m_FreeHandlers.empty()) lincream(); /* 如果没有空闲的handler就增加 */

	lpop(sh);

	if (sh) {
		sh->assign(sock_fd, parse_protocal, encrypt, conn_type);

		handler_map_itr_t it = m_Handlers.find(sock_fd);
		if (it != m_Handlers.end()) {
			FreeHandler(it->second, false); //clean ���ﲻ��ʹ��true����������Ӳ��ܼ������Լ��ر��Լ�
			lpush(it->second);			
		}
		
		minsert(sock_fd, sh);
	}

	return sh;
}

int
Net::epoll()
{
	return m_epoll_fd;
}

int 
Net::connect_server(const char* ip, const int port, bool encrypt, int conn_flag)
{
	int fd = socket(AF_INET , SOCK_STREAM , 0);
	if ( 0 > fd) return -1;

	struct sockaddr_in remote;
	memset(&remote, 0, sizeof(remote));
	remote.sin_family 		 = AF_INET;
	remote.sin_port   		 = htons(port);
	remote.sin_addr.s_addr = inet_addr(ip);

	if (0 != connect(fd, (struct sockaddr*)&remote, sizeof(remote))) {
		log_error("Error: Connect Faile connect(): %s\n", strerror(errno));
		::close(fd); /* 连接失败关闭出现的socket */
		return -1;
	}

	log_error("connect host[%s:%d] success!\n", ip, port);

	SocketHandler* s = _new_handler(fd, true, encrypt, conn_flag);
	
	if (s) {
		__add_event(m_epoll_fd, s);
		
		return fd;
	}
	
	::close(fd); 

	return -1;
}

int 
Net::create_listener(int port)
{
	m_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_listen_fd == INVALID_SOCKET) return -1;

	int nreuse = 1;
	setsockopt(m_listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&nreuse, sizeof(nreuse));	

	__set_non_block(m_listen_fd);

	sockaddr_in addr_in;
	addr_in.sin_family = AF_INET;
	addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_in.sin_port = htons(port);

	if (bind(m_listen_fd, (sockaddr*) &addr_in, sizeof(addr_in)) == SOCKET_ERROR) {
		return -1;
	}

	if (listen(m_listen_fd, 256) == SOCKET_ERROR) {
		return -1;
	}

//	__add_event(m_epoll_fd, m_listen_fd);
	
	struct epoll_event ev;
	ev.data.fd = m_listen_fd;
	ev.events = EPOLLIN | EPOLLET;

	epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_listen_fd, &ev);

	log_error("Server start listening, listen port:%d\n", port);
	return 0;
}

void 
Net::_print_all_handler() 
{
	handler_map_itr_t it;

	log_debug("HanlderSize: %zu\n", m_Handlers.size());
	
	for (it = m_Handlers.begin(); it != m_Handlers.end(); it++) {
		log_debug("m_Handlers[%d]: %p\n", it->first, it->second);
	}
}

void
Net::CloseHandler(const int& fd) 
{
	handler_map_itr_t it = m_Handlers.find(fd);

	SocketHandler* p = it == m_Handlers.end() ? NULL : it->second;

	if (p) {
		merase(fd);
		FreeHandler(p, true);
		lpush(p);
	}
}



