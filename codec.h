#include <string>

// needs string conversion.

// int code_convert(char* from_charset, char* to_charset, char* inbuf, size_t inlen, char* outbuf, size_t outlen);
#ifndef _WIN32
std::string gb_utf_conv(std::string& in, const char* fromEncode, const char* toEncode);
#else
std::string gb_utf_conv(std::string& in, int fromCodePage, int toCodePage);
#endif

bool IsTextUTF8(const std::string& str);
