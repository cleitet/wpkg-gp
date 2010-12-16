#include <string>
#include <userenv.h>
#include <winbase.h>
#include "luaFunctions.h"
#include "LogMessage.h"

extern "C"
{
	#include <lauxlib.h>
	#include <lualib.h>
	#include <lua.h>
}

static PFNSTATUSMESSAGECALLBACK gStatusCallback = NULL;

//Performs conversion from LPCSTR to LPWSTR - if any failure occurs returns NULL
//When not needed returned string MUST be freed with HeapFree function only
LPWSTR convertToWideString (LPCSTR lpszStringIn)
{
	LPWSTR lpwszOut = NULL;
	size_t uiSize = 0;
	size_t uiConvertedChars = 0;
	errno_t iRetVal = -1;

	if (lpszStringIn == NULL)
		return NULL;

	uiSize = strlen(lpszStringIn) + 1;
	
	lpwszOut = (LPWSTR) HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, uiSize * sizeof (WCHAR));
	if (lpwszOut != NULL)
	{
		iRetVal = mbstowcs_s(&uiConvertedChars, lpwszOut, uiSize, lpszStringIn, _TRUNCATE);
		if (iRetVal != 0)
		{
			//TODO
			//process error here
			return NULL;
		}
	}

	return lpwszOut;
}

//Provides the ability to write to the Winlogon status window at boot
//Returns return value of the global StatusCallback function or
//NULL if gStatusCallback is NULL.
int luaWriteStatus(lua_State *L)
{
	LPWSTR lpwszMessage = NULL;
	DWORD ulRet;

	if (gStatusCallback == NULL) return 0;

	LPCSTR lpcszMessage = luaL_checkstring(L, 1);
	lpwszMessage = convertToWideString (lpcszMessage);
	if (lpwszMessage != NULL)
	{
		ulRet = gStatusCallback(FALSE, lpwszMessage);
		HeapFree (GetProcessHeap(), 0, lpwszMessage);
	}
	return 0; // Number of arguments put on the LUA stack
}

int luaSetLogFlags(lua_State *L)
{
	lua_pushnumber (L, EVENTLOG_SUCCESS);
	lua_setglobal  (L, "EVENTLOG_SUCCESS");
	lua_pushnumber (L, EVENTLOG_AUDIT_FAILURE);
	lua_setglobal  (L, "EVENTLOG_AUDIT_FAILURE");
	lua_pushnumber (L, EVENTLOG_AUDIT_SUCCESS);
	lua_setglobal  (L, "EVENTLOG_AUDIT_SUCCESS");
	lua_pushnumber (L, EVENTLOG_ERROR_TYPE);
	lua_setglobal  (L, "EVENTLOG_ERROR_TYPE");
	lua_pushnumber (L, EVENTLOG_INFORMATION_TYPE);
	lua_setglobal  (L, "EVENTLOG_INFORMATION_TYPE");
	lua_pushnumber (L, EVENTLOG_WARNING_TYPE);
	lua_setglobal  (L, "EVENTLOG_WARNING_TYPE");
	return 0;
}

int luaWriteLog(lua_State *L)
{	
	LPCSTR lpszString = NULL;
	// Log flag (call SetLogFlags to get)
    int wFlag = luaL_checkinteger(L, 1);
	// String to log
	lpszString = luaL_checkstring(L, 2);

	if (lpszString != NULL)
	{
		logMessage(wFlag, lpszString);
	}
	return 0;
}

// Returns a malloc()ed pointer. Make sure you free() it afterwards
// Returns NULL on error and logs error to Event Log
LPSTR GetHKLMStringValue(LPCSTR lpcszSubKey, LPCSTR lpcszValueName)
{
	const DWORD dwMaxBufferSize = 4096;
	HKEY   hKey       = NULL;
	DWORD  dwSize     = 0;
	DWORD  dwDataType = REG_SZ;
	LPBYTE lpValue    = NULL;
	LONG   lRet;
	char szErrorString[dwMaxBufferSize];
	char szErrorMessage[dwMaxBufferSize];

	lRet = RegOpenKeyExA(HKEY_LOCAL_MACHINE, lpcszSubKey, 0, KEY_QUERY_VALUE, &hKey);
	if (lRet != ERROR_SUCCESS){
		//error handling
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, lRet, NULL, szErrorString, dwMaxBufferSize, NULL);
		sprintf_s(szErrorMessage, dwMaxBufferSize,
			"Error opening key: HKLM\\%s\r\nError was: %s",
			lpcszSubKey, szErrorString);
		// Writing to log
		logMessage(EVENTLOG_ERROR_TYPE, szErrorMessage);
		return NULL;
	}
	
	// Call first RegQueryValueEx to retrieve the necessary buffer size
	lRet = RegQueryValueExA(hKey, lpcszValueName, 0, &dwDataType, lpValue, &dwSize); 
	// dwSize will contain the data size

	if (lRet != ERROR_SUCCESS){
		//error handling
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, lRet, NULL, szErrorString, dwMaxBufferSize, NULL);
		sprintf_s(szErrorMessage, dwMaxBufferSize,
			"Error reading key value: HKLM\\%s\\%s\\%s\\\r\nError was: %s",
			lpcszSubKey, lpcszValueName, szErrorString);
		// Writing to log
		logMessage(EVENTLOG_ERROR_TYPE, szErrorMessage);
		return NULL;
	}

	// Second call:
	// Allocate the buffer
	lpValue = (LPBYTE) malloc(dwSize + 1); // Add a byte for the Null termination

	// Call second RegQueryValueEx to get the value
	lRet = RegQueryValueExA(hKey, lpcszValueName, 0, &dwDataType, lpValue, &dwSize);
	RegCloseKey(hKey);

	// Adding null termination to lpValue
	lpValue[dwSize] = '\0';

	return (LPSTR)lpValue;
}

