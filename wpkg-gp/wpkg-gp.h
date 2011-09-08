// WPKG-gp.h
#include <Windows.h>

extern "C" __declspec( dllexport ) DWORD CALLBACK ProcessGroupPolicy(
  DWORD dwFlags,
  HANDLE hToken,
  HKEY hKeyRoot,
  PGROUP_POLICY_OBJECT pDeletedGPOList,
  PGROUP_POLICY_OBJECT pChangedGPOList,
  ASYNCCOMPLETIONHANDLE pHandle,
  BOOL *pbAbort,
  PFNSTATUSMESSAGECALLBACK pStatusCallback
);