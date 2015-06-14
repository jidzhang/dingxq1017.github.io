#ifndef STRING_H__
#define STRING_H__

BOOL WCharToMByte(const wchar_t * lpcwszStr, char **lpszStr);
DWORD MByteToWChar(const char * lpcszStr, wchar_t ** lpwszStr);

CString ToCString(const std::string & str);
std::string ToString(CString str);

#endif