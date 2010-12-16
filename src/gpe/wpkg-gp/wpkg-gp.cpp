#include <windows.h>
#include <userenv.h>
#include <winbase.h>
#include <stdio.h>
#include "wpkg-gp.h"

#define REG_KEY   L"SOFTWARE\\WPKG-gp"
#define REG_VALUE L"WPKGClientExecuteString"
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

	//Read path to executable from registry
	HKEY hKey;
	LONG lRet;
	DWORD  dwDataType = REG_SZ;
	DWORD  dwSize     = 0;
	LPBYTE lpValue = NULL;

	lRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE, REG_KEY, 0, KEY_QUERY_VALUE, &hKey);
	if (lRet != ERROR_SUCCESS){
		return 201; //Could not open key
	}
	// Call first RegQueryValueEx to retrieve the necessary buffer size
	// dwSize will contain the data size
	lRet = RegQueryValueExW(hKey, REG_VALUE, 0, &dwDataType, lpValue, &dwSize);
	if (lRet != ERROR_SUCCESS){
		return 202; //Could not read value
	}
	// Allocate the buffer
	lpValue = (LPBYTE) malloc(dwSize + 1); // Add a byte for the Null termination
	lRet = RegQueryValueExW(hKey, REG_VALUE, 0, &dwDataType, lpValue, &dwSize);
	RegCloseKey(hKey);
	// Adding null termination to lpValue
	lpValue[dwSize] = '\0';

	SECURITY_ATTRIBUTES sa;
	PROCESS_INFORMATION pi;
	
	SECURITY_DESCRIPTOR sd; //security information for pipes
	if (!InitializeSecurityDescriptor(&sd,SECURITY_DESCRIPTOR_REVISION))
		return 203; //Unable to InitializeSecurityDescriptor
	if (!SetSecurityDescriptorDacl(&sd, true, NULL, false))
		return 204; //Could not set Security Descriptor
    sa.lpSecurityDescriptor = &sd;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = true;         //allow inheritable handles

	HANDLE pipe_stdin_read, pipe_stdin_write;  //pipe handles
	HANDLE pipe_stdout_read, pipe_stdout_write;
	HANDLE pipe_stderr_read, pipe_stderr_write;
	// Creating pipes
	if (!CreatePipe(&pipe_stdin_read, &pipe_stdin_write, &sa, 1))
		return 205; //Unable to CreatePipe
	if (!CreatePipe(&pipe_stdout_read, &pipe_stdout_write, &sa, 1))
		return 205; //Unable to CreatePipe
	if (!CreatePipe(&pipe_stderr_read, &pipe_stderr_write, &sa, 1))
		return 205; //Unable to CreatePipe

	STARTUPINFOW si;
	GetStartupInfoW(&si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = pipe_stdout_write;
	si.hStdError = pipe_stderr_write;
	si.hStdInput = pipe_stdin_read;
	
	//Create process
	if (!CreateProcessW(NULL, (wchar_t *) lpValue,NULL,NULL,TRUE, DETACHED_PROCESS, NULL,NULL,&si,&pi))
		return 206; //Unable to CreateProcess
	CloseHandle(pipe_stdout_write);
	CloseHandle(pipe_stderr_write);
	CloseHandle(pipe_stdin_read);

	
	char cBuff[BUFSIZE];
	wchar_t wBuff[BUFSIZE];
	bzero(cBuff, BUFSIZE);
	bzero(wBuff, BUFSIZE);
	DWORD dwRead = 0;

	while (ReadFile(pipe_stdout_read, cBuff, BUFSIZE - 1, &dwRead, NULL)){
		//remove newline
		cBuff[dwRead-2] = NULL; //Windows has CR and LF, 2 bytes.
		MultiByteToWideChar( CP_UTF8, 0, cBuff, -1, wBuff, BUFSIZE);
		pStatusCallback(FALSE, wBuff);
		bzero(wBuff, BUFSIZE);
		bzero(cBuff, BUFSIZE);
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	CloseHandle(pipe_stdin_write);
	CloseHandle(pipe_stdout_read);
	CloseHandle(pipe_stderr_read);


	return( ERROR_SUCCESS );
}