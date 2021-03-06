/*
 *  Gwork
 *  Copyright (c) 2010 Facepunch Studios
 *  Copyright (c) 2013-16 Billy Quith
 *  See license in Gwork.h
 */

#include <Gwork/ToolTip.h>
#include <Gwork/Utility.h>

#include <cstdio>

// For Unicode support.
// Note: <codecvt> is C++11 and in Xcode you'll need to use libc++ (LLVM), not libstdc++ (GNU).
#include <locale>       // Narrow/widen
#include <codecvt>      // Narrow/widen - C++11

namespace Gwk {
namespace Utility {
    
//  This section from: https://github.com/bkaradzic/bx/blob/master/include/bx/string.h
//
//  Copyright 2010-2013 Branimir Karadzic. All rights reserved.
//  License: http://www.opensource.org/licenses/BSD-2-Clause
//

/// Cross platform implementation of vsnprintf that returns number of
/// characters which would have been written to the final string if
/// enough space had been available.
int vsnprintf(char* _str, size_t _count, const char* _format, va_list _argList)
{
#if defined(_MSC_VER)
    int len = ::vsnprintf_s(_str, _count, _count, _format, _argList);
    return -1 == len ? ::_vscprintf(_format, _argList) : len;
#else
    return ::vsnprintf(_str, _count, _format, _argList);
#endif
}

/// Cross platform implementation of vsnwprintf that returns number of
/// characters which would have been written to the final string if
/// enough space had been available.
int vsnwprintf(wchar_t* _str, size_t _count, const wchar_t* _format, va_list _argList)
{
#if defined(_MSC_VER)
    int len = ::_vsnwprintf_s(_str, _count, _count, _format, _argList);
    return -1 == len ? ::_vscwprintf(_format, _argList) : len;
#elif defined(__MINGW32__)
    return ::vsnwprintf(_str, _count, _format, _argList);
#else
    return ::vswprintf(_str, _count, _format, _argList);
#endif
}

int snprintf(char* _str, size_t _count, const char* _format, ...)
{
    va_list argList;
    va_start(argList, _format);
    int len = vsnprintf(_str, _count, _format, argList);
    va_end(argList);
    return len;
}

int swnprintf(wchar_t* _out, size_t _count, const wchar_t* _format, ...)
{
    va_list argList;
    va_start(argList, _format);
    int len = vsnwprintf(_out, _count, _format, argList);
    va_end(argList);
    return len;
}

void PrintfVargs(std::string& _out, const char* _format, va_list _argList)
{
    char temp[2048];
    
    char* out = temp;
    int len = Utility::vsnprintf(out, sizeof(temp), _format, _argList);
    if ( int(sizeof(temp)) < len)
    {
        out = (char*)alloca(len+1);
        len = Utility::vsnprintf(out, len, _format, _argList);
    }
    out[len] = '\0';
    _out.append(out);
}

void Printf(std::string& _out, const char* _format, ...)
{
    va_list argList;
    va_start(argList, _format);
    PrintfVargs(_out, _format, argList);
    va_end(argList);
}

String Format(const char* _format, ...)
{
    va_list argList;
    va_start(argList, _format);
    String out;
    PrintfVargs(out, _format, argList);
    va_end(argList);
    return out;
}

std::wstring Widen(const Gwk::String &nstr)
{
    // UTF-8 to UTF-16 (C++11)
    // See: http://en.cppreference.com/w/cpp/locale/codecvt_utf8_utf16
    // See: http://www.cplusplus.com/reference/codecvt/codecvt_utf8_utf16/
    
    std::wstring_convert< std::codecvt_utf8_utf16<wchar_t>, wchar_t > conversion;
    const std::wstring wstr( conversion.from_bytes( nstr.c_str() ) );
    
    return wstr;
}

Gwk::String Narrow(const std::wstring &wstr)
{
    // wide to UTF-8 (C++11)
    // See: http://en.cppreference.com/w/cpp/locale/wstring_convert/to_bytes
    
    std::wstring_convert< std::codecvt_utf8<wchar_t> > conv1;
    Gwk::String u8str = conv1.to_bytes(wstr);
    
    return u8str;
}

void Replace(String& str, const String& strFind, const String& strReplace)
{
    size_t pos = 0;
    
    while ((pos = str.find(strFind, pos)) != String::npos)
    {
        str.replace(pos, strFind.length(), strReplace);
        pos += strReplace.length();
    }
}

void Strings::Split(const Gwk::String& str, const Gwk::String& seperator,
                    Strings::List& outbits, bool bLeave)
{
    size_t iOffset = 0;
    size_t iLength = str.length();
    size_t iSepLen = seperator.length();
    size_t i = str.find(seperator, 0);
    
    while (i != std::string::npos)
    {
        outbits.push_back(str.substr(iOffset, i-iOffset));
        iOffset = i+iSepLen;
        i = str.find(seperator, iOffset);
        
        if (bLeave)
            iOffset -= iSepLen;
    }
    
    outbits.push_back(str.substr(iOffset, iLength-iOffset));
}

int Strings::To::Int(const Gwk::String& str)
{
    if (str == "")
        return 0;
    
    return atoi(str.c_str());
}

float Strings::To::Float(const Gwk::String& str)
{
    if (str == "")
        return 0.0f;
    
    return static_cast<float>( atof(str.c_str()) );
}

bool Strings::To::Bool(const Gwk::String& str)
{
    if (str.size() == 0)
        return false;
    
    if (str[0] == 'T' || str[0] == 't' || str[0] == 'y' || str[0] == 'Y')
        return true;
    
    if (str[0] == 'F' || str[0] == 'f' || str[0] == 'n' || str[0] == 'N')
        return false;
    
    if (str[0] == '0')
        return false;
    
    return true;
}

bool Strings::To::Floats(const Gwk::String& str, float* f, size_t iCount)
{
    Strings::List lst;
    Strings::Split(str, " ", lst);
    
    if (lst.size() != iCount)
        return false;
    
    for (size_t i = 0; i < iCount; i++)
    {
        f[i] = Strings::To::Float(lst[i]);
    }
    
    return true;
}

bool Strings::Wildcard(const String& strWildcard, const String& strHaystack)
{
    const String& W = strWildcard;
    const String& H = strHaystack;
    
    if (strWildcard == "*")
        return true;
    
    size_t iPos = W.find("*", 0);
    
    if (iPos == String::npos)
        return strWildcard == strHaystack;
    
    // First half matches
    if (iPos > 0 && W.substr(0, iPos) != H.substr(0, iPos))
        return false;
    
    // Second half matches
    if (iPos != W.length()-1)
    {
        String strEnd = W.substr(iPos+1, W.length());
        
        if (strEnd != H.substr(H.length()-strEnd.length(), H.length()))
            return false;
    }
    
    return true;
}

void Strings::ToUpper(Gwk::String& str)
{
    transform(str.begin(), str.end(), str.begin(), toupper);
}

void Strings::Strip(Gwk::String& str, const Gwk::String& chars)
{
    Gwk::String Source = str;
    str = "";
    
    for (unsigned int i = 0; i < Source.length(); i++)
    {
        if (chars.find(Source[i]) != Gwk::String::npos)
            continue;
        
        str += Source[i];
    }
}

Gwk::Rect ClampRectToRect(Gwk::Rect inside, Gwk::Rect outside, bool clampSize)
{
    if (inside.x < outside.x)
        inside.x = outside.x;
    
    if (inside.y  < outside.y)
        inside.y = outside.y;
    
    if (inside.x+inside.w > outside.x+outside.w)
    {
        if (clampSize)
            inside.w = outside.w;
        else
            inside.x = outside.x+outside.w-inside.w;
    }
    
    if (inside.y+inside.h > outside.y+outside.h)
    {
        if (clampSize)
            inside.h = outside.h;
        else
            inside.y = outside.w+outside.h-inside.h;
    }
    
    return inside;
}

} // namespace Utility
    
namespace Debug
{
    
void Msg(const char* str, ...)
{
    char strOut[1024];
    va_list s;
    
    va_start(s, str);
    Utility::vsnprintf(strOut, sizeof(strOut), str, s);
    va_end(s);
    
#ifdef WIN32
    OutputDebugStringA(strOut);
#else
    puts(strOut);
#endif
}

#ifdef UNICODE
void Msg(const wchar_t* str, ...)
{
    wchar_t strOut[1024];
    va_list s;
    va_start(s, str);
    vswprintf(strOut, sizeof(strOut), str, s);
    va_end(s);
    GwkUtil_OutputDebugWideString(strOut);
}
#endif // ifdef UNICODE

void AssertCheck(bool b, const char* strMsg)
{
    if (!b)
    {
        Msg("Assert: %s\n", strMsg);
#ifdef WIN32
        MessageBoxA(nullptr, strMsg, "Assert", MB_ICONEXCLAMATION | MB_OK);
        _asm { int 3 }      // Break to debugger.
#endif
        
#ifdef __APPLE__
        __builtin_trap();   // Break to debugger.
#endif
    }
}

} // namespace Debug
    
} // namespace Gwk


