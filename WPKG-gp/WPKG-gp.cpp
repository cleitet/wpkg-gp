#include "stdafx.h"
#include <string>
#include <windows.h>
#include <userenv.h>
#include <winbase.h>
#include "wpkg-gp.h"
#include "..\Components\LogMessage.h"
#include "..\Components\LuaFunctions.h"


PFNSTATUSMESSAGECALLBACK gStatusCallback;

//GUID = A9B8D792-F454-11DE-BA92-FDCF56D89593

// Running as NT AUTHORITY\SYSTEM
DWORD CALLBACK ProcessGroupPolicy(
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

   //TODO: Check flags

   // Process deleted GPOs.

   // Process changed GPOs.

	// Raise htoken rights
	TOKEN_PRIVILEGES tp;
	LUID luid;
	

	/*
	if( !LookupPrivilegeValue(NULL, L"SeShutdownPrivilege", &luid) ){
		std::wstring sErrorMessage;
		int error;
		LPWSTR lpErrorString;
		error = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			0, error, NULL, (LPWSTR)&lpErrorString, 0, NULL);
		
		sErrorMessage += L"Error calling LookupPrivilegeValue(): ";
		sErrorMessage += lpErrorString;

		logMessage(EVENTLOG_ERROR_TYPE, sErrorMessage.data());
		LocalFree(lpErrorString);
		return ERROR_FUNCTION_FAILED;
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if( !AdjustTokenPrivileges(hToken, 
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES) NULL,
		(PDWORD) NULL)
		)
	{
		//Log errors and exit
		std::wstring sErrorMessage;
		int error;
		LPWSTR lpErrorString;
		error = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			0, error, NULL, (LPWSTR)&lpErrorString, 0, NULL);
		
		sErrorMessage += L"Error calling AdjustPrivileges(): ";
		sErrorMessage += lpErrorString;

		logMessage(EVENTLOG_ERROR_TYPE, sErrorMessage.data());
		LocalFree(lpErrorString);
		return ERROR_FUNCTION_FAILED;
	}*/

	int executelua = runLua("gpe", pStatusCallback);

	if(!InitiateSystemShutdownEx(
		NULL, NULL, 0, TRUE, TRUE, 
		SHTDN_REASON_MAJOR_APPLICATION | 
		SHTDN_REASON_MINOR_INSTALLATION | 
		SHTDN_REASON_FLAG_PLANNED ))
	{
		int error = GetLastError();
		LPWSTR lpErrorString;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			0, error, NULL, (LPWSTR)&lpErrorString, 0, NULL);
		logMessage(EVENTLOG_ERROR_TYPE, lpErrorString);
		LocalFree(lpErrorString);
	}

	return( ERROR_SUCCESS ); 

}