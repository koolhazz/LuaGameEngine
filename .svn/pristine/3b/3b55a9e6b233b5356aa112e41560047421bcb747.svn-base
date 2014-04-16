#pragma once

#include "PacketParser.h"
#include "log.h"
#include <assert.h>
#include <string>
#include <netinet/in.h>
using namespace std;

typedef unsigned char       BYTE;

#ifdef TCP_BUFFER_SIZE
#define	ICHAT_TCP_DEFAULT_BUFFER	TCP_BUFFER_SIZE//8192
#else
	#define	ICHAT_TCP_DEFAULT_BUFFER	8192
#endif

#define HEADER_SIZE_14		14
#define HEADER_SIZE_9		9

template <int _buffer_size>
class PacketBase
{
public:
	PacketBase(void){}
	virtual ~PacketBase(void){}

	char *packet_buf(void)	{return m_strBuf;}
	int packet_size(void)	{return m_nPacketSize;}
	enum
	{
#if HEADER_SIZE == 9
#warning("header_size == 9")
		PACKET_HEADER_SIZE = 9,
#else 
#warning("header_size == 14")
		PACKET_HEADER_SIZE = 14,
#endif
		PACKET_BUFFER_SIZE = _buffer_size
	};
private:
	char m_strBuf[PACKET_BUFFER_SIZE];	// 报文包缓存
	int m_nPacketSize ;	// 实际报文总长度
	int m_nBufPos;

protected:
	////////////////////////////////////////////////////////////////////////////////
	bool _copy(const void *pInBuf, int nLen)
	{
		if(nLen > PACKET_BUFFER_SIZE) {
			return false;
		}

		_reset();
		memcpy(m_strBuf, pInBuf, nLen);
		m_nPacketSize = nLen;
		assert(m_nPacketSize>PACKET_HEADER_SIZE);
		return true;
	}
	////////////////////////////////////////////////////////////////////////////////
	void _begin(short nCmdType, char cVersion, char cSubVersion)
	{
		_reset();
		short cmd = htons(nCmdType);
		_writeHeader("BY", sizeof(char)*2, 2);			// magic word
		_writeHeader(&cVersion, sizeof(char), 4);		// 主版本号
		_writeHeader(&cSubVersion, sizeof(char), 5);	// 子版本号
		_writeHeader((char*)&cmd, sizeof(short), 6);	// 命令码
	}

    void _begin(short nCmdType, char cVersion, char cSubVersion, short nSubCmd, short nSeq, char cSourceType)
	{
		_reset();
		short cmd = htons(nCmdType);
		_writeHeader("BY", sizeof(char)*2, 2);			// magic word
		_writeHeader(&cVersion, sizeof(char), 4);		// 主版本号
		_writeHeader(&cSubVersion, sizeof(char), 5);	// 子版本号
		_writeHeader((char*)&cmd, sizeof(short), 6);	// 命令码
        _writeHeader((char*)&cmd, sizeof(short), 9);	// 子命令码
        _writeHeader((char*)&cmd, sizeof(short), 11);	// 命令序列号
        _writeHeader(&cSourceType, sizeof(char), 12);	// 消息来源
	}

	void _SetBegin(short nCmdType)
	{
		short cmd = htons(nCmdType);
		_writeHeader((char*)&cmd, sizeof(short), 6);// 命令码
	}
public:
	short GetCmdType(void)
	{
		short nCmdType;
		_readHeader((char*)&nCmdType, sizeof(short), 6);// 命令码
		short cmd = ntohs(nCmdType);
		return cmd;
	}
	char GetVersion(void)
	{
		char c;
		_readHeader(&c, sizeof(char), 4);	// 主版本号
		return c;
	}
	char GetSubVersion(void)
	{
		char c;
		_readHeader(&c, sizeof(char), 5);	// 子版本号
		return c;
	}
	short GetBodyLength(void)
	{
		short nLen;
		_readHeader((char*)&nLen, sizeof(short), 0);// 包正文长度
		short len = ntohs(nLen);
		return len;
	}

	BYTE GetcbCheckCode(void)
	{
		BYTE code;
		_readHeader((char*)&code, sizeof(BYTE), 8);// cb code
		return code;
	}

    short GetSubCmdType(void)
	{
		short nCmdType;
		_readHeader((char*)&nCmdType, sizeof(short), 9);// 命令码
		short cmd = ntohs(nCmdType);
		return cmd;
	}

    short GetSequence(void)
	{
		short nSequence;
		_readHeader((char*)&nSequence, sizeof(short), 11);// 命令码
		short seq = ntohs(nSequence);
		return seq;
	}

