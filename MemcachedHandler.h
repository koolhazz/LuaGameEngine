/*
	Desc: 	·â×°libmemcached²Ù×÷Àà
	Author:	Austin 
*/

#ifndef __MEMCACHEDHANDLER_H_
#define __MEMCACHEDHANDLER_H_

#include <string>
#include <libmemcached/memcached.h>

using namespace std;

class CMemcachedHandler 
{
public:
	CMemcachedHandler(const string& host);
	~CMemcachedHandler();
	string Get(string& key);
	int Set(const string& key, const string& value);
private:
	memcached_st* memc_;
	string host_;
};

#endif
