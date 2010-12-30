#include <windows.h>
#include <UserEnv.h>
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

	//Checking flags:
	if (!(dwFlags & GPO_INFO_FLAG_MACHINE) || (dwFlags & (GPO_INFO_FLAG_ASYNC_FOREGROUND | GPO_INFO_FLAG_BACKGROUND | GPO_INFO_FLAG_SAFEMODE_BOOT))){
		UpdateStatus(LOG_ERROR, L"Group Policy Extention called with wrong flags", NULL);
		return 300; //Wrong flags set
	}

	gStatusCallback = pStatusCallback;

	return(executeWpkgViaPipe(GPE, false));
}