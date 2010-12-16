// wpkg-gp-test.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <userenv.h>
#include <winbase.h>
#include <stdio.h>
#define REG_KEY   L"SOFTWARE\\WPKG-gp"
#define REG_VALUE L"WPKGClientExecuteString"
#define BUFSIZE   512
#define bzero(b, len) (memset((b), '\0', (len)), (void) 0)

int main(int argc, char* argv[])
{
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

	while (ReadFile(pipe_stdout_read, cBuff, BUFSIZE-1, &dwRead, NULL)){


		MultiByteToWideChar( CP_UTF8, 0, cBuff, -1, wBuff, BUFSIZE);
		printf("%S", wBuff);
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

