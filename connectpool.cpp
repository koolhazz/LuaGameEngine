
#include "connectpool.h"

// linux 等系统中请加入 -lmysql -I/usr/local/mysql/inlucde
#ifdef WIN32
#pragma comment(lib, "libmysql.lib")
#endif        


//CConnPool * CConnPool::m_instance = NULL;

CSql_error::CSql_error():std::runtime_error("Failed query"),m_err()
{
	//
}

CSql_error::CSql_error(const std::string &whatarg)
:std::runtime_error(whatarg),m_err()
{
	//
}

CSql_error::CSql_error(const std::string &whatarg, const std::string &err)
:std::runtime_error(whatarg),m_err(err)
{
	//
}

CSql_error::~CSql_error() throw ()
{
	//
}

const char * CSql_error::what() const throw ()
{
	return m_err.c_str();
}

//构造对象
CConnect::CConnect()
{
	//
}

//析构对象
CConnect::~CConnect()
{
	//
}

//构造对象
CMysqlConnect::CMysqlConnect():m_conn(NULL)
{
	m_err = "";
	//
}

//析构对象
CMysqlConnect::~CMysqlConnect()
{
	if(m_conn)
	{
		Disconnect();
	}
}

//连接指定的数据库
bool CMysqlConnect::Connect(const std::string &host,
							const std::string &user,
							const std::string &password,
							const std::string &dbname,
							unsigned int port, 
							const std::string &unix_socket,
							const std::string &character)
{
	//初始化数据结构
	if(NULL == (m_conn = mysql_init(m_conn)))
	{
		return false;
	}
	int ret = mysql_options(m_conn, MYSQL_OPT_RECONNECT, "1");
	if(NULL == mysql_real_connect(m_conn,host.c_str(),
		user.c_str(),password.c_str(),dbname.c_str(),port,unix_socket.c_str(),
		CLIENT_MULTI_RESULTS|CLIENT_MULTI_STATEMENTS))
	{
		throw CSql_error(mysql_error(m_conn));
		return false ;
	}
	mysql_set_character_set(m_conn, character.c_str());
	return true;
}

//中断连接
bool CMysqlConnect::Disconnect()
{
	if(m_conn)
	{
		mysql_close(m_conn);
		m_conn = NULL;
	}
	return true;
}

//得到连接
void * CMysqlConnect::GetConnect()
{
	if(m_conn == NULL)
	{
		return NULL;
	}

	//自动重新连接
	if(mysql_ping(m_conn) != 0) //还是失败 要清空
	{
		m_err = mysql_error(m_conn);
		
		return NULL;
	}

	return m_conn;
}

//得到连接的语法
const std::string CMysqlConnect::GetConnectSyntax()
{
	return "";
}

CDataStore::CDataStore()
{
	//
}

CDataStore::~CDataStore()
{
	//
}

CMysqlStore::typeset_t::typeset_t()
{
	name = "";
	type = (filedtype_t)6;
	length = 0;
	index = 0;
}

CMysqlStore::CMysqlStore():m_connptr(NULL),m_resultptr(NULL),
m_row(0),m_err(""),m_increaseID(0),
m_autocommit(true),m_getstatus(false)
{
	//
}

CMysqlStore::~CMysqlStore()
{
	Clear();
}

void CMysqlStore::Clear()
{
	m_recordset.clear();
	m_fieldtype.clear();
	m_row = 0;
	SetIncreaseID(0);
}

//设置连接对象
bool CMysqlStore::SetTransAction(CConnect * conn)
{
	if(conn == 0)
	{
		m_err = "conn == 0";
		return false;
	}

	m_connptr = (MYSQL * )conn->GetConnect();
	if(m_connptr == NULL)
	{
		//CConnPool::Instance().Erase(conn); //此连接失效
		m_err = "m_connptr == 0";
		m_err += conn->What();
		return false;
	}

	mysql_autocommit(m_connptr,m_autocommit);
	return true;
}

//得到当前执行状态
bool CMysqlStore::GetStatus()
{
	return m_getstatus;
}

//错误原因
const std::string CMysqlStore::What()
{
	return m_err;
}

CMysqlStore::filedtype_t CMysqlStore::SetFieldType(enum_field_types fieldtype)
{
	filedtype_t type;
	switch(fieldtype)
	{
	case MYSQL_TYPE_STRING:
		//
	case MYSQL_TYPE_VAR_STRING:
		//
	//case MYSQL_TYPE_TEXT:
		//
	case MYSQL_TYPE_BLOB:
		//
	case MYSQL_TYPE_SET:
		//
	case MYSQL_TYPE_GEOMETRY:
		//
	case MYSQL_TYPE_NULL:
		type = CHAR;
		break;
	case MYSQL_TYPE_TINY:
		//
	case MYSQL_TYPE_SHORT:
		//
	case MYSQL_TYPE_LONG:
		//
	case MYSQL_TYPE_INT24:
		//
	case MYSQL_TYPE_BIT:
		//
	case MYSQL_TYPE_ENUM:
		//
	case MYSQL_TYPE_YEAR:
	case MYSQL_TYPE_LONGLONG:
		type = INT;
		break;
	case MYSQL_TYPE_DECIMAL:
		//
	case MYSQL_TYPE_NEWDECIMAL:
		type = DEC;
		break;
	case MYSQL_TYPE_FLOAT:
		//
	case MYSQL_TYPE_DOUBLE:
		type = DOUBLE;
		break;
	case MYSQL_TYPE_TIMESTAMP:
		//
	case MYSQL_TYPE_DATE:
		//
	case MYSQL_TYPE_TIME:
		type = DATETIME;
		break;
	default:
		type = UNKNOWN;
		break;
	}
	return type;
}


