#ifndef __CON_UTILS_H__
#define __CON_UTILS_H__

#include <stdio.h>
#include <wchar.h>
#include <string>
//#include <algorithm>
#include <sstream>
#include "types.h"

char HexCharPartToWord(char ch);
std::string HexToIntStr(std::string instr, bool is_signed = true);
uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len = 0);
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len = 0);
uint32_t getLenShortWcharStr(const WCHAR_T* Source);
std::wstring strtowstr(const std::string &str);
std::string wstrtostr(const std::wstring &wstr);
std::string byte_2_str(char* bytes, int size);
int str_2_byte(char* bytes, std::string instr, int size);
int subst( char* str, int ln, char* substr, int subln);

template<class T>
std::string toString(const T &value) {
    std::ostringstream os;
    os << value;
    return os.str();
}

//template<typename... Args>
//std::string string_format( const std::string& format, Args... args )
//{
//    int size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
//    if( size <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
//    std::unique_ptr<char[]> buf( new char[ size ] ); 
//    snprintf( buf.get(), size, format.c_str(), args ... );
//    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
//}

#endif //__CON_UTILS_H__