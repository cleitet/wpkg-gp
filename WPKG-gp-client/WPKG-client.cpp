// WPKG-client.cpp
#include <windows.h>
//#include <iostream.h>
#include <stdio.h>
#include "WPKG-client.h"
#include "..\Components\LuaFunctions.h"
#include "..\Components\LogMessage.h"


int main(int argc, char* argv[]){

	return runLua("script", NULL);
}