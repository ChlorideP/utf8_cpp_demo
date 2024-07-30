#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <map>
#include <Windows.h>

#include "codec.h"


// the encoding stuff to be implemented in FA2sp.
// just test in linux.
//   edit: windows would only rely on WINAPI.
//   and we need a checker to avoid (utf8)gbk -> gbk, which leads to error.

int main(int argc, const char* argv[]) {
    using namespace std;
    ifstream fin;
    fin.open("./eg_u8.map", ios::in);
    if (fin.bad())
        return 1;

    ofstream fout;
    ostringstream oss;
    fout.open("./eg_ansi.map", ios::out | ios::trunc);
    if (!fout.is_open() || fout.bad())
        return 2;

    while (!fin.eof()) {
        string ssks;     // so actually no need `wstring`, since utf-8 is mutable encodes.
        getline(fin, ssks);
        ssks.erase(
            find_if(
                ssks.begin(), ssks.end(),
                [](const char c) { return c == '\r' || c == '\n' || c == ';'; }),
            ssks.end());
        if (ssks.empty())
            continue;
        // in real FA2sp, I prefer to convert specific strings possible in Chinese,
        // like value of "Name=", and "[Triggers]" "[Tags]" "[AITriggerTypes]" section values.
        // but this repo is just a demo, let's just make it easier.
        if (IsTextUTF8(ssks))
#ifndef _WIN32
            ssks = gb_utf_conv(ssks, "utf-8", "gbk");  // linux ver.
#else
            ssks = gb_utf_conv(ssks, CP_UTF8, CP_ACP);
#endif
        oss << ssks << endl;
    }

    fout << oss.str();
    fout.flush();
    fout.close();

    return 0;
}