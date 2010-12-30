#include <Windows.h>
#include <userenv.h>
#define BUFSIZE 512
#define bzero(b, len) (memset((b), '\0', (len)), (void) 0)

#define LOG_INFO    1
#define LOG_ERROR   2
#define GPE 1
#define EXE 2

extern PFNSTATUSMESSAGECALLBACK gStatusCallback;
void UpdateStatus(int status_type, wchar_t* message, int errorCode);
DWORD executeWpkgViaPipe(int called_by, bool debug_flag);