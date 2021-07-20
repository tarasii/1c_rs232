#include "utils.h"

char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

char HexCharPartToWord(char ch)
{
	char ret = ch;
	if (ret >= 'a' && ret <='f') ret -= 0x57; else 
	if (ret >= 'A' && ret <='F') ret -= 0x37; else 
	if (ret >= '0' && ret <='9') ret -= 0x30; else 
		ret = 0;
	return ret;
}

std::string HexToIntStr(std::string instr, bool is_signed)
{
	std::string ret = ""; 
	int l = instr.length();
	char sc;
	if (is_signed)
	{
		sc = HexCharPartToWord(instr[0]);
		sc = sc & 8;
		if (sc) ret.append("-"); else ret.append(" ");
	}
	int sm = 0;
	for (int i = 0; i < l; ++i) {
		sc = instr[i];
		if (is_signed && (i == 0)) sc = sc & 7;
		sm = sm * 16 + HexCharPartToWord(sc);
	}
	ret.append(toString(sm));
	return ret;
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
		//hc = instr[i*2];
		//if (hc >= 'a' && hc <='f') hc -= 0x57; else 
		//if (hc >= 'A' && hc <='F') hc -= 0x37; else 
		//if (hc >= '0' && hc <='9') hc -= 0x30; else {
		//	res = 0;
		//	break;
		//}
		//lc = instr[i*2+1]; 
		//if (lc >= 'a' && lc <='f') lc -= 0x57; else 
		//if (lc >= 'A' && lc <='F') lc -= 0x37; else 
		//if (lc >= '0' && lc <='9') lc -= 0x30; else {
		//	res = 0;
		//	break;
		//}
		hc = HexCharPartToWord(instr[i*2]);
		lc = HexCharPartToWord(instr[i*2+1]);
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
