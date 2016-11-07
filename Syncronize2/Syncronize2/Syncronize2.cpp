// Syncronize2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#define BUF_SIZE 256
LPCTSTR mapFileName = TEXT("myFile");
LPCTSTR myMutexName = TEXT("NameOfMutexObject");

BOOL SetPrivilege( HANDLE hToken,               // access token handle
				   LPCTSTR lpszPrivilege,    // name of privilege to enable/disable
				   BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValue( NULL,                // lookup privilege on local system
							   lpszPrivilege,    // privilege to lookup
							   &luid))               // receives LUID of privilege
	{
		printf("LookupPrivilegeValue() error: %u\n", GetLastError());
		return FALSE;
	}
	else
		printf("LookupPrivilegeValue() is OK\n");

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;

	// Don't forget to disable the privileges after you enabled them,
	// or have already completed your task. Don't mess up your system :o)
	if (bEnablePrivilege)
	{
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		printf("tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED\n");
	}
	else
	{
		tp.Privileges[0].Attributes = 0;
		printf("tp.Privileges[0].Attributes = 0\n");
	}

	// Enable the privilege (or disable all privileges).
	if (!AdjustTokenPrivileges( hToken,
								FALSE, // If TRUE, function disables all privileges, if FALSE the function modifies privilege based on the tp
								&tp,
								sizeof(TOKEN_PRIVILEGES),
								(PTOKEN_PRIVILEGES)NULL,
								(PDWORD)NULL))
	{
		printf("AdjustTokenPrivileges() error: %u\n", GetLastError());
		return FALSE;
	}
	else
	{
		printf("AdjustTokenPrivileges() is OK, last error if any: %u\n", GetLastError());
		printf("Should be 0, means the operation completed successfully = ERROR_SUCCESS\n");
	}
	return TRUE;
}
bool accessToken(HANDLE &hToken) {
	// Open a handle to the access token for the calling process. That is this running program
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		printf("OpenProcessToken() error %u\n", GetLastError());
		return 1;
	}
	else
		printf("OpenProcessToken() is OK\n");
	return 0;
}

bool myOpenFileMapping(HANDLE &hMapFile, LPCTSTR &pBuf) {
	hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS,   // read/write access
							   FALSE,                 // do not inherit the name
							   mapFileName);               // name of mapping object

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not open file mapping object (%d).\n"), GetLastError());
		return 1;
	}

	pBuf = (LPTSTR)MapViewOfFile(hMapFile, // handle to map object
								 FILE_MAP_ALL_ACCESS,  // read/write permission
								 0,
								 0,
								 BUF_SIZE);

	if (pBuf == NULL)
	{
		_tprintf(TEXT("Could not map view of file (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return 1;
	}
	return 0;
}

bool myOpenMutex(HANDLE &hMutex) {
	hMutex = OpenMutex(SYNCHRONIZE,            // request full access
					   FALSE,                       // handle not inheritable
					   myMutexName);  // object name

	if (hMutex == NULL) {
		printf("OpenMutex error: %d\n", GetLastError());
		return 1;
	}
	else {
		printf("OpenMutex successfully opened the mutex.\n");
		return 0;
	}
	return 0;
}

int main(void)
{
	HANDLE hMapFile = NULL;
	HANDLE hMutex = NULL;
	LPCTSTR pBuf = NULL;
	LPCTSTR lpszPrivilege = SE_DEBUG_NAME;
	BOOL bEnablePrivilege = TRUE;			// Change this BOOL value to set/unset the SE_PRIVILEGE_ENABLED attribute
	HANDLE hToken = NULL;

	if (accessToken(hToken) == 1)
		return 1;

	// Call the user defined SetPrivilege() function to enable and set the needed privilege
	BOOL test = SetPrivilege(hToken, lpszPrivilege, bEnablePrivilege);
	printf("The SetPrivilege() return value: %d\n\n", test);

	//************************************************
	if (myOpenFileMapping(hMapFile, pBuf) == 1)
		return 1;
	
	if (myOpenMutex(hMutex) == 1)
		return 1;

	WaitForSingleObject(hMutex, 10);
	wprintf(L"%s\n", pBuf);

	CloseHandle(hMutex);
	CloseHandle(hMapFile);

	//************************************************
	// After we have completed our task, don't forget to disable the privilege
	bEnablePrivilege = FALSE;
	BOOL test1 = SetPrivilege(hToken, lpszPrivilege, bEnablePrivilege);
	printf("The SetPrivilage() return value: %d\n", test1);
	return 0;
}
