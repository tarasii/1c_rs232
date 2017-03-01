#pragma once
#include "types.h"
#include <stdio.h>
#include <string>

#define PORT_READ_BUFLEN 32
#define ASYNC_READ_TAKT 50

#define PORT_NOERROR 0
#define PORT_ERR_INVALID_HANDLER 1
#define PORT_ERR_GET_STATE 2
#define PORT_ERR_SET_STATE 3
#define PORT_ERR_SET_TIMEOUTS 4
#define PORT_NOOPEN 5
#define PORT_ERR_WRITE 6
#define PORT_ERR_REED 7
#define PORT_ERR_WRITE_LEN 8
#define PORT_ERR_CREATEEVENT 9
#define PORT_ERR_ASYNCWRITESTART 10
#define PORT_ERR_ASYNCWRITE 11
#define PORT_ERR_ASYNCWRITE_LEN 12
#define PORT_ERR_ASYNCWRITEPENDING 13
#define PORT_ERR_ASYNCREADSTART 14
#define PORT_ERR_ASYNCREAD 15
#define PORT_ERR_ASYNCREADEMPTYBUF 16
#define PORT_TIMEOUTOVER 17
#define PORT_ERR_ASYNCREADPENDING 18



class CPort
{
public:
	CPort(void);
	~CPort(void);

	bool Open(uint8_t portNumber, uint32_t baudRate, uint8_t byteSize, uint8_t parity, uint8_t stopBit, bool async);
	bool IsOpen();
	uint8_t GetPortNumber();
	uint32_t GetBaud();
	uint8_t GetByteSize();	
	uint8_t GetParity();	
	uint8_t GetStopBit();	
	uint8_t GetLastError();	
	bool IsAsync();
	void Close();
	DWORD SendBuf(LPVOID lpBuf, DWORD dwNbt);
	DWORD ReadBuf(LPVOID lpBuf, DWORD dwNbt);
	void ClearError();
	BOOL SendByte(BYTE byte);
	BOOL SetRTS();
	BOOL SetDTR();
	BOOL ClrRTS();
	BOOL ClrDTR();
	void Reset(void);

private:
	
	DWORD SendBufAsync(LPVOID lpBuf, DWORD dwNbt);
	DWORD SendBufSync(LPVOID lpBuf, DWORD dwNbt);
	DWORD ReadBufAsync(LPVOID lpBuf, DWORD dwNbt);
	DWORD ReadBufSync(LPVOID lpBuf, DWORD dwNbt);

	HANDLE hComm;

	bool                m_isOpen;
	bool				m_isAsync;

	uint8_t             m_port;
	uint32_t            m_baud;
	uint8_t             m_byteSize;
	uint8_t             m_parity;
	uint8_t             m_stopBit;

	uint8_t				m_err;

	std::wstring		m_ans;
};
