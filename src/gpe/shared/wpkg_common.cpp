#include <windows.h>
#include <userenv.h>
#include <winbase.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "wpkg_common.h"
#include "MessageFile.h"

// This must exist as a key under
// HKLM\System\CurrentControlSet\Services\EventLog\Application\WPKG-gp-GPE
//   EventMessageFile REG_SZ "\Path\To\dll/exe/containing/messages"
//   TypesSupported REG_DWORD 0x0007
//
// (See http://msdn.microsoft.com/en-us/library/aa363661%28VS.85%29.aspx)
// For more info on logging, see:
// - 
// - http://www.codeproject.com/KB/system/mctutorial.aspx
// - http://msdn.microsoft.com/en-us/library/aa363652%28VS.85%29.aspx
// - http://msdn.microsoft.com/en-us/library/ms810429.aspx
// Make installer create this key and set correct path to message file
#define PROVIDER_NAME L"WPKG-gp-GPE"


//Globals
bool DEBUG = TRUE;
int EXECUTE_FROM_GPE = FALSE;
int EXECUTE_FROM_EXE = FALSE;
PFNSTATUSMESSAGECALLBACK gStatusCallback = NULL;
FILE *debugfh = NULL;

void debug(const wchar_t* format, ...)
{
	wchar_t tmpbuf[128];
	time_t rawtime;
	struct tm timeinfo;
	va_list valist;

	if (!DEBUG)
		return;

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	wcsftime(tmpbuf, 128, L"%Y-%m-%d %H:%M:%S ", &timeinfo);

	va_start(valist, format);
	if (!EXECUTE_FROM_GPE){
		fwprintf_s(stdout, L"%ls", tmpbuf);
		vfwprintf_s(stdout, format, valist);
	}
	if (debugfh != NULL) {
		fwprintf_s(debugfh, L"%ls", tmpbuf);
		vfwprintf_s(debugfh, format, valist);
		fflush(debugfh);
	}
	va_end(valist);
}

DWORD logMessage(WORD wType, LPCWSTR message)
{
	debug(L"Starting RegisterEventSourceW\n");
	HANDLE hEventLog = RegisterEventSourceW(NULL, PROVIDER_NAME);
	LPCWSTR pInsertStrings[1] = {message};
	DWORD dwEventDataSize = 0;

	if (hEventLog == NULL)
		return FALSE;

	debug(L"Starting ReportEventW\n");
	if (!ReportEventW(hEventLog, wType, NULL, EVMSG_STDMSG, NULL, 1, 0, pInsertStrings, NULL)) {
		DeregisterEventSource(hEventLog);
		return FALSE;
	}
	DeregisterEventSource(hEventLog);
	return TRUE;
}

void UpdateStatus(int status_type, wchar_t* message, int errorCode)
{
	wchar_t formatedMsg[BUFSIZE];
	wchar_t *prefix = L"Bad";

	if (errorCode) {
		debug(L"Running UpdateStatus with the error code: %i and the message '%ls'\n", errorCode, message);

		wchar_t* wcMsg = NULL;
		int ret = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, errorCode, NULL, (LPWSTR) &wcMsg, 0, NULL);
		if (ret != 0) {
			debug(L"  FormatMessageW was run, value of wcMsg is: %ls", wcMsg);
			swprintf_s(formatedMsg, BUFSIZE, L"%ls\nError code: %i\nError message: %ls", message, errorCode, wcMsg);
			LocalFree(wcMsg);
		}
	} else {
		debug(L"Running UpdateStatus with no error code\n");
		swprintf_s(formatedMsg, BUFSIZE, L"%ls", message);
	}

	int logType;
	if (status_type == LOG_INFO) {
		logType = EVENTLOG_INFORMATION_TYPE;
		prefix = L"Info";
	} else if (status_type == LOG_ERROR) {
		logType = EVENTLOG_ERROR_TYPE;
		prefix = L"Error";
	} else if (status_type == NO_LOG) {
		prefix = L"Info (no event log)";
	}

	debug(L"%ls: %ls\n", prefix, formatedMsg);
	if (status_type != NO_LOG)
		logMessage(logType, formatedMsg);

	if (EXECUTE_FROM_GPE)
		gStatusCallback(FALSE, message);
}

