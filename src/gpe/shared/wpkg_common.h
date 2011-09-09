#include <Windows.h>
#include <userenv.h>
#include <wchar.h>

#define BUFSIZE 2048

#define LOG_INFO    1
#define LOG_ERROR   2
#define NO_LOG		3
#define GPE 1
#define EXE 2

extern PFNSTATUSMESSAGECALLBACK gStatusCallback;
void UpdateStatus(int status_type, wchar_t* message, int errorCode);
DWORD executeWpkgViaPipe(int called_by, bool debug_flag);
void debug(const wchar_t* debugMessage, ...);
