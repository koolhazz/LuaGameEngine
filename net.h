#ifndef __NET_H_
#define __NET_H_

#include "socket.h"

#include <sys/resource.h>
#include <sys/times.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <set>
#include <errno.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define	__INIT_NET_ENVIR__

using namespace std;

typedef struct lge_server_stat_s lge_server_stat_t;
struct lge_server_stat_s {
	unsigned int cnts; /* 当前handler的数量 */
	unsigned int free_cnts; /* 空闲handler的数量 */
};

class SocketHandler;

class Net {
public:
	Net(unsigned int _nums);
	~Net();

public:
	bool init();
	bool start_server();
	void stop_server();

	int  handle_accept();

    int  epoll();
	int  connect_server(const char* ip, 
						const int port, 
						bool encrypt, 
						int conn_flag);
						
	int  create_listener(int port);

	size_t Handlers() { return m_Handlers.size(); }
	size_t Frees() { return m_FreeHandlers.size(); }
	
	void CloseHandler(const int& fd);

	typedef tr1::unordered_map<int, SocketHandler*> handler_map_t; /* handler 集合 */
	typedef handler_map_t::iterator					handler_map_itr_t; 
	typedef tr1::unordered_set<SocketHandler*>		free_handler_set_t; /* 空闲handler集合 */

private:
	bool _start();

	void _print_all_handler();

	SocketHandler* 
	_new_handler(SOCKET sock_fd, 
				bool parse_protocal = true, 
				bool encrypt = true, 
				int conn_type = 0);

	/* stat */
	inline void _stat_hdr_inc() { _server_stat.cnts++; }
	inline void _stat_hdr_sub() { _server_stat.cnts--; }

	inline void _stat_hdr_free_inc() { _server_stat.free_cnts++; }
	inline void _stat_hdr_free_sub() { _server_stat.free_cnts--; }

	inline void _log_stat() 
	{ 
		log_debug("[cnts]: %u\t [free]: %u", _server_stat.cnts, _server_stat.free_cnts); 
	}

	void handle_close(SocketHandler* sh);
private:
	int							m_listen_fd;
	handler_map_t 				m_Handlers; // <fd, SocketHandler*>
	free_handler_set_t 			m_FreeHandlers;
	bool    					m_running;
	int  						m_epoll_fd;
	struct epoll_event			*m_epev_arr; //epoll_event����
	lge_server_stat_t 			_server_stat; //服务器的状态
};

#endif


