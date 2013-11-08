#ifndef __ADDINNATIVE_H__
#define __ADDINNATIVE_H__

#include "ComponentBase.h"
#include "AddInDefBase.h"
#include "IMemoryManager.h"

#include <string>

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
        ePropLast      // Always last
    };

    enum Methods
    {
        eMethOpen = 0,
        eMethClose,
        eMethSend,
        eMethRecieve,
		eMethDelay,
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
    
private:
    long findName(wchar_t* names[], const wchar_t* name, const uint32_t size) const;
    void addError(uint32_t wcode, const wchar_t* source, 
                    const wchar_t* descriptor, long code);

	bool wstring_to_p(std::wstring s, tVariant* val);

	uint8_t CAddInNative::OpenPort(void);
	void CAddInNative::ClosePort(void);
	void CAddInNative::Delay(int nDelay);
	uint8_t CAddInNative::Send(void);
	uint8_t CAddInNative::Recieve(void);

	// Attributes
    IAddInDefBase      *m_iConnect;
    IMemoryManager     *m_iMemory;

	uint8_t             m_err;
	
	uint8_t             m_port;
	uint32_t            m_baud;
	uint8_t             m_byteSize;
	uint8_t             m_parity;
	uint8_t             m_stopBit;

	bool                m_isOpen;

    //uint32_t            m_uiTimer;

	std::wstring		m_cmd;
	std::wstring		m_ans;

};

#endif //__ADDINNATIVE_H__
