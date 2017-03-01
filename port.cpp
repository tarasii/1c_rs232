#include "port.h"


CPort::CPort(void)
{
	m_isOpen = false;
	m_port = 1;
	m_baud = 9600;
	m_parity = 0;
	m_stopBit = 0;
	m_err = 0;
	m_byteSize = 8;
	hComm = 0;
}

CPort::~CPort(void)
{
	hComm = 0;
}

bool CPort::Open(uint8_t portNumber, uint32_t baudRate, uint8_t byteSize, uint8_t parity, uint8_t stopBit, bool async )
{
	char	pcCommPort[32];
	DCB		dcb;
	COMMTIMEOUTS timeouts;

	ClearError();

	m_port     = portNumber;
	m_baud     = baudRate;
	m_byteSize = byteSize;
	m_parity   = parity;
	m_stopBit  = stopBit;
	m_isAsync  = async;

	sprintf(pcCommPort, "\\\\.\\COM%d", m_port);
	if (m_isAsync) {
		hComm = CreateFileA(pcCommPort, 
			GENERIC_READ | GENERIC_WRITE, 
			0, 
			NULL, 
			OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 
			NULL);
	} else {
		hComm = CreateFileA(pcCommPort, 
			GENERIC_READ | GENERIC_WRITE, 
			0, 
			NULL, 
			OPEN_EXISTING, 
			0, 
			NULL);
	}

	if (hComm == INVALID_HANDLE_VALUE)
	{
		m_err = PORT_ERR_INVALID_HANDLER;
		return false; //error opening com port
	}
	if (!GetCommState(hComm, &dcb))
	{
		Close();
		m_err = PORT_ERR_GET_STATE;
		return false;  //error geting com port settings
	}

	dcb.BaudRate	= m_baud;
	dcb.ByteSize	= m_byteSize;
	dcb.Parity		= m_parity;
	dcb.StopBits	= m_stopBit;
	//dcb.fDtrControl = DTR_CONTROL_DISABLE;

	if (m_parity != 0) dcb.fParity = true; else dcb.fParity = false;

	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	if (!SetCommState(hComm, &dcb))
	{
		Close();
		m_err = PORT_ERR_SET_STATE;
		return false; //error write com port settings 
	}

	memset(&timeouts, 0, sizeof(timeouts));
	timeouts.ReadTotalTimeoutConstant	 = ASYNC_READ_TAKT;
	timeouts.WriteTotalTimeoutConstant	 = 0;
	timeouts.ReadIntervalTimeout		 = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier	 = MAXDWORD;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	//timeouts.ReadTotalTimeoutConstant	 = 500;

	if (!SetCommTimeouts(hComm, &timeouts))
	{
		Close();
		m_err = PORT_ERR_SET_TIMEOUTS;
		return false; //error seting com port timeouts
	}

	Reset();
	m_isOpen = true;

	return true;
}

bool CPort::IsOpen()
{
	return m_isOpen;
}

uint8_t CPort::GetPortNumber()
{
	return m_port;
}

uint32_t CPort::GetBaud()
{
	return m_baud;
}

uint8_t CPort::GetByteSize()
{
	return m_byteSize;
}

uint8_t CPort::GetParity()
{
	return m_parity;
}

uint8_t CPort::GetStopBit()
{
	return m_stopBit;
}

uint8_t CPort::GetLastError()
{
	return m_err;
}


void CPort::Close()
{
	if(m_isOpen)
	{
		CloseHandle(hComm);
		m_isOpen = false;
	}
}

DWORD CPort::SendBuf(LPVOID lpBuf, DWORD dwBytesToWrite)
{
	DWORD dwBytesWritten = 0;

	if (m_isAsync){
		dwBytesWritten = SendBufAsync(lpBuf, dwBytesToWrite); 
	} else {
		dwBytesWritten = SendBufSync(lpBuf, dwBytesToWrite); 
	}
	return dwBytesWritten;
}

DWORD CPort::SendBufSync(LPVOID lpBuf, DWORD dwBytesToWrite)
{
	DWORD	dwBytesWritten = 0;
	BOOL	bStatus;

	ClearError();
	if(!m_isOpen)
	{
		m_err = PORT_NOOPEN;
		return 0;
	}

	bStatus = WriteFile(hComm, lpBuf, dwBytesToWrite, &dwBytesWritten, NULL);
	if (!bStatus)
	{
		m_err = PORT_ERR_WRITE;
	}

	if (dwBytesWritten != dwBytesToWrite)
	{
		m_err = PORT_ERR_WRITE_LEN;
	}

	return dwBytesWritten;
}

