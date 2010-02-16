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

static PFNSTATUSMESSAGECALLBACK gStatusCallback;

//Provides the ability to write to the Winlogon status window at boot
//Returns return value of the global StatusCallback function or
//NULL if gStatusCallback is NULL.
int luaWriteStatus(lua_State *L)
{

	LPCSTR orig = luaL_checkstring(L, 1);

	// Convert to a wchar_t*
    size_t origsize = strlen(orig) + 1;
    const size_t newsize = 100;
    size_t convertedChars = 0;
    wchar_t wcstring[newsize];
    mbstowcs_s(&convertedChars, wcstring, origsize, orig, _TRUNCATE);

	if (gStatusCallback == NULL) return NULL;
    else return gStatusCallback(FALSE, wcstring);
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
	// Log flag (call SetLogFlags to get)
    int wFlag = luaL_checkinteger(L, 1);
	// String to log
	const char *orig = luaL_checkstring(L, 2);

	// Convert to a wchar_t*
    size_t origsize = strlen(orig) + 1;
    const size_t newsize = 100;
    size_t convertedChars = 0;
    wchar_t wcstring[newsize];
    mbstowcs_s(&convertedChars, wcstring, origsize, orig, _TRUNCATE);
    //wcscat_s(wcstring, L" (wchar_t *)");

	
	logMessage(wFlag, wcstring);
	return 0;
}





// Returns a malloc()ed pointer. Make sure you free() it afterwards
// Returns NULL on error and logs error to Event Log
LPWSTR GetHKLMStringValue(LPWSTR wcSubKey, LPWSTR wcValueName)
{
	HKEY  hKey       = NULL;
	DWORD dwSize     = 0;
	DWORD dwDataType = REG_SZ;
	LPBYTE lpValue   = NULL;
	LPCTSTR const lpValueName = wcValueName;
	
	LPWSTR lpErrorString;

	LONG lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wcSubKey, 0, KEY_QUERY_VALUE, &hKey);
	
	if (lRet != ERROR_SUCCESS){
		
		//error handling
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			0, lRet, NULL, (LPWSTR)&lpErrorString, 0, NULL);

		std::wstring sErrorMessage;
		sErrorMessage += L"Error opening key: HKLM\\";
		sErrorMessage += wcSubKey;
		sErrorMessage += L"\r\nError was: ";
		sErrorMessage += lpErrorString;
		
		// Writing to log
		logMessage(EVENTLOG_ERROR_TYPE, sErrorMessage.data());
		
		LocalFree(lpErrorString);
		return L"";
	}
	
	// Call first RegQueryValueEx to retrieve the necessary buffer size
	lRet = RegQueryValueEx(hKey, lpValueName, 0, &dwDataType, lpValue, &dwSize); // dwSize will contain the data size
	if (lRet != ERROR_SUCCESS){
		//error handling
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			0, lRet, NULL, (LPWSTR)&lpErrorString, 0, NULL);

		std::wstring sErrorMessage;
		sErrorMessage += L"Error reading key value: HKLM\\";
		sErrorMessage += wcSubKey;
		sErrorMessage += L"\\";
		sErrorMessage += lpValueName;
		sErrorMessage += L"\r\nError was: ";
		sErrorMessage += lpErrorString;
		
		// Writing to log
		logMessage(EVENTLOG_ERROR_TYPE, sErrorMessage.data());
		
		LocalFree(lpErrorString);
		return L"";
	}


	// Allocate the buffer
	lpValue = (LPBYTE) malloc(dwSize + 1); // Add a byte for the Null termination

	// Call second RegQueryValueEx to get the value
	lRet = RegQueryValueEx(hKey, lpValueName, 0, &dwDataType, lpValue, &dwSize);
	RegCloseKey(hKey);

	// Adding null termination to lpValue
	lpValue[dwSize] = '\0';

	return (LPWSTR )lpValue;
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

// runLua: Initializes global variables for Lua, and runs it
// cCallingMetod is a parameter to tell Lua script weither it's called from gpe
//or from a testing script (can be either "script" or "gpe")
// returns 0 on success, 1 on Lua script not found and 2 on unable to run lua script
// (probably due to syntax errors)
int runLua(LPCSTR cCallingMethod, PFNSTATUSMESSAGECALLBACK pStatusCallback) {
	gStatusCallback = pStatusCallback;

	LPWSTR wLuaScript = GetHKLMStringValue(L"SOFTWARE\\WPKG-gp", L"LuaScript");
	if (wLuaScript == NULL)
	{
		//error: could not find lua scritp
		return(1);
	}

	LPSTR file = wchar2char(wLuaScript);
	free(wLuaScript);

	// Getting settings from regstry (Set by Group Policy).
	LPWSTR wWpkgPath       = GetHKLMStringValue(L"SOFTWARE\\Policies\\WPKG_GP", L"WpkgPath");
	LPWSTR wWpkgParameters = GetHKLMStringValue(L"SOFTWARE\\Policies\\WPKG_GP", L"WpkgParameters");
	LPWSTR wWpkgVerbosity  = GetHKLMStringValue(L"SOFTWARE\\Policies\\WPKG_GP", L"WpkgVerbosity");
	LPSTR  cWpkgPath       = wchar2char(wWpkgPath);
	LPSTR  cWpkgParameters = wchar2char(wWpkgParameters);
	LPSTR  cWpkgVerbosity  = wchar2char(wWpkgVerbosity);
	free(wWpkgPath);
	free(wWpkgParameters);
	free(wWpkgVerbosity);

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
		logcMessage(EVENTLOG_ERROR_TYPE, cLogMsg);
		return(2);
	}
	lua_close(L);
	return(0);
}