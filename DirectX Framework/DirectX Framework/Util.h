#pragma once

inline wchar_t* CharToWChar(char *mbString)
{
	int len = 0;
	len = (int)strlen(mbString) + 1;
	wchar_t *ucString = new wchar_t[len];
	mbstowcs(ucString, mbString, len);
	return ucString;
}