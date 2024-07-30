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

    int len = MultiByteToWideChar(fromCodePage, 0, in.c_str(), -1, NULL, 0);
    std::wstring wstr(len + 1, '\0');
    MultiByteToWideChar(fromCodePage, 0, in.c_str(), -1, wstr.data(), len);

    len = WideCharToMultiByte(toCodePage, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string str(len + 1, '\0');
    WideCharToMultiByte(toCodePage, 0, wstr.c_str(), -1, str.data(), len, NULL, NULL);

    return str;
}
#endif

bool IsTextUTF8(const std::string& str)
{
    char nBytes = 0;
    unsigned char chr;
    bool bAllAscii = true;

    for (int i = 0; i < str.length(); i++)
    {
        chr = str[i];

        if (bAllAscii && (chr & 0x80) != 0)
            bAllAscii = false;

        if (nBytes == 0)
        {
            if (chr >= 0x80)
            {
                if (chr >= 0xFC && chr <= 0xFD)   nBytes = 6;
                else if (chr >= 0xF8)         nBytes = 5;
                else if (chr >= 0xF0)         nBytes = 4;
                else if (chr >= 0xE0)         nBytes = 3;
                else if (chr >= 0xC0)         nBytes = 2;
                else {
                    return false;
                }
                nBytes--;
            }
        }
        else
        {
            if ((chr & 0xC0) != 0x80) {
                return false;
            }
            nBytes--;
        }
    }

    if (nBytes > 0)
        return false;

    if (bAllAscii)
        return false;

    return true;
}
