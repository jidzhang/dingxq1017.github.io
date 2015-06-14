#include "StdAfx.h"
#include "String.h"

BOOL WCharToMByte(const wchar_t * lpcwszStr, char **lpszStr)
{
	DWORD dwMinSize = WideCharToMultiByte(CP_OEMCP, NULL, lpcwszStr, -1, NULL, 0, NULL, FALSE);
	if (dwMinSize > 0) {
		char *sText = new char[dwMinSize];
		if (!sText) {
			delete [] sText; sText = NULL;
			dwMinSize = 0;
		}
		if (dwMinSize > 0) {
			WideCharToMultiByte(CP_OEMCP, NULL, lpcwszStr, -1, sText, dwMinSize, NULL, FALSE);
			if (lpszStr)
				*lpszStr = sText;
		}
		return TRUE;
	}
	return FALSE;
}

DWORD MByteToWChar(const char * lpcszStr, wchar_t ** lpwszStr)
{
	DWORD dwMinSize = MultiByteToWideChar(CP_ACP, 0, lpcszStr, -1, NULL, 0);
	if (dwMinSize > 0) {
		wchar_t * pwText = new wchar_t[dwMinSize];
		if (!pwText) {
			delete [] pwText; pwText = NULL;
			dwMinSize = 0;
		}
		if (dwMinSize > 0) {
			MultiByteToWideChar(CP_ACP, 0, lpcszStr, -1, pwText, dwMinSize);
			if (lpwszStr)
				*lpwszStr = pwText;
		}
	}
	return dwMinSize;
}

CString ToCString(const std::string & str)
{
	CString strRe;
#if _UNICODE
	strRe = str.c_str();
#else
	wchar_t * ch = NULL;
	MByteToWChar(str.c_str(), &ch);
	if (ch)
		strRe = ch;
	delete ch; ch = NULL;
#endif
	return strRe;
}

std::string ToString(CString str)
{
	std::string strRe;
	CString strSrc(str);
#if _UNICODE
	char *ch = NULL;
	WCharToMByte(strSrc.GetBuffer(strSrc.GetLength()+1), &ch);
	if (ch)
		strRe = ch;
	delete ch; ch = NULL;
	strSrc.ReleaseBuffer(strSrc.GetLength()+1);
#else
	strRe = strSrc.GetBuffer(strSrc.GetLength()+1);
	strSrc.ReleaseBuffer(strSrc.GetLength()+1);
#endif
	return strRe;
}
