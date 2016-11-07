// Syncronize.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#define BUF_SIZE 256
LPCTSTR mapFileNap = TEXT("myFile");
LPCTSTR myMutexName = TEXT("NameOfMutexObject");
wchar_t * message = TEXT("\nFirst process\n");

bool createAndMapFile(HANDLE &hMapFile, LPTSTR &pBuf) {
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE,    // use paging file
								 NULL,                    // default security
								 PAGE_READWRITE,          // read/write access
								 0,                       // maximum object size (high-order DWORD)
								 BUF_SIZE,                // maximum object size (low-order DWORD)
								 mapFileNap);

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not create file mapping object (%d).\n"), GetLastError());
		return 1;
	}

	pBuf = (LPTSTR)MapViewOfFile(hMapFile,   // handle to map object
								 FILE_MAP_ALL_ACCESS, // read/write permission
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

bool myCreateMutex(HANDLE &hMutex) {
	hMutex = CreateMutex(NULL,                        // default security descriptor
						 FALSE,                       // mutex not owned
						 myMutexName);  // object name

	if (hMutex == NULL) {
		printf("CreateMutex error: %d\n", GetLastError());
		return 1;
	}
	else {
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			printf("CreateMutex opened an existing mutex\n");
			return 1;
		}
		else {
			printf("CreateMutex created a new mutex.\n");
			return 0;
		}
	}
	return 0;
}

int main(void)
{
	HANDLE hMutex = NULL;
	HANDLE hMapFile = NULL;
	LPTSTR pBuf = NULL;

	if (createAndMapFile(hMapFile, pBuf) == 1)
		return 1;
	
	if (myCreateMutex(hMutex) == 1)
		return 1;
	
	WaitForSingleObject(hMutex, INFINITE);
	CopyMemory((PVOID)pBuf, message, BUF_SIZE * sizeof(wchar_t));
	_getch();

	UnmapViewOfFile(pBuf);
	CloseHandle(hMutex);
	CloseHandle(hMapFile);
	return 0;
}


