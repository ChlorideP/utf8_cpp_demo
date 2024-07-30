#include <string_view>
#include "codec.h"

// see "C++ gbk utf8 converting": https://gist.github.com/mgttt/d0aba83da88552f95edafb65124d41c3
// and windows ver with this ref: https://www.sindsun.com/articles/16/136
// the IsTextUTF8 refs this: https://www.codersrc.com/archives/14823.html
// all slightly changed.

#ifndef _WIN32
#include <iconv.h> //for gbk/big5/utf8

int code_convert(
    const char* from_charset, const char* to_charset,
    char* inbuf, size_t inlen, char* outbuf, size_t outlen)
{
    iconv_t cd;
    char** pin = &inbuf;
    char** pout = &outbuf;
    cd = iconv_open(to_charset, from_charset);
    if (cd == 0)
        return -1;
    memset(outbuf, 0, outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
        return -1;
    iconv_close(cd);
    return 0;
}
std::string gb_utf_conv(std::string& in, const char* fromEncode, const char* toEncode)
{
    // char* inbuf = (char*)in.c_str();
    int inlen = strlen(in.c_str());
    // always alloc 3 times size array, lazy to check the real bytes there are.
    int outlen = inlen * 3; // in case unicode 3 times than ascii
    char outbuf[outlen] = { 0 };
    int rst = code_convert(fromEncode, toEncode, (char*)in.c_str(), inlen, outbuf, outlen);
    return rst == 0 ? std::string(outbuf) : in;
}

#else
#include <Windows.h>

std::string gb_utf_conv(std::string& in, int fromCodePage, int toCodePage)
{
    // basically decode source string with specific code page, make it wchar_t
    // and then encode wchar_t with the code page we need, make it string again.

    // empty string no needs conversion.
    if (in.empty())
        return in;
    
    int len = MultiByteToWideChar(fromCodePage, 0, in.c_str(), -1, NULL, 0);
    std::wstring wstr(len + 1, '\0');
    MultiByteToWideChar(fromCodePage, 0, in.c_str(), -1, wstr.data(), len);

    len = WideCharToMultiByte(toCodePage, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string str(len + 1, '\0');
    WideCharToMultiByte(toCodePage, 0, wstr.c_str(), -1, str.data(), len, NULL, NULL);

    // ini like map<string, string> may not store '\0'
    // as we will "oss << gb_utf_conv(pair.first, ...) << '=' << ..."
    // the stream appending may got '\0' into buffer. 
    if (auto nil = str.find('\0'))
        str.resize(nil);
    return str;
}
#endif

bool IsTextUTF8(const std::string& str)
{
    // a better version, see https://www.tutorialspoint.com/utf-8-validation-in-cplusplus.
    if (str.empty())
        return false;

    int cnt = 0;
    // Since FA2sp is Windows Native with GBK default code page,
    // I'd consider to shorten the validating process, and avoid extra (utf8)ASCII -> (gbk)ASCII stuff.
    bool isASCII = true;

    for (int i = 0; i < str.length(); i++)
    {
        int chr = str[i];

        if ((chr & 0x80) == 0)
            continue;
        isASCII = false;

        if (!cnt)
        {
            if ((chr >> 5) == 0b110)        cnt = 1;
            else if ((chr >> 4) == 0b1110)  cnt = 2;
            else if ((chr >> 3) == 0b11110) cnt = 3;
            else if ((chr >> 7) != 0)       return false;
            else {
                if ((chr >> 6) != 0b10)     return false;
                cnt--;
            }
        }
    }

    return !isASCII && cnt == 0;
}
