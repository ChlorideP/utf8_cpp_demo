#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <Windows.h>

#include "codec.h"


// the encoding stuff to be implemented in FA2sp.
// just test in linux.
//   edit: windows would only rely on WINAPI.
//   and we need a checker to avoid (utf8)gbk -> gbk, which leads to error.

int main(int argc, const char* argv[]) {
    using namespace std;
    ifstream fin;
    fin.open("../newall02 - u8.map", ios::in);
    if (!fin.is_open() || fin.bad())
        return 1;

    ofstream fout;
    ostringstream oss;
    fout.open("./eg_utf-8.map", ios::out | ios::trunc);
    if (!fout.is_open() || fout.bad())
        return 2;

    while (!fin.eof()) {
        string ssks;     // so actually no need `wstring`, since utf-8 is mutable encodes.
        getline(fin, ssks, '\n');
        ssks.erase(
            find_if(
                ssks.begin(), ssks.end(),
                [](const char c) { return c == '\r' || c == '\n' || c == ';'; }),
            ssks.end());
        if (IsTextUTF8(ssks))
            ssks = gb_utf_conv(ssks, CP_UTF8, CP_ACP);
        oss << ssks << endl;
    }

    fout << oss.str();
    fout.flush();
    fout.close();

    return 0;
}