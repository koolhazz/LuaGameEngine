#include "connectpool.h"

class CMysql
{
public:
    CMysql();
    ~CMysql();

public:
    int connect_mysql(const char* host, const char* user, const char* password, const char* dbname, unsigned int port);
    int query(const char* sql);
private:
    CConnect*     mysql_conn;
	CMysqlStore*  mysql_store;
    string m_strError;
};

