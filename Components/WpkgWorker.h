#include <windows.h>
#include <sstream>
#include <string>
#include <iostream>
#include "LogMessage.h"

// Max characters per line of output from WPKG
#define WPKG_MAX_OUTPUT_LENGTH 1024
#define WPKG_CSCRIPT_COMMAND L"cscript //nologo"

class WpkgWorker
{
private:
	std::wstring wWpkgCommand;
	void ParseOutput(wchar_t[WPKG_MAX_OUTPUT_LENGTH]);
public:
	WpkgWorker(std::wstring, std::wstring);
	void ExecuteCommand();
};