DWORD executeWpkgViaPipe(int called_by, bool debug_flag)
{
	BOOL fSuccess = FALSE;
	int err;

	if (debug_flag) {
		DEBUG = TRUE;
		_wfopen_s(&debugfh, L"c:\\wpkg-gp-debug.log", L"a");
	}

	if (called_by == GPE)
		EXECUTE_FROM_GPE = TRUE;
	else if (called_by == EXE)
		EXECUTE_FROM_EXE = TRUE;

	//Making sure the service has started
	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwBytesNeeded;

	// Get a handle to the SCM database. 
	debug(L"Starting OpenSCManager\n");
	SC_HANDLE schSCManager = OpenSCManager(
		NULL,			// local computer
		NULL,			// servicesActive database
		SC_MANAGER_ALL_ACCESS);	// full access rights 
	if (schSCManager == NULL) {
		err = GetLastError();
		UpdateStatus(LOG_ERROR, L"Error when calling OpenSCManager", err);
		return err;
	}
	debug(L"Started OpenSCManager\n");

	// Get a handle to the service.
	debug(L"Starting OpenService\n");
	SC_HANDLE schService = OpenServiceW( 
		schSCManager,         // SCM database 
		L"WpkgServer",            // name of service 
		SERVICE_ALL_ACCESS);  // full access 
 
	if (schService == NULL) { 
		err = GetLastError();
		CloseServiceHandle(schSCManager);
		UpdateStatus(LOG_ERROR, L"Error when calling OpenService", err);
		return err;
	}
	debug(L"Started OpenService\n");

	// Check the status of the service
	debug(L"Starting QueryServiceStatusEx\n");
	if (!QueryServiceStatusEx( 
			schService,                     // handle to service 
			SC_STATUS_PROCESS_INFO,         // information level
			(LPBYTE) &ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded ) )              // size needed if buffer is too small
	{
		err = GetLastError();
		CloseServiceHandle(schService); 
		CloseServiceHandle(schSCManager);
		UpdateStatus(LOG_ERROR, L"Error when calling QueryServiceStatusEx", err);
		return err; 
	}

	debug(L"Finished QueryServiceStatusEx\n");

	debug(L"Checking the status of the service\n");
	for (int i = 0; ssStatus.dwCurrentState != SERVICE_RUNNING; i++) {
		UpdateStatus(LOG_INFO, L"Waiting for the WPKG-GP software installation service to start.", FALSE);
		debug(L"The current status was not SERVICE_RUNNING: %i, but %i, entering wait loop. i is %i (max 120)\n",
		      SERVICE_RUNNING, ssStatus.dwCurrentState, i);
		//Then wait for the service for maximum 120 seconds
		if (i >= 120) {
			UpdateStatus(LOG_ERROR, L"Service did not start for 120 seconds, quitting.", FALSE);
			return 101; //Timeout
		}
		if (EXECUTE_FROM_EXE) {
			UpdateStatus(LOG_INFO, L"Executed from exe file, will not wait for service to start", FALSE);
			break;
		}
		debug(L" Sleeping 1 second\n");
		Sleep(1000);
		if (!QueryServiceStatusEx( 
				schService,                     // handle to service 
				SC_STATUS_PROCESS_INFO,         // information level
				(LPBYTE) &ssStatus,             // address of structure
				sizeof(SERVICE_STATUS_PROCESS), // size of structure
				&dwBytesNeeded ) )              // size needed if buffer is too small
		{
			err = GetLastError();
			CloseServiceHandle(schService); 
			CloseServiceHandle(schSCManager);
			UpdateStatus(LOG_ERROR, L"Error when calling QueryServiceStatusEx in loop", err);
			return err; 
		}

		debug(L"Checking if service is running, comparing CurrentState: %i to SERVICE_RUNNING: %i\n",
			ssStatus.dwCurrentState, SERVICE_RUNNING);
	}

	CloseServiceHandle(schService); 
	CloseServiceHandle(schSCManager);

	HANDLE hPipe;
	LPCWSTR lpszPipename = L"\\\\.\\pipe\\WPKG";

	// Try to open a named pipe; wait for it, if necessary.
	while (1) {
		debug(L"Trying to run CreateFile on named pipe\n");
		hPipe = CreateFileW( 
			lpszPipename,   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE, 
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file

		// Break the loop if the pipe handle is valid. 
		if (hPipe != INVALID_HANDLE_VALUE) {
			debug(L"The pipe handle was valid, continuing\n");
			break;
		}

		// Exit if an error other than ERROR_PIPE_BUSY occurs.
		err = GetLastError();
		if (err != ERROR_PIPE_BUSY) {
			UpdateStatus(LOG_ERROR, L"CreatePipe returned an error.", err);
			return err;
		}
		// All pipe instances are busy, so wait for 20 seconds.
		debug(L"Waiting in 20 seconds for a free pipe instance\n");
		if (!WaitNamedPipeW(lpszPipename, 20000)){
			err = GetLastError();
			UpdateStatus(LOG_ERROR, L"Pipe server had no free instances for 20 seconds, quitting.", err);
			return err;
		}
	}

	// The pipe connected; change to message-read mode. 
	DWORD dwMode = PIPE_READMODE_MESSAGE;
	debug(L"Executing SetNamedPipeHandleState\n");
	fSuccess = SetNamedPipeHandleState(
			hPipe,    // pipe handle 
			&dwMode,  // new pipe mode 
			NULL,     // don't set maximum bytes 
			NULL);    // don't set maximum time
	if (!fSuccess){
		err = GetLastError();
		UpdateStatus(LOG_ERROR, L"CreatePipe returned an error.", err);
		return 206;
	}

	debug(L"Sending message to pipe server\n");

	// Send a message to the pipe server.
	LPCSTR lpvMessage="ExecuteFromGPE";
	DWORD cbWritten;

	fSuccess = WriteFile(
			hPipe,                  // pipe handle 
			lpvMessage,             // message 
			lstrlenA(lpvMessage)+1, // message length 
			&cbWritten,             // bytes written 
			NULL);                  // not overlapped 
	if (!fSuccess) {
		err = GetLastError();
		UpdateStatus(LOG_ERROR, L"WriteFile to named pipe returned an error.", err);
		return err;
	}

	debug(L"Waiting for reply from pipe server\n");
	while (1) { 
		char chBuf[BUFSIZE];
		wchar_t wcBuf[BUFSIZE];
		BOOL bLog = TRUE;
		DWORD cbRead;

		// Read from the pipe. 
		debug(L"Executing ReadFile on pipe\n");
		fSuccess = ReadFile( 
			hPipe,    // pipe handle 
			chBuf,    // buffer to receive reply 
			sizeof(chBuf),  // size of buffer 
			&cbRead,  // number of bytes read 
			NULL);    // not overlapped

		err = GetLastError();
		if (err == ERROR_HANDLE_EOF) {
			debug(L"Got to EOF, continuing");
			break;
		}
		if (err == ERROR_PIPE_NOT_CONNECTED) {
			debug(L"Other end closed the pipe, continuing\n");
			break;
		}
		if (!fSuccess && err != ERROR_MORE_DATA) {
			UpdateStatus(LOG_ERROR, L"ReadFile from named pipe returned an error.", err);
			return err;
		}
		if (cbRead < 4) {
			debug(L"Short message received %d bytes", cbRead);
			continue;
		}

		debug(L"Successfully read from pipe, stripping status code\n");

		// If status code = 101, do not use EventVwr
		if (memcmp(chBuf, "101", 3) == 0) {
			debug(L"Retrieved Status Code 101, will not print anything to event log\n");
			bLog = false;
		} else {
			debug(L"Retrieved Status Code %3.3S\n", chBuf);
			bLog = true;
		}

		debug(L"Converting string to wchar_t\n");
		cbWritten = MultiByteToWideChar(CP_UTF8, 0, &chBuf[4], cbRead-4, wcBuf, BUFSIZE-1);
		if (cbWritten == 0) {
			err = GetLastError();
			UpdateStatus(LOG_ERROR, L"Conversion of data read from pipe failed.", err);
			return err;
		}
		wcBuf[cbWritten] = 0;

		debug(L"Calling UpdateStatus\n");
		//pStatusCallback(FALSE, wcBuf);
		if (bLog == TRUE) {
			UpdateStatus(LOG_INFO, wcBuf, FALSE);
		} else {
			UpdateStatus(NO_LOG, wcBuf, FALSE);
		}
	}

	CloseHandle(hPipe);
	if (DEBUG)
		if (debugfh != false)
			fclose(debugfh);
	
	return ERROR_SUCCESS;
}