//事务回滚
bool CMysqlStore::RollBack()
{
	if(m_autocommit)
	{
		return true;
	}

	if(mysql_rollback(m_connptr) == 0)
	{
		return true;
	}

	return false;
}

//事务提交
bool CMysqlStore::Commit()
{
	if(m_autocommit)
	{
		return true;
	}

	if(mysql_commit(m_connptr) == 0)
	{
		return true;
	}

	return false;
}

//执行数据定义语言(DDL)类语句
bool CMysqlStore::Exec(const std::string &ddl)
{
	//清除缓冲
	Clear();

	if(0 == mysql_query(m_connptr,ddl.c_str()))
	{
		//得到受影响的行数
		do{
			m_row +=(unsigned long)mysql_affected_rows(m_connptr);
			//m_increaseID =(long)mysql_insert_id(m_connptr);
		}
		while(!mysql_next_result(m_connptr));
	}
	else
	{
		m_err = mysql_error(m_connptr);
		return false;
	}

	return true;
}

void CMysqlStore::SetIncreaseID(long id)
{
	m_increaseID = id;
}

//得到新自增的序列号
long CMysqlStore::GetIncreaseID()
{
	return m_increaseID;
}

CMysqlStore::row_t * CMysqlStore::FindRow(unsigned long findrow)
{
	if(findrow >= this->RowCount() || m_recordset.size() == 0)
	{
		return NULL;
	}

	return &m_recordset[findrow];
}

//得到查询记录数
unsigned long CMysqlStore::RowCount()
{
	return m_row;
}


unsigned int CMysqlStore::GetFieldIndex(const std::string &fieldname)
{
	unsigned int index = 10000;
	for(unsigned int i = 0; i < m_fieldtype.size(); ++i)
	{
		if(0 == strcmp(m_fieldtype[i].name.c_str(),
			fieldname.c_str()))
		{
			index = m_fieldtype[i].index;
			break;
		}
	}
	return index;
}


const std::string CMysqlStore::GetItemValue(unsigned long row,
											unsigned int index)
{
	if(index >= m_fieldtype.size())
	{
		m_err = "column index upper bound";
		//得到当前执行状态
		m_getstatus = false;
		return "";
	}

	row_t * rowvalue = FindRow(row);
	if(rowvalue ==  NULL)
	{
		m_err = "row index upper bound";
		m_getstatus = false;
		return "";
	}

	return (*rowvalue)[index];
}

const std::string CMysqlStore::GetItemValue(unsigned long row,
											const std::string &fieldname)
{
	int index = 10000;
	if((index = GetFieldIndex(fieldname)) >= 10000)
	{
		m_err = "column index upper bound";
		return "";
	}

	row_t * rowvalue = FindRow(row);
	if(rowvalue ==  NULL)
	{
		m_err = "row index upper bound";
		m_getstatus = false;
		return "";
	}

	return (*rowvalue)[index];
}

//得到指定行某个字段的字符串类型值
const std::string CMysqlStore::GetItemString(unsigned long row,
											 unsigned int index)
{
	return GetItemValue(row,index);
}

const std::string CMysqlStore::GetItemString(unsigned long row,
											 const std::string &fieldname)
{
	return GetItemValue(row,fieldname);
}

//得到指定行某个字段的数值
float CMysqlStore::GetItemFloat(unsigned long row,
								const unsigned int index)
{
	return (float)atof(GetItemValue(row,index).c_str());
}


float CMysqlStore::GetItemFloat(unsigned long row,
								const std::string &fieldname)
{
	return (float)atof(GetItemValue(row,fieldname).c_str());
}
 

//得到指定行某个字段的整数值
long CMysqlStore::GetItemLong(unsigned long row,
							  const unsigned int index)
{
	return atol(GetItemValue(row,index).c_str());
}

long CMysqlStore::GetItemLong(unsigned long row,
							  const std::string &fieldname)
{
	return atol(GetItemValue(row,fieldname).c_str());
}


//执行数据操作语言(DML)类语句
bool CMysqlStore::Query(const std::string &dml)
{
	//清除缓冲
	Clear();

	if(mysql_query(m_connptr,dml.c_str()) != 0)
	//if(mysql_real_query(m_connptr,dml.c_str(), dml.length()) != 0)
	{
		m_err = mysql_error(m_connptr);
		return false;
	}
	do
	{
		m_resultptr = mysql_store_result(m_connptr);
		if(m_resultptr == NULL)
			continue;
		//得到查询返回的行数
		m_row += (unsigned long)mysql_affected_rows(m_connptr);

		//指向  mysql 的查询字段集
		MYSQL_FIELD *fieldptr = NULL;

		//取得各字段名和类型
		while(fieldptr = mysql_fetch_field(m_resultptr))
		{
			typeset_t typeset;
			typeset.index = (unsigned int)m_fieldtype.size();
			typeset.length = fieldptr->length;
			typeset.name = fieldptr->name;
			typeset.type = SetFieldType(fieldptr->type);
			m_fieldtype.push_back(typeset);
		}

		MYSQL_ROW currrow = NULL;
		while((currrow = mysql_fetch_row(m_resultptr)))
		{
			//读行的记录
			const unsigned int colcount = mysql_num_fields(m_resultptr);
			m_colcount = colcount;
            row_t rows(colcount);
			for(unsigned int i = 0; i < colcount; ++i)
			{
				rows[i] = currrow[i] ? currrow[i] : "NULL";
			}
			m_recordset.push_back(rows);
		}
		mysql_free_result(m_resultptr); 
		m_resultptr = NULL;

	}
	while(!mysql_next_result(m_connptr));

	return true;
}

