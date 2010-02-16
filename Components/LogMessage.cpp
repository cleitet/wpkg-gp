// wpkg-gp-tester.cpp
#include "LogMessage.h"

// This must exist as a key under
// HKLM\System\CurrentControlSet\Services\EventLog\Application\WPKG-gp
//   EventMessageFile REG_SZ "\Path\To\dll/exe/containing/messages"
//   TypesSupported REG_DWORD 0x0007
//
// (See http://msdn.microsoft.com/en-us/library/aa363661%28VS.85%29.aspx)
//
// Make installer create this key and set correct path to message file
#define PROVIDER_NAME L"WPKG-gp"


// For more info on logging, see:
// - http://www.codeproject.com/KB/system/CEventLog.aspx
// - http://www.codeproject.com/KB/system/mctutorial.aspx
// - http://msdn.microsoft.com/en-us/library/aa363652%28VS.85%29.aspx
// - http://msdn.microsoft.com/en-us/library/ms810429.aspx


extern "C"
DWORD logcMessage(WORD wType, LPCSTR lMessage)
{
	// Convert to a wchar_t*
    size_t origsize = strlen(lMessage) + 1;
    const size_t newsize = 100;
    size_t convertedChars = 0;
    wchar_t wcstring[newsize];
    mbstowcs_s(&convertedChars, wcstring, origsize, lMessage, _TRUNCATE);
	return logMessage(wType, wcstring);
}

extern "C"
DWORD logMessage(WORD wType, LPCWSTR lMessage)
{
	HANDLE hEventLog = RegisterEventSource(NULL, PROVIDER_NAME);
    LPCWSTR pInsertStrings[1] = {lMessage};
    DWORD dwEventDataSize = 0;

	if (hEventLog == NULL)
    {
		// Unable to Register Event Source
		if (hEventLog){
			DeregisterEventSource(hEventLog);
		}
		return 1;
    }

	// See: http://msdn.microsoft.com/en-us/library/aa363679%28VS.85%29.aspx
	if (!ReportEvent(hEventLog, wType, NULL, EVMSG_STDMSG, NULL, 1, 0, (LPCWSTR*)pInsertStrings, NULL))
    {
        //"ReportEvent failed"
        if (hEventLog){
			DeregisterEventSource(hEventLog);
		}
		return GetLastError();
    }

	return 0;
}