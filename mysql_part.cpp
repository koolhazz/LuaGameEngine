#include "mysql_part.h"
#include "log.h"
#include "interface_c.h"

extern "C"
{
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}


extern lua_State* L;

CMysql::CMysql()
{
}

CMysql::~CMysql()
{
}

int 
CMysql::connect_mysql(const char* host, 
					  const char* user, 
					  const char* password, 
					  const char* dbname, 
					  unsigned int port)
{
    mysql_conn = new CMysqlConnect(); 
    if(!mysql_conn->Connect(host, user, password, dbname, port, "", "utf8")) {
        log_error("connect mysql failed\n");
        return -1;
    }

    mysql_store = new CMysqlStore();
	mysql_store->SetTransAction(mysql_conn);

    return 0;
}

int 
CMysql::query(const char* sql)
{
	if(sql == "") return 0;
	
	if (mysql_conn->GetConnect() == NULL) {
		m_strError = mysql_conn->What();
        log_error("mysql query error, reason:%s", m_strError.c_str());
		return -1;
	}
	
    if(mysql_store->Query(sql))
	{
        unsigned long count = mysql_store->RowCount(); 
        if (count <= 0) {
            return 0;
        }
        
        lua_newtable(L);
        for(unsigned int i=0; i<count; i++)
        {
            int colcount = mysql_store->GetColCount();
            lua_pushinteger(L, i+1);
            lua_newtable(L);
            
            for(int j=0; j<colcount;j++)
            {
                string val = mysql_store->GetItemString(i, j);
                lua_pushinteger(L, j+1);
                lua_pushstring(L, val.c_str());
                lua_rawset(L, -3); 
            }           
            lua_rawset(L, -3); 
        }

        lua_setglobal(L, MYSQL_RESULT_SET);        
		return 0;
	}
	m_strError = mysql_store->What();
    
    log_error("mysql query error, reason:%s", m_strError.c_str());
	return -1;
}