    BYTE GetcbSourceType(void)
	{
		BYTE sourceType;
		_readHeader((char*)&sourceType, sizeof(BYTE), 12);// cb code
		return sourceType;
	}

protected:
	void _end()
	{
		short nBody = m_nPacketSize - 2;	//数据包长度包括命令头和body,2个字节是数据包长度
		short len = htons(nBody);
		_writeHeader((char*)&len, sizeof(short), 0);	// 包正文长度
		BYTE code = 0;
		_writeHeader((char*)&code, sizeof(BYTE), 8);	//效验码
	}
	void _oldend()
	{
		short nBody = m_nPacketSize - 2;
		short len = ntohs(nBody);
		_writeHeader((char*)&len, sizeof(short), 0);	// 包正文长度
	}
	/////////////////////////////////////////////////////////////////////////////////
	void _reset(void)
	{
		memset(m_strBuf, 0, PACKET_BUFFER_SIZE);
		m_nBufPos = PACKET_HEADER_SIZE;
		m_nPacketSize = PACKET_HEADER_SIZE;
	}
	// 取出一个变量
	bool _Read(char *pOut, int nLen)
	{
		if((nLen + m_nBufPos) > m_nPacketSize || (nLen + m_nBufPos) > PACKET_BUFFER_SIZE)
			return false ;

		memcpy(pOut, m_strBuf + m_nBufPos, nLen);
		m_nBufPos += nLen;
		return true;
	}
	//取出变量并从包中移除
	bool _ReadDel(char *pOut, int nLen)
	{
		if(!_Read(pOut, nLen))
			return false;
		memcpy(m_strBuf + m_nBufPos - nLen, m_strBuf + m_nBufPos, PACKET_BUFFER_SIZE - m_nBufPos);
		m_nBufPos -= nLen;
		m_nPacketSize -= nLen;
		_end();
		return true;
	}
	//读撤消
	void _readundo(int nLen)
	{
		m_nBufPos -= nLen;
	}
	//读出当前POS位置的BUFFER指针
	char *_readpoint(int nLen) //注意返回的是指针 请慎重使用string
	{
		if((nLen + m_nBufPos) > m_nPacketSize)
			return NULL; 
		char *p = &m_strBuf[m_nBufPos];
		m_nBufPos += nLen;
		return p;

	}
	// 写入一个变量
	bool _Write(const char *pIn, int nLen)
	{
		if((m_nPacketSize < 0) || ((nLen + m_nPacketSize) > PACKET_BUFFER_SIZE))
			return false ;
		memcpy(m_strBuf+m_nPacketSize, pIn, nLen);
		m_nPacketSize += nLen;
		return true;
	}
	//插入一个变量
	bool _Insert(const char *pIn, int nLen)
	{
		if((nLen + m_nPacketSize) > PACKET_BUFFER_SIZE)
			return false;
		memcpy(m_strBuf+PACKET_HEADER_SIZE+nLen, m_strBuf+PACKET_HEADER_SIZE, m_nPacketSize-PACKET_HEADER_SIZE);
		memcpy(m_strBuf+PACKET_HEADER_SIZE, pIn, nLen);
		m_nPacketSize += nLen;
		_end();
		return true;
	}
	// 写入一个变量
	bool _writezero(void)
	{
		if((m_nPacketSize + 1) > PACKET_BUFFER_SIZE)
			return false ;
		memset(m_strBuf+m_nPacketSize, '\0', sizeof(char)) ;
		m_nPacketSize ++;
		return true;
	}
	// readHeader
	void _readHeader(char *pOut, int nLen, int nPos)
	{
		if(nPos > 0 || nPos+nLen <= PACKET_HEADER_SIZE)
		{
			memcpy(pOut, m_strBuf+nPos, nLen) ;
		}
	}
	// writeHeader
	void _writeHeader(char *pIn, int nLen, int nPos)
	{
		if(nPos > 0 || nPos+nLen < PACKET_HEADER_SIZE)
		{
			memcpy(m_strBuf+nPos, pIn, nLen) ;
		}
	}
};

template <int _buffer_size>
class InputPacket: public PacketBase<_buffer_size>
{
public:
	typedef PacketBase<_buffer_size> base;

	int ReadInt(void)		{int nValue = -1; base::_Read((char*)&nValue, sizeof(int)); return ntohl(nValue);} //这里必需初始化
	unsigned long ReadULong(void) {unsigned long nValue = -1; base::_Read((char*)&nValue, sizeof(unsigned long)); return ntohl(nValue);}
	int ReadIntDel(void)	{int nValue = -1; base::_ReadDel((char*)&nValue, sizeof(int)); return ntohl(nValue);} 
	short ReadShort(void)	{short nValue = -1; base::_Read((char*)&nValue, sizeof(short)); return ntohs(nValue);}
	BYTE ReadByte(void)		{BYTE nValue = -1; base::_Read((char*)&nValue, sizeof(BYTE)); return nValue;}

	bool ReadString(char *pOutString, int nMaxLen)
	{
		int nLen = ReadInt();

		if(nLen == -1) {
			return false;
		}

		if(nLen > nMaxLen) {
			base::_readundo(sizeof(short));
			return false;
		}

		return base::_Read(pOutString, nLen);
	}

