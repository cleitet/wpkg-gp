#include <windows.h>
#include <UserEnv.h>
#include <wchar.h>
#include "wpkg-gp.h"
#include "wpkg_common.h"

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

	// Reading debug settings from registry
	HKEY hKey = NULL;
	DWORD dwDataType = REG_SZ;
	DWORD dwSize = 0;
	LPBYTE lpValue   = NULL;

	BOOL DEBUG = false;
	
	LONG lRet = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Policies\\WPKG_GP", 0, KEY_QUERY_VALUE, &hKey);
	lRet = RegQueryValueExA(hKey, "WpkgVerbosity", 0, &dwDataType, lpValue, &dwSize); // dwSize will contain the data size
	// Allocate the buffer
	lpValue = (LPBYTE) malloc(dwSize + 1);
	lRet = RegQueryValueExA(hKey, "WpkgVerbosity", 0, &dwDataType, lpValue, &dwSize);
	RegCloseKey(hKey);
	// Adding null termination
	lpValue[dwSize] = '\0';
	if (atoi((LPSTR) lpValue) >= 3){
		DEBUG=true;
	}
	free(lpValue);

	//Checking flags:
	if (DEBUG){
		UpdateStatus(LOG_INFO, L"Debug mode on", NULL);
		if (dwFlags & GPO_INFO_FLAG_MACHINE){
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_MACHINE on", NULL);
		} else {
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_MACHINE off", NULL);
		}
		if (dwFlags & GPO_INFO_FLAG_BACKGROUND){
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_BACKGROUND on", NULL);
		} else {
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_BACKGROUND off", NULL);
		}
		if (dwFlags & GPO_INFO_FLAG_ASYNC_FOREGROUND){
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_ASYNC_FOREGROUND on", NULL);
		} else {
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_ASYNC_FOREGROUND off", NULL);
		}
		if (dwFlags & GPO_INFO_FLAG_SLOWLINK){
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_SLOWLINK on", NULL);
		} else {
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_SLOWLINK off", NULL);
		}
		if (dwFlags & GPO_INFO_FLAG_VERBOSE){
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_VERBOSE on", NULL);
		} else {
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_VERBOSE off", NULL);
		}
		if (dwFlags & GPO_INFO_FLAG_NOCHANGES){
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_NOCHANGES on", NULL);
		} else {
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_NOCHANGES off", NULL);
		}
		if (dwFlags & GPO_INFO_FLAG_LINKTRANSITION){
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_LINKTRANSITION on", NULL);
		} else {
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_LINKTRANSITION off", NULL);
		}
		if (dwFlags & GPO_INFO_FLAG_FORCED_REFRESH){
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_FORCED_REFRESH on", NULL);
		} else {
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_FORCED_REFRESH off", NULL);
		}
		if (dwFlags & GPO_INFO_FLAG_SAFEMODE_BOOT){
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_SAFEMODE_BOOT on", NULL);
		} else {
			UpdateStatus(LOG_INFO, L"GPO_INFO_FLAG_SAFEMODE_BOOT off", NULL);
		}
		if (pStatusCallback != NULL){
			UpdateStatus(LOG_INFO, L"pStatusCallback is not NULL", NULL);
		} else {
			UpdateStatus(LOG_INFO, L"pStatusCallback is NULL", NULL);
		}
	}
	gStatusCallback = pStatusCallback;

	if (!(dwFlags & GPO_INFO_FLAG_MACHINE) || (dwFlags & (GPO_INFO_FLAG_ASYNC_FOREGROUND | GPO_INFO_FLAG_BACKGROUND | GPO_INFO_FLAG_SAFEMODE_BOOT))){
		if (dwFlags & GPO_INFO_FLAG_SAFEMODE_BOOT){
			UpdateStatus(LOG_INFO, L"WPKG-GP Group Policy Extension called with GPO_INFO_FLAG_SAFEMODE_BOOT set. WPKG-GP WPKG-GP will not execute.", NULL);
			return 0;
		}
		if (dwFlags & GPO_INFO_FLAG_ASYNC_FOREGROUND){
			//"HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\Windows NT\CurrentVersion\Winlogon" with registry value "SyncForegroundPolicy"=dword:00000001
			//Registry: "HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\Windows NT\CurrentVersion\Winlogon" with registry value "SyncForegroundPolicy"=dword:00000001
			UpdateStatus(LOG_ERROR, L"WPKG-GP Group Policy Extension called with GPO_INFO_FLAG_ASYNC_FOREGROUND set. WPKG-GP can only be applied synchronously. Please configure your system to apply Group Policies synchronously.", NULL);
		}
		if (dwFlags & GPO_INFO_FLAG_BACKGROUND){
			// You have to mess with the settings from the installer to make this happen
			UpdateStatus(LOG_ERROR, L"WPKG-GP Group Policy Extension called with GPO_INFO_FLAG_BACKGROUND set. WPKG-GP can only be applied in the foreground. Please configure your system to apply WPKG-GP in the foreground only.", NULL);
		}
		if (!(dwFlags & GPO_INFO_FLAG_MACHINE)){
			// You have to mess with the settings from the installer to make this happen
			UpdateStatus(LOG_ERROR, L"WPKG-GP Group Policy Extension called without GPO_INFO_FLAG_MACHINE set. WPKG-GP can only be applied in the foreground. Please configure your system to apply WPKG-GP Policy for computers only.", NULL);
		}
		
		UpdateStatus(LOG_INFO, L"WPKG-GP Group Policy Extension called with wrong flags", NULL);
		return 300; //Wrong flags set
	}
	return(executeWpkgViaPipe(GPE, DEBUG));
}