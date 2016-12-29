
#include "stdafx.h"


#ifdef __linux__
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#endif

#include <stdio.h>
#include <wchar.h>
#include "AddInNative.h"
#include <string>

#define TIME_LEN 34

#define BASE_ERRNO     7

static wchar_t *g_PropNames[] = {L"IsOpen", L"Port", L"Baud", L"ByteSize", L"Parity", L"StopBit", L"Command", L"Answer", L"Error", L"Loging"};
static wchar_t *g_MethodNames[] = {L"Open", L"Close", L"Send", L"Receive", L"Delay", L"SendIKS", L"toHEX", L"StartTimer", L"StopTimer", L"SendHex", L"ReceiveHex", L"fromHEX", L"Version", L"Test", L"SetParam", L"GetParams", L"GetInfo", L"GetMore"};

static wchar_t *g_PropNamesRu[] = {L"Открыт", L"Порт", L"Скорость", L"Байт", L"Четность", L"СтопБит", L"Команда", L"Ответ", L"Ошибка", L"Логирование"};
static wchar_t *g_MethodNamesRu[] = { L"Открыть", L"Закрыть", L"Отправить", L"Получить", L"Задержка", L"ОтправитьИКС", L"в16", L"СтартТаймер", L"СтопТаймер", L"Отправить16", L"Получить16", L"из16", L"ПолучитьНомерВерсии", L"ТестУстройства", L"УстановитьПараметр", L"ПолучитьПараметры", L"ПолучитьОписание", L"ПолучитьДополнительныеДействия"};

static const wchar_t g_kClassNames[] = L"CAddInNative"; //"|OtherClass1|OtherClass2";
static IAddInDefBase *pAsyncEvent = NULL;

HANDLE hComm;
HANDLE hTempFile;

char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

std::wstring verwstr = L"1.4.2";
std::wstring st1wstr = L"IKS";
std::wstring st2wstr = L"QUAD";
std::wstring st3wstr = L"Фискальный регистратор";
std::wstring addrwstr = L"kermit.kiev.ua";
std::wstring paramwstr = L"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\
<Settings>\
	<Group Caption=\"Параметры устройства\">\
		<Parameter Name=\"Port\" Caption=\"Port\" TypeValue=\"Number\" DefaultValue=\"1\"></Parameter>\
		<Parameter Name=\"Speed\" Caption=\"Speed\" TypeValue=\"Number\" DefaultValue=\"9600\" MasterParameterOperation=\"NotEqual\" MasterParameterName=\"Port\" MasterParameterValue=\"0\">\
			<ChoiceList>\
				<Item Value=\"9600\">9600</Item>\
				<Item Value=\"19200\">19200</Item>\
				<Item Value=\"38400\">38400</Item>\
			</ChoiceList>\
		</Parameter>\
	</Group>\
</Settings>";
std::wstring nopewstr = L"";

static wchar_t strOK[] = L"OK";

uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len = 0);
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len = 0);
uint32_t getLenShortWcharStr(const WCHAR_T* Source);

std::wstring strtowstr(const std::string &str);
std::string wstrtostr(const std::wstring &wstr);
std::string byte_2_str(char* bytes, int size);
int str_2_byte(char* bytes, std::string instr, int size);
int subst( char* str, int ln, char* substr, int subln);

//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
    if(!*pInterface)
    {
        *pInterface= new CAddInNative;
        return (long)*pInterface;
    }
    return 0;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pIntf)
{
   if(!*pIntf)
      return -1;

   delete *pIntf;
   *pIntf = 0;
   return 0;
}
//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
    static WCHAR_T* names = 0;
    if (!names)
        ::convToShortWchar(&names, g_kClassNames);
    return names;
}
//---------------------------------------------------------------------------//
#ifndef __linux__
VOID CALLBACK MyTimerProc(
        HWND hwnd, // handle of window for timer messages
        UINT uMsg, // WM_TIMER message
        UINT idEvent, // timer identifier
        DWORD dwTime // current system time
);
#else
static void MyTimerProc(int sig);
#endif //__linux__