// Converting Widechar (UTF-16) to char
LPSTR wchar2char(LPWSTR wString)
{
	// Converting from widechar
	int cc = WideCharToMultiByte(CP_OEMCP, 0, wString, -1, NULL, 0, NULL, NULL);
	LPSTR cString = new char[cc];
	WideCharToMultiByte(CP_OEMCP, 0, wString, -1, cString, cc, NULL, NULL);
	return cString;
}

//Reboots the computer
//int luaReboot(lua_State *L)
//{
//	int i = InitiateSystemShutdownEx(
//		NULL, NULL, 0, TRUE, TRUE, 
//		SHTDN_REASON_MAJOR_APPLICATION | 
//		SHTDN_REASON_MINOR_INSTALLATION | 
//		SHTDN_REASON_FLAG_PLANNED
//		);
//	if (i == 0){
//		int error = GetLastError();
//		LPWSTR lpErrorString;
//		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
//			0, error, NULL, (LPWSTR)&lpErrorString, 0, NULL);
//
//		LPSTR ErrorString = wchar2char(lpErrorString);
//		LocalFree(lpErrorString);
//
//		lua_pushboolean(L, 1);
//		lua_pushstring(L, ErrorString);
//		
//		return 2;
//	} else {
//		lua_pushboolean(L, 0);
//		return 1;
//	}
//}

// runLua: Initializes global variables for Lua, and runs it
// cCallingMetod is a parameter to tell Lua script weither it's called from gpe
//or from a testing script (can be either "script" or "gpe")
// returns 0 on success, 1 on Lua script not found and 2 on unable to run lua script
// (probably due to syntax errors)
int runLua(LPCSTR cCallingMethod, PFNSTATUSMESSAGECALLBACK pStatusCallback) {
	gStatusCallback = pStatusCallback;

	LPSTR LuaScript = GetHKLMStringValue("SOFTWARE\\WPKG-gp", "LuaScript");
	if (LuaScript == NULL)
	{
		//error: could not find lua script
		return(1);
	}

	LPSTR file = LuaScript;

	// Getting settings from regstry (Set by Group Policy).
	LPSTR cWpkgPath       = GetHKLMStringValue("SOFTWARE\\Policies\\WPKG_GP", "WpkgPath");
	LPSTR cWpkgParameters = GetHKLMStringValue("SOFTWARE\\Policies\\WPKG_GP", "WpkgParameters");
	LPSTR cWpkgVerbosity  = GetHKLMStringValue("SOFTWARE\\Policies\\WPKG_GP", "WpkgVerbosity");
	if (!cWpkgPath || !cWpkgParameters || !cWpkgVerbosity){
		LPSTR cLogMsg = "Error when opening keys under HKLM\\SOFTWARE\\Policies\\WPKG_GP";
		logMessage(EVENTLOG_ERROR_TYPE, cLogMsg);
		return(2);
	}

	//Preparing Lua
	lua_State *L = lua_open();
	luaL_openlibs(L);

	//Adding functions
	lua_pushcfunction(L, luaSetLogFlags);
	lua_setglobal(L, "SetLogFlags");
	lua_pushcfunction(L, luaWriteStatus);
	lua_setglobal(L,  "WriteStatus");
	lua_pushcfunction(L, luaWriteLog);
	lua_setglobal(L,  "WriteLog");
	//lua_pushcfunction(L, luaReboot);
	//lua_setglobal(L, "Reboot");

	//Adding settings
	lua_pushstring(L, cWpkgPath);
	lua_setglobal(L,  "WpkgPath");
	lua_pushstring(L, cWpkgParameters);
	lua_setglobal(L,  "WpkgParameters");
	lua_pushstring(L, cWpkgVerbosity);
	lua_setglobal(L,  "WpkgVerbosity");
	lua_pushstring(L, cCallingMethod);
	lua_setglobal(L,  "CallingMethod");
	
	luaL_loadfile(L, file);

	if (lua_pcall(L, 0, 0, 0)){
		LPSTR cLogMsg = NULL;
		//printf("Unable to run Lua script file: %s", lua_tostring(L, -1));
		sprintf_s(cLogMsg, 200, "Unable to run Lua script file: %s", lua_tostring(L, -1));
		logMessage(EVENTLOG_ERROR_TYPE, cLogMsg);
		return(2);
	}
	lua_close(L);
	return(0);
}