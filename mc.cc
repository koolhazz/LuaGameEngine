#include "mc.h"

mc_handler_t::mc_handler_t(const string& host)
	:host_(host)
{
#if 0
	memcached_server_st* server_ = memcached_servers_parse(host_.c_str());
	memc_ = memcached_create(NULL);
	memcached_server_push(memc_, server_);
	memcached_server_list_free(server_);
#endif
	memc_ = memcached(host_.c_str(), host_.size());
}

mc_handler_t::~mc_handler_t()
{
	memcached_free(memc_);
}

string
mc_handler_t::Get(string& key)
{
	size_t vlen;
	uint32_t flags;
	memcached_return rc;
	
	char* result = memcached_get(memc_, key.c_str(), key.size(), &vlen, &flags, &rc);

	string value = result ? result : "";

	free(result);

	if (rc == MEMCACHED_SUCCESS) {
		return value;//string(result);
	} else {
		return string("");
	}
}

int
mc_handler_t::Set(const string & key,const string & value)
{
	memcached_return rc;

	rc = memcached_set(memc_, key.c_str(), key.size(), value.c_str(), value.size(), 0, 0);

	if (rc == MEMCACHED_SUCCESS) {
		return 0;
	} else {
		return -1;
	}
}


