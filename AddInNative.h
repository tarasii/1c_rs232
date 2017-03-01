#ifndef __ADDINNATIVE_H__
#define __ADDINNATIVE_H__

#include "ComponentBase.h"
#include "AddInDefBase.h"
#include "IMemoryManager.h"

#include <string>
#include "port.h"

///////////////////////////////////////////////////////////////////////////////
// class CAddInNative
class CAddInNative : public IComponentBase
{
public:
    enum Props
    {
		ePropIsOpen = 0,
		ePropPort,
		ePropBaud,
		ePropByteSize,
		ePropParity,
		ePropStopBit,
		ePropCommand,
		ePropAnswer,
		ePropError,
		ePropLoging,
        ePropLast      // Always last
    };

    enum Methods
    {
        eMethOpen = 0,
        eMethClose,
        eMethSend,
        eMethRecieve,
		eMethDelay,
        eMethSendIKS,
        eMethToHex,
		eMethStartTimer,
		eMethStopTimer,        
        eMethSendHex,
        eMethRecieveHex,
        eMethFromHex,
        eMethVersion,
        eMethTest,
        eMethInitMaria,
		eMethSendMaria,
		eMethStartPollACS,
		eMethLast      // Always last
    };

	//enum Parity
	//{
	//	eParOdd = 0,
	//	eParEven 
	//};

    CAddInNative(void);
    virtual ~CAddInNative();
    // IInitDoneBase
    virtual bool ADDIN_API Init(void*);
    virtual bool ADDIN_API setMemManager(void* mem);
    virtual long ADDIN_API GetInfo();
    virtual void ADDIN_API Done();
    // ILanguageExtenderBase
    virtual bool ADDIN_API RegisterExtensionAs(WCHAR_T**);
    virtual long ADDIN_API GetNProps();
    virtual long ADDIN_API FindProp(const WCHAR_T* wsPropName);
    virtual const WCHAR_T* ADDIN_API GetPropName(long lPropNum, long lPropAlias);
    virtual bool ADDIN_API GetPropVal(const long lPropNum, tVariant* pvarPropVal);
    virtual bool ADDIN_API SetPropVal(const long lPropNum, tVariant* varPropVal);
    virtual bool ADDIN_API IsPropReadable(const long lPropNum);
    virtual bool ADDIN_API IsPropWritable(const long lPropNum);
    virtual long ADDIN_API GetNMethods();
    virtual long ADDIN_API FindMethod(const WCHAR_T* wsMethodName);
    virtual const WCHAR_T* ADDIN_API GetMethodName(const long lMethodNum, 
                            const long lMethodAlias);
    virtual long ADDIN_API GetNParams(const long lMethodNum);
    virtual bool ADDIN_API GetParamDefValue(const long lMethodNum, const long lParamNum,
                            tVariant *pvarParamDefValue);   
    virtual bool ADDIN_API HasRetVal(const long lMethodNum);
    virtual bool ADDIN_API CallAsProc(const long lMethodNum,
                    tVariant* paParams, const long lSizeArray);
    virtual bool ADDIN_API CallAsFunc(const long lMethodNum,
                tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray);
    // LocaleBase
    virtual void ADDIN_API SetLocale(const WCHAR_T* loc);
	
	int CAddInNative::Recieve(void);
	std::wstring		m_ans;

private:
    long findName(wchar_t* names[], const wchar_t* name, const uint32_t size) const;
    void addError(uint32_t wcode, const wchar_t* source, 
                    const wchar_t* descriptor, long code);

	bool wstring_to_p(std::wstring s, tVariant* val);
	

	uint8_t CAddInNative::OpenPort(tVariant* paParams);
	void CAddInNative::ClosePort(void);
	void CAddInNative::Delay(int nDelay);
	int CAddInNative::Send(void);
	int CAddInNative::SendIKS(uint8_t cmd);
	int CAddInNative::SendHex(void);
	int CAddInNative::RecieveHex(void);
	bool CAddInNative::SendMaria( tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray);
	uint8_t CAddInNative::InitMaria(void);
	std::wstring CAddInNative::ToHEX(std::wstring s);
	std::wstring CAddInNative::FromHEX(std::wstring s);
	void CAddInNative::write_log(char* OUTBUFFER, int l, char log_type);


	// Attributes
    IAddInDefBase      *m_iConnect;
    IMemoryManager     *m_iMemory;

	int             m_err;
	


	uint8_t             m_num;

	bool                m_loging;

    uint32_t            m_uiTimer;

	std::wstring		m_cmd;
//	std::wstring		m_ans;

	CPort				m_ComPort;

};

#endif //__ADDINNATIVE_H__
