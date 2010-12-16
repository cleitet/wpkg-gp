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
#define PROVIDER_NAME "WPKG-gp"


// For more info on logging, see:
// - http://www.codeproject.com/KB/system/CEventLog.aspx
// - http://www.codeproject.com/KB/system/mctutorial.aspx
// - http://msdn.microsoft.com/en-us/library/aa363652%28VS.85%29.aspx
// - http://msdn.microsoft.com/en-us/library/ms810429.aspx

extern "C"
DWORD logMessage(WORD wType, LPCSTR cMessage)
{
	HANDLE hEventLog = RegisterEventSourceA(NULL, PROVIDER_NAME);
    LPCSTR pInsertStrings[1] = {cMessage};
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
	if (!ReportEventA(hEventLog, wType, NULL, EVMSG_STDMSG, NULL, 1, 0, pInsertStrings, NULL))
    {
        //"ReportEvent failed"
        if (hEventLog){
			DeregisterEventSource(hEventLog);
		}
		return GetLastError();
    }

	return 0;
}