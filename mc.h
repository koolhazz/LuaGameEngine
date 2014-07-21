/*
	Desc: 	·â×°libmemcached²Ù×÷Àà
	Author:	Austin 
*/

#ifndef _MC_HANDLER_H_
#define _MC_HANDLER_H_

#include <string>
#include <libmemcached/memcached.h>

using std::string;

class mc_handler_t {
public:
	mc_handler_t(const string& host);
	~mc_handler_t();
	string Get(string& key);
	int Set(const string& key, const string& value);
private:
	memcached_st* 	memc_;
	string 			host_;
};

#endif
