#include "WpkgWorker.h"



WpkgWorker::WpkgWorker(std::wstring wWpkgPath, std::wstring wWpkgOptions)
{
	// Compiling command string
	wWpkgCommand = WPKG_CSCRIPT_COMMAND;
	wWpkgCommand += L" ";
	wWpkgCommand += wWpkgPath.data();
	wWpkgCommand += L" ";
	wWpkgCommand += wWpkgOptions.data();
}

void WpkgWorker::ParseOutput(wchar_t wOutput[WPKG_MAX_OUTPUT_LENGTH])
{
	// log the data
	//logMessage(EVENTLOG_INFORMATION_TYPE, wOutput);

	printf("%ls", wOutput);
	wchar_t* part = wcstok(&wOutput[1], L":");
	//printf("%ls", part);
}


void WpkgWorker::ExecuteCommand()
{
   wchar_t psBuffer[WPKG_MAX_OUTPUT_LENGTH];
   FILE    *pPipe;

        /* Run command so that it writes its output to a pipe. Open this
         * pipe with read text attribute so that we can read it 
         * like a text file. 
         */

   // _wopen() - http://msdn.microsoft.com/en-us/library/96ayss4b%28VS.80%29.aspx
   if( (pPipe = _wpopen(wWpkgCommand.data(), L"rt" )) == NULL )
	   logMessage(EVENTLOG_ERROR_TYPE, L"Error: WpkgWorker failed to open pipe:");

   //Read pipe WPKG_MAX_OUTPUT_LENGTH characters at the time until end
   // of file, or an error occurs.

   while(fgetws(psBuffer, WPKG_MAX_OUTPUT_LENGTH, pPipe))
   {
	   //Output from WPKG comes here
	   ParseOutput(psBuffer);
   }


   // Close pipe and print return value of pPipe.
   if (feof( pPipe))
   {
	   std::wstring logMsg;
	   int iReturnValue = _pclose(pPipe);
	   std::wostringstream ssReturnValue;
	   ssReturnValue << iReturnValue;


	   logMsg  = L"WpkgWorker: pipe for Wpkg process returned: ";
	   logMsg += ssReturnValue.str();
	   logMsg += L"\nCommand run was: ";
	   logMsg += wWpkgCommand.data();

	   //On successful close
	   if (iReturnValue == 0)
		   logMessage(EVENTLOG_INFORMATION_TYPE, (LPWSTR) logMsg.data());
	   //On closing error
	   else
		   logMessage(EVENTLOG_ERROR_TYPE, (LPWSTR) logMsg.data());
   }
   else
	   logMessage(EVENTLOG_ERROR_TYPE, L"Error: WpkgWorker failed to read the pipe to the end.");
}