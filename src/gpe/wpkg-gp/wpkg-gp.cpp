#include <windows.h>
#include <userenv.h>
#include <winbase.h>
#include <stdio.h>
#include "wpkg-gp.h"

#define BUFSIZE 512
#define bzero(b, len) (memset((b), '\0', (len)), (void) 0)

//GUID = A9B8D792-F454-11DE-BA92-FDCF56D89593
// Running as NT AUTHORITY\SYSTEM
extern "C" __declspec( dllexport ) DWORD CALLBACK ProcessGroupPolicy(
  DWORD dwFlags,
  HANDLE hToken,
  HKEY hKeyRoot,
  PGROUP_POLICY_OBJECT pDeletedGPOList,
  PGROUP_POLICY_OBJECT pChangedGPOList,
  ASYNCCOMPLETIONHANDLE pHandle,
  BOOL *pbAbort,
  PFNSTATUSMESSAGECALLBACK pStatusCallback
)

{
   //PGROUP_POLICY_OBJECT 
   // Check dwFlags for settings.

   // Process only when the following flags are set:
   // GPO_INFO_FLAG_MACHINE: Apply computer policy rather than user policy.
   //
   // And the following flags are not set:
   // GPO_INFO_FLAG_ASYNC_FOREGROUND: Perform an asynchronous foreground refresh
   //   of policy. (This must happen SYNCHRONOUS, as we do not want the user to be
   //   able to log on before WPKG is finished) (NOTE: The GPO timeout is 60 minutes
   //   (http://msdn.microsoft.com/en-us/library/aa374304%28VS.85%29.aspx), and it
   //   is not possible to modify this)
   // GPO_INFO_FLAG_BACKGROUND: Perform a background refresh of the policy.
   // GPO_INFO_FLAG_SAFEMODE_BOOT: Computer is starting in safe mode

	//Checking flags:
	if (!(dwFlags & GPO_INFO_FLAG_MACHINE) || (dwFlags & (GPO_INFO_FLAG_ASYNC_FOREGROUND | GPO_INFO_FLAG_BACKGROUND | GPO_INFO_FLAG_SAFEMODE_BOOT)))
		return 300; //Wrong flags set

	HANDLE hPipe;
	LPTSTR lpvMessage="Execute";
	TCHAR  chBuf[BUFSIZE];
	TCHAR  chTempBuf[BUFSIZE];
	WCHAR  wcBuf[BUFSIZE * sizeof(wchar_t)];
	BOOL   fSuccess = FALSE;
	DWORD  cbRead, cbToWrite, cbWritten, dwMode;
	LPTSTR lpszPipename = "\\\\.\\pipe\\WPKG";

	// Try to open a named pipe; wait for it, if necessary.
	while (1) {
		hPipe = CreateFile( 
			lpszPipename,   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE, 
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file
		if (hPipe != INVALID_HANDLE_VALUE)
			break;
		// Exit if an error other than ERROR_PIPE_BUSY occurs.
		if (GetLastError() != ERROR_PIPE_BUSY)
			return 204;
		// All pipe instances are busy, so wait for 20 seconds.
		if (!WaitNamedPipe(lpszPipename, 20000))
			return 205;
	}

	// The pipe connected; change to message-read mode. 
	dwMode = PIPE_READMODE_MESSAGE;
	fSuccess = SetNamedPipeHandleState(
		hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time
	if (!fSuccess)
		return 206;

	// Send a message to the pipe server.
	cbToWrite = (lstrlen(lpvMessage)+1)*sizeof(TCHAR);
	fSuccess = WriteFile( 
		hPipe,                  // pipe handle 
		lpvMessage,             // message 
		cbToWrite,              // message length 
		&cbWritten,             // bytes written 
		NULL);                  // not overlapped 
	
	if (!fSuccess)
		return 207;

	while (1) { 
		bzero(chBuf, BUFSIZE);
		bzero(wcBuf, BUFSIZE * sizeof(wchar_t));

		// Read from the pipe. 
		fSuccess = ReadFile( 
			hPipe,    // pipe handle 
			chBuf,    // buffer to receive reply 
			BUFSIZE*sizeof(TCHAR) - 1,  // size of buffer 
			&cbRead,  // number of bytes read 
			NULL);    // not overlapped 

		if (GetLastError() == ERROR_HANDLE_EOF)
			break;

		if ( !fSuccess && GetLastError() != ERROR_MORE_DATA )
			return 208;
		
		// Remove 4 first characters
		int start = 4;
		int i;
		int j = 0;
		for (i=start; chBuf[i]!='\0'; i++){
			chTempBuf[j] = chBuf[i];
			j++;
		}


		MultiByteToWideChar( CP_UTF8, 0, chTempBuf, -1, wcBuf, BUFSIZE * sizeof(WCHAR) -1);

		pStatusCallback(FALSE, wcBuf);
	}

	CloseHandle(hPipe);
	return( ERROR_SUCCESS );
}