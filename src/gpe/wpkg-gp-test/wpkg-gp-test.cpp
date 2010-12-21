// wpkg-gp-test.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <userenv.h>
#include <winbase.h>
#include <stdio.h>

#define BUFSIZE   512
#define bzero(b, len) (memset((b), '\0', (len)), (void) 0)

int main(int argc, char* argv[])
{

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
		bzero(chTempBuf, BUFSIZE);

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

		printf( "%S\n", wcBuf );
	}

	CloseHandle(hPipe);
	return( ERROR_SUCCESS );

	/*

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

	HANDLE pipe_stdout_read, pipe_stdout_write;
	HANDLE pipe_stdin_read, pipe_stdin_write;
	HANDLE pipe_stderr_read, pipe_stderr_write;

	// Creating pipes
	if (!CreatePipe(&pipe_stdout_read, &pipe_stdout_write, &sa, 1))
		return 205; //Unable to CreatePipe
	if (!CreatePipe(&pipe_stdin_read, &pipe_stdin_write, &sa, 1))
		return 205; //Unable to CreatePipe
	if (!CreatePipe(&pipe_stderr_read, &pipe_stderr_write, &sa, 1))
		return 205; //Unable to CreatePipe

	STARTUPINFOW si;
	GetStartupInfoW(&si);
	si.dwFlags = STARTF_USESTDHANDLES;
	
	si.hStdOutput = pipe_stdout_write;
	si.hStdError  = pipe_stderr_write;
	si.hStdInput  = pipe_stdin_read;

	//Create process
	if (!CreateProcessW(NULL, (wchar_t *) lpValue,NULL,NULL,TRUE, DETACHED_PROCESS, NULL,NULL,&si,&pi))
		return 206; //Unable to CreateProcess

	char cBuff[BUFSIZE];
	wchar_t wBuff[BUFSIZE];
	bzero(cBuff, BUFSIZE);
	bzero(wBuff, BUFSIZE);
	DWORD dwRead = 0;

	while (ReadFile(pipe_stdout_read, cBuff, BUFSIZE - 1, &dwRead, NULL)){
		printf("%i",  dwRead);
		
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
	CloseHandle(pipe_stdout_write);
	CloseHandle(pipe_stdin_read);
	CloseHandle(pipe_stderr_write);
	*/
}

