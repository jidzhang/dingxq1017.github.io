#ifndef STRING_H__
#define STRING_H__

BOOL WCharToMByte(const wchar_t * lpcwszStr, char **lpszStr);
DWORD MByteToWChar(const char * lpcszStr, wchar_t ** lpwszStr);

CString String2CString(const std::string & str);
std::string CString2String(const CString & str);

#endif