DWORD CPort::SendBufAsync(LPVOID lpBuf, DWORD dwBytesToWrite)
{
	DWORD	dwBytesWritten = 0;
	DWORD	dwError;
	BOOL	bResult;
	DWORD	dwResult;
	OVERLAPPED ov = {0};

	ClearError();
	if(!m_isOpen)
	{
		m_err = PORT_NOOPEN;
		return 0;
	}

	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);	
	if(ov.hEvent == NULL)
	{
		m_err = PORT_ERR_CREATEEVENT;
		return FALSE;
	}

	bResult = WriteFile(hComm, lpBuf, dwBytesToWrite, &dwBytesWritten, &ov);
	if (!bResult)
	{
		dwError = GetLastError();
		if(dwError != ERROR_IO_PENDING)
		{
			//ошибка записи
			m_err = PORT_ERR_ASYNCWRITESTART;
		}
		else
		{
			//ожидание завершения записи
			dwResult = WaitForSingleObject(ov.hEvent, INFINITE);
			if(dwResult == WAIT_OBJECT_0)
			{
				//проверка завершения операции записи и анализ ее результата
				if(!GetOverlappedResult(hComm, &ov, &dwBytesWritten, FALSE))
				{
					//ошибка при записи 
					m_err = PORT_ERR_ASYNCWRITE;
				}
				else
				{
					if(dwBytesToWrite != dwBytesWritten) m_err = PORT_ERR_ASYNCWRITE_LEN;

					//запись завершена успешно
				}
			}
			else
			{
				//Ошибка закрываем событие и выходим
				m_err = PORT_ERR_ASYNCWRITEPENDING;
			}
		}
	}

	CloseHandle(ov.hEvent);

	return dwBytesWritten;
}

DWORD CPort::ReadBuf(LPVOID lpBuf, DWORD dwBytesToRead)
{
	DWORD	dwBytesRead = 0;

	if (m_isAsync){
		dwBytesRead = ReadBufAsync(lpBuf, dwBytesToRead); 
	} else {
		dwBytesRead = ReadBufSync(lpBuf, dwBytesToRead); 
	}
	return dwBytesRead;
}

DWORD CPort::ReadBufSync(LPVOID lpBuf, DWORD dwBytesToRead)
{
	DWORD dwBytesRead = 0;
	BOOL  bResult;

	ClearError();
	if(!m_isOpen)
	{
		m_err = PORT_NOOPEN;
		return 0;
	}

	bResult = ReadFile(hComm, &lpBuf, dwBytesToRead, &dwBytesRead, NULL);
	if (!bResult)
	{
		m_err = PORT_ERR_REED;
	}

	return dwBytesRead;
}

DWORD CPort::ReadBufAsync(LPVOID lpBuf, DWORD dwBytesToRead)
{
	DWORD dwBytesRead = 0;
	BOOL  bResult;
	DWORD dwResult;
	DWORD dwError;
	BOOL  bReadDone = FALSE;
	OVERLAPPED ov={0};

	ClearError();
	if(!m_isOpen)
	{
		m_err = PORT_NOOPEN;
		return 0;
	}

	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);	
	if(ov.hEvent == NULL)
	{
		m_err = PORT_ERR_CREATEEVENT;
		return FALSE;
	}

	bResult = ReadFile(hComm, lpBuf, dwBytesToRead, &dwBytesRead, &ov);
	if(!bResult)
	{
		//происходит чтение 
		dwError = GetLastError();
		if(dwError != ERROR_IO_PENDING)
		{
			//ошибка старта программы асинхронного чтения
			m_err = PORT_ERR_ASYNCREADSTART;
			return 0;
			
		}

		//программа асинхронного старта успешно стартовала
		while (!bReadDone)
		{ 
			dwResult = WaitForSingleObject(ov.hEvent, ASYNC_READ_TAKT);
			switch(dwResult)
			{
				case WAIT_OBJECT_0:
					//проверка завершения операции чтения и анализ ее результата
					if(!GetOverlappedResult(hComm, &ov, &dwBytesRead, FALSE))
					{
						//ошибка при чтении 
						m_err = PORT_ERR_ASYNCREAD;
					}
					else
					{
						//чтение завершено 
						if (dwBytesRead == 0) m_err = PORT_ERR_ASYNCREADEMPTYBUF;
						else if(dwBytesRead != dwBytesToRead) m_err = PORT_TIMEOUTOVER;
					}
					bReadDone = TRUE;
					break;

				case WAIT_TIMEOUT:
					//here we can to do something
					break;
				
				default:
					//Ошибка  выходим
					m_err = PORT_ERR_ASYNCREADPENDING;
					return 0;
			}
		}
	}

	CloseHandle(ov.hEvent);

	return dwBytesRead;
}

void CPort::ClearError()
{
	m_err = PORT_NOERROR;
}

BOOL CPort::SendByte(BYTE byte)
{
	return(TransmitCommChar(hComm, byte));
}

void CPort::Reset()
{
	PurgeComm(hComm,PURGE_TXCLEAR|PURGE_RXCLEAR);
}


BOOL CPort::ClrDTR()
{
	return(EscapeCommFunction(hComm,CLRDTR));
}

BOOL CPort::ClrRTS()
{
	return(EscapeCommFunction(hComm,CLRRTS));
}

BOOL CPort::SetDTR()
{
	return(EscapeCommFunction(hComm,SETDTR));
}

BOOL CPort::SetRTS()
{
	return(EscapeCommFunction(hComm,SETRTS));
}
