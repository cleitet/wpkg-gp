// wpkg-gp-test.cpp : Defines the entry point for the console application.
#include <windows.h>
#include <userenv.h>
#include <winbase.h>
#include <stdio.h>
#include "wpkg_common.h"
#include "messagefile.h"

int main(int argc, char* argv[])
{
	if (argc > 1 && strcmp(argv[1], "-debug") == 0)
		return(executeWpkgViaPipe(EXE, TRUE));
	else
		return(executeWpkgViaPipe(EXE, FALSE));
}