	char *ReadChar(void)
	{
		int nLen = ReadInt();

		if(nLen == -1) {
			return NULL;
		}

		return base::_readpoint(nLen);
	}

	string ReadString(void)
	{
		char *p = ReadChar();
		return p == NULL ? "" : p;
	}

	int ReadBinary(char *pBuf, int nMaxLen)
	{
		int nLen = ReadInt();

		if(nLen == -1) {
			return -1;
		}

		if(nLen > nMaxLen) {
			base::_readundo(sizeof(int));
			return -1;
		}

		if(base::_Read(pBuf, nLen)) return nLen ;

		return 0;
	}

	void Reset(void) { base::_reset(); }

	bool Copy(const void *pInBuf, int nLen) { return base::_copy(pInBuf, nLen); }

	bool WriteBody(const char *pIn, int nLen) { return base::_Write(pIn, nLen); }
	
	void Begin(short nCommand, 
			   char cVersion = SERVER_PACKET_DEFAULT_VER, 
			   char cSubVersion = SERVER_PACKET_DEFAULT_SUBVER, 
			   short nSubCmd = SERVER_PACKET_DEFAULT_SUBCMD, 
			   short nSeq = SERVER_PACKET_DEFAULT_SEQ, 
			   char cSourceType = SERVER_PACKET_DEFAULT_SOURCETYPE)
	{
        if(9 == base::PACKET_HEADER_SIZE) {
            base::_begin(nCommand, cVersion, cSubVersion);
        } else {
            base::_begin(nCommand, cVersion, cSubVersion, nSubCmd, nSeq, cSourceType);
        }
	}

	void End(void) { base::_end(); }
};

template <int BUFFER_SIZE>
class OutputPacket: public PacketBase<BUFFER_SIZE>
{
	bool m_isCheckCode;
public:
	OutputPacket(void){m_isCheckCode = false;}
public:
	typedef PacketBase<BUFFER_SIZE> base;

	bool WriteInt(int nValue)		{int value = htonl(nValue); return base::_Write((char*)&value, sizeof(int));}
	bool WriteULong(unsigned long nValue) {unsigned long value = htonl(nValue);return base::_Write((char*)&value, sizeof(unsigned long));}
	bool WriteByte(BYTE nValue)		{return base::_Write((char*)&nValue, sizeof(BYTE));}
	bool WriteShort(short nValue)	{short value = htons(nValue); return base::_Write((char*)&value, sizeof(short));}
	//在正文首插入数据
	bool InsertInt(int nValue)		{int value = htonl(nValue); return base::_Insert((char*)&value, sizeof(int));}
	bool InsertByte(BYTE nValue)	{return base::_Insert((char*)&nValue, sizeof(BYTE));}
	bool WriteString(const char *pString)
	{
		int nLen = (int)strlen(pString) ;
		WriteInt(nLen + 1) ;
		return base::_Write(pString, nLen) && base::_writezero();
	}

	bool WriteString(const string &strDate)
	{
		int nLen = (int)strDate.size();
		WriteInt(nLen + 1) ;
		return base::_Write(strDate.c_str(), nLen) && base::_writezero();
	}

	bool WriteBinary(const char *pBuf, int nLen)
	{
		WriteInt(nLen) ;
		return base::_Write(pBuf, nLen) ;
	}

	bool Copy(const void *pInBuf, int nLen)
	{
		return base::_copy(pInBuf, nLen);
	}

    void Begin(short nCommand, 
			   char cVersion = SERVER_PACKET_DEFAULT_VER, 
			   char cSubVersion = SERVER_PACKET_DEFAULT_SUBVER, 
			   short nSubCmd = SERVER_PACKET_DEFAULT_SUBCMD, 
			   short nSeq = SERVER_PACKET_DEFAULT_SEQ, 
			   char cSourceType = SERVER_PACKET_DEFAULT_SOURCETYPE)
	{
        if(9 == base::PACKET_HEADER_SIZE) {
            base::_begin(nCommand, cVersion, cSubVersion);
        } else {
            base::_begin(nCommand, cVersion, cSubVersion, nSubCmd, nSeq, cSourceType);
        }

        m_isCheckCode = false;
	}
	void End(void)
	{
		m_isCheckCode = false;
		base::_end();
	}
	void oldEnd(void)
	{
		m_isCheckCode = false;
		base::_oldend();
	}
	//增加
	void SetBegin(short nCommand)
	{
		base::_SetBegin(nCommand);
	}
	//效验码
	void WritecbCheckCode(BYTE nValue)
	{
		base::_writeHeader((char*)&nValue, sizeof(BYTE), 8); //效验码
		//m_isCheckCode = true;
	}

	bool IsWritecbCheckCode(void)
	{
		return m_isCheckCode;
	}
};

typedef InputPacket<ICHAT_TCP_DEFAULT_BUFFER>	NETInputPacket;
typedef OutputPacket<ICHAT_TCP_DEFAULT_BUFFER>	NETOutputPacket;

