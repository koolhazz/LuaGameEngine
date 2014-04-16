
#ifndef _CONNECTPOOL_H_
#define _CONNECTPOOL_H_

//#include "myosmutex.h"
#ifdef WIN32
#include <process.h>
#include <winsock2.h>
#else
#include <sys/errno.h>
#include <pthread.h>
#endif

#include <mysql.h> //文件位于 MySQL 提供的 C API 目录中
#include <mysqld_error.h>

#include <map>
#include <vector>
#include <stdexcept>
#include <string>
#include <string.h>
#include <stdlib.h>

using namespace std;

class CSql_error : public std::runtime_error
{
private:
	std::string m_err;
public:
  explicit CSql_error();
  
  explicit CSql_error(const std::string &whatarg);
 
  explicit CSql_error(const std::string &whatarg, const std::string &err);
    
  virtual ~CSql_error() throw ();
  
  virtual const char * what() const throw ();
};

class CConnect
{
private:
	//
protected:
    CConnect(const CConnect &);
    CConnect &operator=(const CConnect &);
	//错误原因
	std::string m_err;
public:
	//构造对象
	CConnect();
	
	//析构对象
	virtual ~CConnect();
	
	//连接指定的数据库
	virtual bool Connect(const std::string &host,
		                 const std::string &user,
		                 const std::string &password,
		                 const std::string &dbname,
		                 unsigned int port,
						 const std::string &unix_socket,
						 const std::string &character) = 0;

	//得到连接的语法
	virtual const std::string GetConnectSyntax() = 0;
	
	//中断连接
	virtual bool Disconnect() = 0;

	//得到连接对象
	virtual void * GetConnect()  = 0;
	//virtual MYSQL * GetConnect()  = 0;
		//错误原因
	std::string What(){return m_err;}
};


class CMysqlConnect : public CConnect
{
private:
	//一个连接
	MYSQL *m_conn;

	//MYSQL 对象不能 copy,屏蔽 拷贝构造
	CMysqlConnect(const CMysqlConnect &rhs);

	//赋值运算也被屏蔽
	CMysqlConnect & operator=(const CMysqlConnect &rhs);
	
protected:
	//
public:
	//构造对象
	CMysqlConnect();
		
	//析构对象
	virtual ~CMysqlConnect();

	//连接指定的数据库
	virtual bool Connect(const std::string &host,
		                 const std::string &user,
		                 const std::string &password,
		                 const std::string &dbname,
		                 unsigned int port,
						 const std::string &unix_socket,
						 const std::string &character);
		
	//中断连接
	virtual bool Disconnect();

	//得到连接
	void * GetConnect();

	//得到连接的语法
	virtual const std::string GetConnectSyntax();

};

class CDataStore
{
private:
	//
protected:
	//
public:

	CDataStore();
	
	virtual ~CDataStore();
	
	virtual bool SetTransAction(CConnect * conn) = 0;
	
	//执行数据定义语言(DDL)类语句
	virtual bool Exec(const std::string &ddl) = 0;

	//执行数据定义操作(DML)类语句
	virtual bool Query(const std::string &dml) = 0;

	//事务提交
	virtual bool Commit() = 0;

	//事务回滚
	virtual bool RollBack() = 0;

	//得到查询记录数
	virtual unsigned long RowCount() = 0;

	//错误原因
	virtual const std::string What() = 0;

	//得到指定行某个字段的字符串类型值
	virtual const std::string GetItemString(
		unsigned long row,
		unsigned int index) = 0;
	virtual const std::string GetItemString(
		unsigned long row,
		const std::string &fieldname) = 0;

	
	//得到指定行某个字段的数值
	virtual float GetItemFloat(unsigned long row,
		const unsigned int index) = 0;
	virtual float GetItemFloat(unsigned long row,
		const std::string &fieldname) = 0;

	//得到指定行某个字段的整数值
	virtual long GetItemLong(unsigned long row,
		const unsigned int index) = 0;
	virtual long GetItemLong(unsigned long row,
		const std::string &fieldname) = 0;
};


class CMysqlStore : public CDataStore
{
private:
	//指向  mysql 的连接指针
	MYSQL * m_connptr;

	//指向  mysql 的查询数据集
	MYSQL_RES *m_resultptr;

	//操作影响的记录数
	unsigned long m_row;
	
	//错误原因
	std::string m_err;

	//新的自增的序列号
	long m_increaseID;

	//事务提交模式
	bool m_autocommit;

	//字段索引和字段类型的对应表
	enum  filedtype_t
	{CHAR = 1,INT = 2,DATETIME = 3,DOUBLE = 4,DEC = 5,UNKNOWN = 6};
	struct typeset_t
	{
		std::string name;
		filedtype_t type;
		unsigned int length;
		//查询列表中的列位置
		unsigned int index;
		typeset_t();	
	};

	//取得信息状态
	bool m_getstatus;
	
	//字段信息表
	std::vector<typeset_t> m_fieldtype;

	typedef std::vector<std::string> row_t;
	
	std::vector<row_t> m_recordset;
    
    int m_colcount;

	//清除临时对象
	void Clear();	

	//找到行
	row_t * FindRow(unsigned long findrow);
	
	//找到对应的列序号
	unsigned int GetFieldIndex(const std::string &fieldname);

	//设置自增序列号
	void SetIncreaseID(long id);

	const std::string GetItemValue(unsigned long row,
		unsigned int index);

	const std::string GetItemValue(unsigned long row,
		const std::string &fieldname);

protected:
	//
public:

	CMysqlStore();

	virtual ~CMysqlStore();


	//设置连接对象
	bool SetTransAction(CConnect * conn);

	//得到当前执行状态
	bool GetStatus();
	
	//错误原因
	virtual const std::string What();
	
	//执行数据定义语言(DDL)类语句
	virtual bool Exec(const std::string &ddl);

	//执行数据操作语言(DML)类语句
	virtual bool Query(const std::string &dml);

	filedtype_t SetFieldType(enum_field_types fieldtype);

	//得到新自增的序列号
	long GetIncreaseID();
	
	//事务提交
	virtual bool Commit();

	//事务回滚
	virtual bool RollBack();

	//得到查询记录数
	virtual unsigned long RowCount();
		
	//得到指定行某个字段的字符串类型值
	virtual const std::string GetItemString(unsigned long row,
		unsigned int index);
	
	virtual const std::string GetItemString(unsigned long row,
		const std::string &fieldname);
		
	//得到指定行某个字段的数值
	virtual float GetItemFloat(unsigned long row,
		const unsigned int index);
		
	virtual float GetItemFloat(unsigned long row,
		const std::string &fieldname);
	
	//得到指定行某个字段的整数值
	virtual long GetItemLong(unsigned long row,
		const unsigned int index);
		
	virtual long GetItemLong(unsigned long row,
		const std::string &fieldname);

	MYSQL* GetMySqlConn()
	{
		return m_connptr;
	}

    int GetColCount()
    {
        return m_colcount;
    }
};

#endif

