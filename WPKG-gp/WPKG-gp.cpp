#include "stdafx.h"
#include <windows.h>
#include <userenv.h>
#include <winbase.h>
#include "wpkg-gp.h"
#include "..\Components\LuaFunctions.h"


PFNSTATUSMESSAGECALLBACK gStatusCallback;

//GUID = A9B8D792-F454-11DE-BA92-FDCF56D89593

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
	//return( ERROR_SUCCESS ); 
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
   
	//lua_State *L = lua_open();
	//luaL_openlibs(L);

	// Get path to install dir from registry 
	//wchar_t *sLuaPath;
	int test = runLua("gpe", pStatusCallback);

	return( ERROR_SUCCESS ); 
}