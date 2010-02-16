//logMessage.h
#include <windows.h>
#include "MessageFile.h"

extern "C" {
	DWORD logcMessage(WORD wType, LPCSTR lMessage);
	DWORD logMessage(WORD wType, LPCWSTR lMessage);
}