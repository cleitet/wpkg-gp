//logMessage.h
#include <windows.h>
#include "MessageFile.h"

extern "C" {
	DWORD logMessage(WORD wType, LPCSTR lMessage);
}