// CAddInNative
//---------------------------------------------------------------------------//
CAddInNative::CAddInNative()
{
    m_iMemory = 0;
    m_iConnect = 0;

	m_port = 1;
	m_baud = 9600;
	m_parity = 0;
	m_stopBit = 0;
	m_err = 0;
	m_byteSize = 8;

	m_isOpen = false;
}
//---------------------------------------------------------------------------//
CAddInNative::~CAddInNative()
{
	if (m_isOpen){
		CAddInNative::ClosePort();
	}
}
//---------------------------------------------------------------------------//
bool CAddInNative::Init(void* pConnection)
{ 
    m_iConnect = (IAddInDefBase*)pConnection;
    return m_iConnect != NULL;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetInfo()
{ 
    // Component should put supported component technology version 
    // This component supports 2.0 version
    return 2016; 
}
//---------------------------------------------------------------------------//
void CAddInNative::Done()
{
}
/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase
//---------------------------------------------------------------------------//
bool CAddInNative::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{ 
    wchar_t *wsExtension = L"IKS";
    int iActualSize = ::wcslen(wsExtension) + 1;
    WCHAR_T* dest = 0;

    if (m_iMemory)
    {
        if(m_iMemory->AllocMemory((void**)wsExtensionName, iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(wsExtensionName, wsExtension, iActualSize);
        return true;
    }

    return false; 
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNProps()
{ 
    // You may delete next lines and add your own implementation code here
    return ePropLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindProp(const WCHAR_T* wsPropName)
{ 
    long plPropNum = -1;
    wchar_t* propName = 0;

    ::convFromShortWchar(&propName, wsPropName);
    plPropNum = findName(g_PropNames, propName, ePropLast);

    if (plPropNum == -1)
        plPropNum = findName(g_PropNamesRu, propName, ePropLast);

    delete[] propName;

    return plPropNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetPropName(long lPropNum, long lPropAlias)
{ 
    if (lPropNum >= ePropLast)
        return NULL;

    wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsPropName = NULL;
    int iActualSize = 0;

    switch(lPropAlias)
    {
    case 0: // First language
        wsCurrentName = g_PropNames[lPropNum];
        break;
    case 1: // Second language
        wsCurrentName = g_PropNamesRu[lPropNum];
        break;
    default:
        return 0;
    }
    
    iActualSize = wcslen(wsCurrentName)+1;

    if (m_iMemory && wsCurrentName)
    {
        if (m_iMemory->AllocMemory((void**)&wsPropName, iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(&wsPropName, wsCurrentName, iActualSize);
    }

    return wsPropName;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{ 
    switch(lPropNum)
    {
    case ePropIsOpen:
        TV_VT(pvarPropVal) = VTYPE_BOOL;
        TV_BOOL(pvarPropVal) = m_isOpen;
        break;
    case ePropPort:
		TV_VT(pvarPropVal) = VTYPE_UI1;
		TV_UI1(pvarPropVal) = m_port;
		break;
    case ePropBaud:
		TV_VT(pvarPropVal) = VTYPE_UI4;
		TV_UI4(pvarPropVal) = m_baud;
		break;    
    case ePropByteSize:
		TV_VT(pvarPropVal) = VTYPE_UI1;
		TV_UI1(pvarPropVal) = m_byteSize;
		break;    
	case ePropParity:
		TV_VT(pvarPropVal) = VTYPE_UI1;
		TV_UI1(pvarPropVal) = m_parity;
		break;
    case ePropStopBit:
		TV_VT(pvarPropVal) = VTYPE_UI1;
		TV_UI1(pvarPropVal) = m_stopBit;
		break;
    case ePropError:
		TV_VT(pvarPropVal) = VTYPE_I4;
		TV_I4(pvarPropVal) = m_err;
		break;
    case ePropCommand:
		wstring_to_p(m_cmd, pvarPropVal);
		break;
	case ePropAnswer:
		wstring_to_p(m_ans, pvarPropVal);
		break;
	case ePropLoging:
        TV_VT(pvarPropVal) = VTYPE_BOOL;
        TV_BOOL(pvarPropVal) = m_loging;
        break;
    default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::SetPropVal(const long lPropNum, tVariant *varPropVal)
{ 
    switch(lPropNum)
    { 
	case ePropIsOpen:
        //if (TV_VT(varPropVal) != VTYPE_BOOL)
        //    return false;
        //m_boolEnabled = TV_BOOL(varPropVal);
        //break;
	case ePropLoging:
        if (TV_VT(varPropVal) != VTYPE_BOOL)
            return false;
        m_loging = TV_BOOL(varPropVal);
        break;
	case ePropPort:
    case ePropBaud:
    case ePropParity:
    case ePropStopBit:
	case ePropCommand:
	case ePropAnswer:
	default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropReadable(const long lPropNum)
{ 
    switch(lPropNum)
    { 
	case ePropIsOpen:
	case ePropPort:
    case ePropBaud:
    case ePropParity:
    case ePropStopBit:
	case ePropCommand:
	case ePropAnswer:
	case ePropError:
	case ePropLoging:
		return true;
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropWritable(const long lPropNum)
{
    switch(lPropNum)
    { 
	case ePropLoging:
		return true;
	case ePropIsOpen:
	case ePropPort:
    case ePropBaud:
    case ePropParity:
    case ePropStopBit:
	case ePropCommand:
	case ePropAnswer:
		return false;
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNMethods()
{ 
    return eMethLast;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindMethod(const WCHAR_T* wsMethodName)
{ 
    long plMethodNum = -1;
    wchar_t* name = 0;

    ::convFromShortWchar(&name, wsMethodName);

    plMethodNum = findName(g_MethodNames, name, eMethLast);

    if (plMethodNum == -1)
        plMethodNum = findName(g_MethodNamesRu, name, eMethLast);

    return plMethodNum;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetMethodName(const long lMethodNum, const long lMethodAlias)
{ 
    if (lMethodNum >= eMethLast)
        return NULL;

    wchar_t *wsCurrentName = NULL;
    WCHAR_T *wsMethodName = NULL;
    int iActualSize = 0;

    switch(lMethodAlias)
    {
    case 0: // First language
        wsCurrentName = g_MethodNames[lMethodNum];
        break;
    case 1: // Second language
        wsCurrentName = g_MethodNamesRu[lMethodNum];
        break;
    default: 
        return 0;
    }

    iActualSize = wcslen(wsCurrentName)+1;

    if (m_iMemory && wsCurrentName)
    {
        if(m_iMemory->AllocMemory((void**)&wsMethodName, iActualSize * sizeof(WCHAR_T)))
            ::convToShortWchar(&wsMethodName, wsCurrentName, iActualSize);
    }

    return wsMethodName;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNParams(const long lMethodNum)
{ 
    switch(lMethodNum)
    { 
    case eMethGetInfo:
        return 7;
    case eMethOpen:
        return 5;
    case eMethSendIKS:
    case eMethTest:
    case eMethSetParam:
		return 2;
    case eMethSend:
	case eMethStartTimer:
    case eMethSendHex:
	case eMethDelay:
	case eMethToHex:
	case eMethFromHex:
	case eMethGetParams:
	case eMethGetMore:
		return 1;
    default:
        return 0;
    }
    
    return 0;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetParamDefValue(const long lMethodNum, const long lParamNum,
                          tVariant *pvarParamDefValue)
{ 
    TV_VT(pvarParamDefValue)= VTYPE_EMPTY;

    switch(lMethodNum)
    { 
    case eMethOpen:
		switch (lParamNum)
		{
		case 0:
			//port = 1;
			TV_VT(pvarParamDefValue) = VTYPE_UI1;
			TV_UI1(pvarParamDefValue) = 1;
			return true;
		case 1:
			//speed = 9600;
			TV_VT(pvarParamDefValue) = VTYPE_UI4;
			TV_UI4(pvarParamDefValue) = 9600;
			return true;
		case 2:
			//no parity
			TV_VT(pvarParamDefValue) = VTYPE_UI1;
			TV_UI4(pvarParamDefValue) = 0;
			return true;
		case 3:
			//byte size = 8;
			TV_VT(pvarParamDefValue) = VTYPE_UI1;
			TV_UI4(pvarParamDefValue) = 8;
			return true;
		case 4:
			//one stop bit
			TV_VT(pvarParamDefValue) = VTYPE_UI1;
			TV_UI4(pvarParamDefValue) = 0;
			return true;
		}
		break;
	case eMethDelay:
		switch (lParamNum)
		{
		case 0:
			TV_VT(pvarParamDefValue) = VTYPE_UI4;
			TV_UI4(pvarParamDefValue) = 10;
			return true;
		}
        break;
    case eMethSendIKS:
		switch (lParamNum)
		{
		case 0:
			//cmd = 0;
			TV_VT(pvarParamDefValue) = VTYPE_UI1;
			TV_UI1(pvarParamDefValue) = 0;
			return true;
		}
        break;
	case eMethStartTimer:
		switch (lParamNum)
		{
		case 0:
			TV_VT(pvarParamDefValue) = VTYPE_UI4;
			TV_UI4(pvarParamDefValue) = 1000;
			return true;
		}
        break;
	case eMethClose:
    case eMethSend:
    case eMethToHex:
    case eMethFromHex:
    case eMethRecieve:
    case eMethSendHex:
    case eMethRecieveHex:
    case eMethStopTimer:
       // There are no parameter values by default 
    default:
        return false;
    }

    return false;
} 
//---------------------------------------------------------------------------//
bool CAddInNative::HasRetVal(const long lMethodNum)
{ 
    switch(lMethodNum)
    { 
    case eMethOpen:
    case eMethSend:
    case eMethSendIKS:
    case eMethRecieve:
    case eMethSendHex:
    case eMethRecieveHex:
    case eMethToHex:
    case eMethFromHex:
    case eMethVersion:
    case eMethTest:
	case eMethSetParam:
        return true;
    default:
        return false;
    }

    return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsProc(const long lMethodNum,
                    tVariant* paParams, const long lSizeArray)
{
	uint32_t dt;
	switch(lMethodNum)
    { 
    case eMethClose:
		CAddInNative::ClosePort();
        break;
    case eMethDelay:
		dt = TV_UI4(paParams);
		CAddInNative::Delay(dt);
        break;
    case eMethStartTimer:
		if (!lSizeArray || TV_VT(paParams) != VTYPE_I4 || TV_I4(paParams) <= 0)
			return false;
		if (!pAsyncEvent) pAsyncEvent = m_iConnect;
        /* The task of length of turn of messages
        if (m_iConnect)
            m_iConnect->SetEventBufferDepth(4000);
        */
        m_uiTimer = ::SetTimer(NULL,0,TV_I4(paParams),(TIMERPROC)MyTimerProc);
        break;
    case eMethStopTimer:
        if (m_uiTimer != 0)
            ::KillTimer(NULL,m_uiTimer);
        m_uiTimer = 0;
        //pAsyncEvent = NULL;
        break;
    case eMethGetParams:
		wstring_to_p(paramwstr, paParams);
        break;
    case eMethGetInfo:
		wstring_to_p(st1wstr, paParams);
		wstring_to_p(st2wstr, paParams+1);
		wstring_to_p(st3wstr, paParams+2);
		TV_VT(paParams+3) = VTYPE_I4;
		TV_I4(paParams+3) = 2016;
		TV_VT(paParams+4) = VTYPE_BOOL;
		TV_BOOL(paParams+4) = false;
		TV_VT(paParams+5) = VTYPE_BOOL;
		TV_BOOL(paParams+5) = true;
		wstring_to_p(addrwstr, paParams+6);
		break;
    case eMethGetMore:
		wstring_to_p(nopewstr, paParams);
        break;

    default:
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsFunc(const long lMethodNum,
                tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{ 
    bool ret = false;
	int res;

	uint8_t tmpcmd;
	std::wstring tmpwstr;

    switch(lMethodNum)
    {
	case eMethOpen:

        if (!lSizeArray || !paParams)
            return false;
            
		res = 0;
		if (!m_isOpen)
		{

			m_port = TV_UI1(paParams);
			m_baud = TV_UI4(paParams + 1);
			m_parity = TV_UI1(paParams + 2);
			m_byteSize = TV_UI1(paParams + 3);
			m_stopBit = TV_UI1(paParams + 4);

			res = CAddInNative::OpenPort();
		}
		else res = 5; //allready opened

		TV_VT(pvarRetValue) = VTYPE_UI1;
		TV_UI1(pvarRetValue) = res;

		ret = true;
		break;

	case eMethSend:
		if (TV_VT(paParams)== VTYPE_PWSTR)
		{
			m_cmd = (paParams)->pwstrVal;
		}
		res = CAddInNative::Send();
		TV_VT(pvarRetValue) = VTYPE_I4;
		TV_I4(pvarRetValue) = res;
		ret = true;
		break;

	case eMethSendIKS:
		tmpcmd = TV_UI1(paParams);
		if (TV_VT(paParams+1)== VTYPE_PWSTR)
		{
			m_cmd = (paParams+1)->pwstrVal;
		}
		res = CAddInNative::SendIKS(tmpcmd);

		TV_VT(pvarRetValue) = VTYPE_I4;
		TV_I4(pvarRetValue) = res;
		ret = true;
		break;

	case eMethRecieve:
		res = CAddInNative::Recieve();
		TV_VT(pvarRetValue) = VTYPE_I4;
		TV_I4(pvarRetValue) = res;
		ret = true;
		break;

	case eMethSendHex:
		if (TV_VT(paParams)== VTYPE_PWSTR)
		{
			m_cmd = (paParams)->pwstrVal;
		}
		res = CAddInNative::SendHex();
		TV_VT(pvarRetValue) = VTYPE_I4;
		TV_I4(pvarRetValue) = res;
		ret = true;
		break;

	case eMethRecieveHex:
		res = CAddInNative::RecieveHex();
		TV_VT(pvarRetValue) = VTYPE_I4;
		TV_I4(pvarRetValue) = res;
		ret = true;
		break;

	case eMethToHex:
		if (TV_VT(paParams)== VTYPE_PWSTR)
		{
			tmpwstr = (paParams)->pwstrVal;
		}
		tmpwstr = CAddInNative::ToHEX(tmpwstr);
		wstring_to_p(tmpwstr, pvarRetValue);
		ret = true;
		break;

	case eMethFromHex:
		ret = false;
		if (TV_VT(paParams) == VTYPE_PWSTR)
		{
			tmpwstr = (paParams)->pwstrVal;
			tmpwstr = CAddInNative::FromHEX(tmpwstr);
			wstring_to_p(tmpwstr, pvarRetValue);
			ret = true;
		}
		break;
	case eMethVersion:
		wstring_to_p(verwstr, pvarRetValue);
		ret = true;
		break;
	case eMethTest:
		ret = false;
		res = CAddInNative::OpenPort();
		if (res == 0) {	
			ret = true;
			//CAddInNative::ClosePort();
		}
		TV_VT(pvarRetValue) = VTYPE_BOOL;
		TV_BOOL(pvarRetValue) = ret;

		//if (m_iConnect) {
		//	m_iConnect->SetStatusLine(strOK);
		//	//m_iConnect->Alert(strOK);
		//}

		ret = true;
		break;
    case eMethSetParam:
		ret = false;
		if (TV_VT(paParams) == VTYPE_PWSTR)
		{
			tmpwstr = (paParams)->pwstrVal;
			if ((tmpwstr.size() == 4) && std::equal(tmpwstr.begin(), tmpwstr.end(), L"Port")){
				m_port = TV_UI1(paParams + 1);
				ret = true;
			}
			if ((tmpwstr.size() == 5) && std::equal(tmpwstr.begin(), tmpwstr.end(), L"Speed")){
				m_baud = TV_UI4(paParams + 1);
				ret = true;
			}
			TV_VT(pvarRetValue) = VTYPE_BOOL;
			TV_BOOL(pvarRetValue) = ret;
			ret = true;
		}
        break;
	}
    return ret; 
}
//---------------------------------------------------------------------------//
// This code will work only on the client!
#ifndef __linux__
VOID CALLBACK MyTimerProc(
  HWND hwnd,    // handle of window for timer messages
  UINT uMsg,    // WM_TIMER message
  UINT idEvent, // timer identifier
  DWORD dwTime  // current system time
)
{
    if (!pAsyncEvent)
        return;

    wchar_t *who = L"ComponentNative", *what = L"Timer";

    wchar_t *wstime = new wchar_t[TIME_LEN];
    if (wstime)
    {
        wmemset(wstime, 0, TIME_LEN);
        ::_ultow(dwTime, wstime, 10);
        pAsyncEvent->ExternalEvent(who, what, wstime);
        delete[] wstime;
    }
}
#else
void MyTimerProc(int sig)
{
    if (pAsyncEvent)
        return;

    WCHAR_T *who = 0, *what = 0, *wdata = 0;
    wchar_t *data = 0;
    time_t dwTime = time(NULL);

    data = new wchar_t[TIME_LEN];
    
    if (data)
    {
        wmemset(data, 0, TIME_LEN);
        swprintf(data, TIME_LEN, L"%ul", dwTime);
        ::convToShortWchar(&who, L"ComponentNative");
        ::convToShortWchar(&what, L"Timer");
        ::convToShortWchar(&wdata, data);

        pAsyncEvent->ExternalEvent(who, what, wdata);

        delete[] who;
        delete[] what;
        delete[] wdata;
        delete[] data;
    }
}
#endif
//---------------------------------------------------------------------------//
void CAddInNative::SetLocale(const WCHAR_T* loc)
{
#ifndef __linux__
    _wsetlocale(LC_ALL, loc);
#else
    //We convert in char* char_locale
    //also we establish locale
    //setlocale(LC_ALL, char_locale);
#endif
}
/////////////////////////////////////////////////////////////////////////////
// LocaleBase
//---------------------------------------------------------------------------//
bool CAddInNative::setMemManager(void* mem)
{
    m_iMemory = (IMemoryManager*)mem;
    return m_iMemory != 0;
}
//---------------------------------------------------------------------------//
void CAddInNative::addError(uint32_t wcode, const wchar_t* source, 
                        const wchar_t* descriptor, long code)
{
    if (m_iConnect)
    {
        WCHAR_T *err = 0;
        WCHAR_T *descr = 0;
        
        ::convToShortWchar(&err, source);
        ::convToShortWchar(&descr, descriptor);

        m_iConnect->AddError(wcode, err, descr, code);
        delete[] err;
        delete[] descr;
    }
}
//---------------------------------------------------------------------------//
long CAddInNative::findName(wchar_t* names[], const wchar_t* name, 
                         const uint32_t size) const
{
    long ret = -1;
    for (uint32_t i = 0; i < size; i++)
    {
        if (!wcscmp(names[i], name))
        {
            ret = i;
            break;
        }
    }
    return ret;
}
//---------------------------------------------------------------------------//

uint8_t CAddInNative::OpenPort(void)
{
	char	pcCommPort[32];
	DCB		dcb;
	COMMTIMEOUTS timeouts;

	m_err = 0;
	m_num = 1;

	TCHAR lpTempPathBuffer[MAX_PATH];
	DWORD	dwStatus;

	if (m_loging) 
	{
		dwStatus = GetTempPath(MAX_PATH, lpTempPathBuffer);
		memcpy(lpTempPathBuffer + dwStatus, TEXT("iks.log"),14);
		//uRetVal = GetTempFileName(lpTempPathBuffer, TEXT("maria"), 0, szTempFileName);
	    hTempFile = CreateFile((LPTSTR) lpTempPathBuffer,  GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);                // no template 
		dwStatus = SetFilePointer(hTempFile, 0, NULL, FILE_END);
	}

	sprintf(pcCommPort, "\\\\.\\COM%d", m_port);
	hComm = CreateFileA(pcCommPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hComm == INVALID_HANDLE_VALUE)
	{
		m_err = 1;
		return 1; //error opening com port
	}
	if (!GetCommState(hComm, &dcb))
	{
		CAddInNative::ClosePort();
		m_err = 2;
		return 2;  //error geting com port settings
	}
	dcb.BaudRate	= m_baud;
	dcb.ByteSize	= m_byteSize;
	dcb.Parity		= m_parity;
	dcb.StopBits	= m_stopBit;
	//dcb.fDtrControl = DTR_CONTROL_DISABLE;

	if (m_parity != 0) dcb.fParity = TRUE;

	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	if (!SetCommState(hComm, &dcb))
	{
		CAddInNative::ClosePort();
		m_err = 3;
		return 3; //error write com port settings 
	}

	memset(&timeouts, 0, sizeof(timeouts));
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.ReadIntervalTimeout = 0;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	if (!SetCommTimeouts(hComm, &timeouts))
	{
		CAddInNative::ClosePort();
		m_err = 4;
		return 4; //error seting com port timeouts
	}

	m_isOpen = true;
	write_log("\0\0\0\0\0", 5, 'o');

	return 0;

}

void CAddInNative::ClosePort(void)
{
	//if (m_isOpen) {
	CloseHandle(hComm);
	//}
	
	if (m_loging) CloseHandle(hTempFile);

	m_isOpen = false;
	return;
}

void CAddInNative::Delay(int nDelay)
{
	Sleep(nDelay);
	return;
}


void CAddInNative::write_log(char* OUTBUFFER, int l, char log_type)
{
	char	TMPBUFFER[50];
    DWORD   bytes_written = 0;
	int		bStatus;
	std::string s;

	if (m_loging)
	{
		time_t t = time(0);   // get time now
		struct tm * now = localtime( & t );

		sprintf(TMPBUFFER,"[%04d%02d%02d%02d%02d%02d]", now->tm_year+1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
		TMPBUFFER[16] = log_type;
		TMPBUFFER[17] = ':';
		bStatus = WriteFile(hTempFile, &TMPBUFFER, 18, &bytes_written, NULL);

		s = byte_2_str(OUTBUFFER, l);

		bStatus = WriteFile(hTempFile, s.c_str(), s.length(), &bytes_written, NULL);

		TMPBUFFER[0] = '\r';
		TMPBUFFER[1] = '\n';
		bStatus = WriteFile(hTempFile, &TMPBUFFER, 2, &bytes_written, NULL);

	}
}

int CAddInNative::Send(void)
{
	char	OUTBUFFER[256];
	DWORD   bytes_written = 0;

	int		bStatus;
	uint8_t l;
	//char ss[50];

	std::string s;

	m_err = 0;

	l = m_cmd.length();
	if (l > 256)
	{
		m_err = -1;
		return -1; //to long command
	}

	s = wstrtostr(m_cmd);
	l = s.length();
	if (l > 256)
	{
		m_err = -1;
		return -1; //to long command
	}

	memcpy(OUTBUFFER, s.c_str(), l);
	//OUTBUFFER[l] = (char)0;

	//s = OUTBUFFER;
	//sprintf(ss, "=%d =%d =%d =%d =%d", OUTBUFFER[0], OUTBUFFER[1], OUTBUFFER[2], OUTBUFFER[3], l);
	//s = ss;
	//s.resize(l+1);
	//m_cmd.assign(s.begin(), s.end());


	bStatus = WriteFile(hComm, &OUTBUFFER, l, &bytes_written, NULL);
	if (!bStatus)
	{
		m_err = -2;
		return -2; //error while data send
	}

	write_log(OUTBUFFER, l, 's');	
	return (int) bytes_written;
}


int CAddInNative::SendIKS(uint8_t cmd)
{
	uint8_t	OUTBUFFER[256];
	DWORD   bytes_written = 0;
	uint8_t	INBUFFER[1024];
	DWORD   bytes_read = 0;
	uint8_t	TMPBUFFER[64];
	DWORD   total_bytes_read = 0;

	int		bStatus;
	int		l, i;
	int		pos_begin = -1, pos_end = -1; 
	int		cnt = 10;
	uint8_t bc;

	std::string s;

	m_err = 0;

	m_ans = (L"");

	s = wstrtostr(m_cmd);
	l = s.length();
	if (l > 256 * 2)
	{
		m_err = -2;
		return -2; //to long command
	}

	OUTBUFFER[0] = 0x10;
	OUTBUFFER[1] = 0x02;
	OUTBUFFER[2] = m_num;
	OUTBUFFER[3] = cmd;
	//memcpy(OUTBUFFER+4, s.c_str(), l);
	str_2_byte((char*) OUTBUFFER+4, s, l);
	l /=2;
	bc = 0;
	for (i=2; i<=l+3; i++){
		bc += OUTBUFFER[i];
	}
	OUTBUFFER[l+4] = 0x100 - bc;
	OUTBUFFER[l+5] = 0x10;
	OUTBUFFER[l+6] = 0x03;

	s = byte_2_str((char*) OUTBUFFER, l+7);
	m_cmd.assign(s.begin(), s.end());

	if (!m_isOpen)
	{
		m_err = -1;
		return -1; //to long command
	}

	bStatus = WriteFile(hComm, &OUTBUFFER, l+7, &bytes_written, NULL);
	if (!bStatus)
	{
		m_err = -3;
		return -3; //error while data send
	}

	m_num++;
	write_log((char*)  OUTBUFFER, l+7, 'S');

	total_bytes_read = 0;
	for (i=0; i<cnt && pos_end<0; i++){
		bytes_read = 0;
		bStatus = ReadFile(hComm, &TMPBUFFER, 64, &bytes_read, NULL);
		if (!bStatus)
		{
			m_err = -4;
			return -4; //error while data recieve
		}

		if (bytes_read){
			memcpy(INBUFFER+total_bytes_read, TMPBUFFER, bytes_read);
			total_bytes_read += bytes_read;
			INBUFFER[total_bytes_read] = 0;
			if (pos_begin < 0){
				if (TMPBUFFER[0] == 0x06) cnt += 10;
				if (TMPBUFFER[0] == 0x15){
					m_err = -5;
					s = byte_2_str((char*) INBUFFER, bytes_read);
					m_ans.assign(s.begin(), s.end());
					return -5; //bad command
				}
				if (TMPBUFFER[0] == 0x16){
					//m_err = -6;
					//return -6; //busy
					cnt += 20;
				}
				pos_begin = subst((char*) INBUFFER, total_bytes_read, "\x10\x02", 2);
			} 
			if (pos_begin >= 0){
				pos_end = subst((char*) INBUFFER, total_bytes_read, "\x10\x03", 2);
			}
		}


	}

	write_log((char*) INBUFFER, bytes_read, 'R');
	s = byte_2_str((char*) INBUFFER, bytes_read);
	m_ans.assign(s.begin(), s.end());

	if (pos_begin <0)
	{
		m_err = -6;
		return -6; //no responce
	}

	if (pos_end <0)
	{
		m_err = -7;
		return -7; //no end of responce
	}

	bc = 0;
	for (i=pos_begin+2; i<pos_end-1; i++){
		bc += INBUFFER[i];
	}
	if (INBUFFER[pos_end-1]!=(0x100-bc)){
		m_err = -8;
		return -8; //error bc in responce
	}

	return (int) bytes_read;
}

std::wstring CAddInNative::ToHEX(std::wstring s){
	std::string tmps, ress;
	tmps = wstrtostr(s);
	ress = byte_2_str((char*) tmps.c_str(), tmps.length());
	return strtowstr(ress);
}

std::wstring CAddInNative::FromHEX(std::wstring s){
	std::string tmps, ress;
	tmps = wstrtostr(s);
	int l = tmps.length();
	uint8_t *OUTBUFFER;
	OUTBUFFER = (uint8_t*) malloc(sizeof(*OUTBUFFER)*(l/2)+1);
	if (OUTBUFFER == NULL){
		return L"";
	}
	str_2_byte((char*) OUTBUFFER, tmps, l);
	OUTBUFFER[l/2] = 0;
	ress = (char*) OUTBUFFER;
	free(OUTBUFFER);
	return strtowstr(ress);
}

int CAddInNative::Recieve(void)
{
	char	INBUFFER[1024];
	DWORD   bytes_read = 0;
	int		bStatus;

	std::string s;

	m_ans = (L"");
	m_err = 0;

	bytes_read = 0;
	bStatus = ReadFile(hComm, &INBUFFER, 1024, &bytes_read, NULL);
	if (!bStatus)
	{
		m_err = -1;
		return -1; //error while data recieve
	}

	write_log(INBUFFER, bytes_read, 'r');
	s = INBUFFER;

	s.resize(bytes_read);
	m_ans = strtowstr(s);

	return (int)bytes_read;
}

int CAddInNative::SendHex(void)
{
	char	OUTBUFFER[256];
	DWORD   bytes_written = 0;

	int		bStatus;
	uint8_t l;
	//char ss[50];

	std::string s;

	m_err = 0;

	l = m_cmd.length();
	if (l > 512)
	{
		m_err = -1;
		return -1; //to long command
	}

	s = wstrtostr(m_cmd);
	l = s.length();
	if (l > 512)
	{
		m_err = -1;
		return -1; //to long command
	}

	str_2_byte(OUTBUFFER, s, l);
	l /=2;
	OUTBUFFER[l] = 0;


	bStatus = WriteFile(hComm, &OUTBUFFER, l, &bytes_written, NULL);
	if (!bStatus)
	{
		m_err = -2;
		return -2; //error while data send
	}

	return (int) bytes_written;
}

int CAddInNative::RecieveHex(void)
{
	char	INBUFFER[1024];
	DWORD   bytes_read = 0;
	int		bStatus;

	std::string s;

	m_ans = (L"");
	m_err = 0;

	bytes_read = 0;
	bStatus = ReadFile(hComm, &INBUFFER, 1024, &bytes_read, NULL);
	if (!bStatus)
	{
		m_err = -1;
		return -1; //error while data recieve
	}

	s = byte_2_str(INBUFFER, bytes_read);
	m_ans.assign(s.begin(), s.end());

	return (int)bytes_read;
}


uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len)
{
    if (!len)
        len = ::wcslen(Source)+1;

    if (!*Dest)
        *Dest = new WCHAR_T[len];

    WCHAR_T* tmpShort = *Dest;
    wchar_t* tmpWChar = (wchar_t*) Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len*sizeof(WCHAR_T));
    do
    {
        *tmpShort++ = (WCHAR_T)*tmpWChar++;
        ++res;
    }
    while (len-- && *tmpWChar);

    return res;
}
//---------------------------------------------------------------------------//
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len)
{
    if (!len)
        len = getLenShortWcharStr(Source)+1;

    if (!*Dest)
        *Dest = new wchar_t[len];

    wchar_t* tmpWChar = *Dest;
    WCHAR_T* tmpShort = (WCHAR_T*)Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len*sizeof(wchar_t));
    do
    {
        *tmpWChar++ = (wchar_t)*tmpShort++;
        ++res;
    }
    while (len-- && *tmpShort);

    return res;
}
//---------------------------------------------------------------------------//
uint32_t getLenShortWcharStr(const WCHAR_T* Source)
{
    uint32_t res = 0;
    WCHAR_T *tmpShort = (WCHAR_T*)Source;

    while (*tmpShort++)
        ++res;

    return res;
}
//---------------------------------------------------------------------------//

std::string wstrtostr(const std::wstring &wstr)
{
	// Convert a Unicode string to an ASCII string
	std::string strTo;
	char *szTo = new char[wstr.length() + 1];
	szTo[wstr.size()] = '\0';
	WideCharToMultiByte(CP_OEMCP, 0, wstr.c_str(), -1, szTo, (int)wstr.length(), NULL, NULL);
	strTo = szTo;
	delete[] szTo;
	return strTo;
}

std::wstring strtowstr(const std::string &str)
{
	// Convert an ASCII string to a Unicode String
	std::wstring wstrTo;
	wchar_t *wszTo = new wchar_t[str.length() + 1];
	wszTo[str.size()] = L'\0';
	MultiByteToWideChar(CP_OEMCP, 0, str.c_str(), -1, wszTo, (int)str.length());
	wstrTo = wszTo;
	delete[] wszTo;
	return wstrTo;
}

bool CAddInNative::wstring_to_p(std::wstring str, tVariant* val) {
	char* t1;
	TV_VT(val) = VTYPE_PWSTR;
	m_iMemory->AllocMemory((void**)&t1, (str.length() + 1) * sizeof(WCHAR_T));
	memcpy(t1, str.c_str(), (str.length() + 1) * sizeof(WCHAR_T));
	val->pstrVal = t1;
	val->strLen = str.length();
	return true;
}

std::string byte_2_str(char* bytes, int size) {
  std::string str;
  for (int i = 0; i < size; ++i) {
    const char ch = bytes[i];
    str.append(&hex[(ch  & 0xF0) >> 4], 1);
    str.append(&hex[ch & 0xF], 1);
  }
  return str;
}

int str_2_byte(char* bytes, std::string instr, int size) {
	char lc, hc;
	int res = 1;
	int l = size / 2;
	for (int i = 0; i < l; ++i) {
		hc = instr[i*2];
		if (hc >= 'a' && hc <='f') hc -= 0x57; else 
		if (hc >= 'A' && hc <='F') hc -= 0x37; else 
		if (hc >= '0' && hc <='9') hc -= 0x30; else {
			res = 0;
			break;
		}
		lc = instr[i*2+1]; 
		if (lc >= 'a' && lc <='f') lc -= 0x57; else 
		if (lc >= 'A' && lc <='F') lc -= 0x37; else 
		if (lc >= '0' && lc <='9') lc -= 0x30; else {
			res = 0;
			break;
		}
		bytes[i] = (hc << 4) + lc;
  }
  return res;
}

int subst( char * str, int ln, char * substr, int subln)
{
	int i,j,k=0;
	int ret = -1;
	for(i=0; i<(ln-subln+1); i++)
	{
		if ( *(str+i) == *substr )
		{
			ret = i;
			for(j=1; j<subln; j++)
			{
				if ( *(str+i+j) != *(substr+j) ) ret = -1;
			}
			if (ret >=0) return ret;
		}
	}
	return ret;
}

