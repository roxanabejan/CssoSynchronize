// Syncronize.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include<time.h>
#include <iostream>
using namespace std;
#define BUF_SIZE 1024

HANDLE hMutex = NULL;
HANDLE hReadEvent = NULL, hWriteEvent = NULL;
LPTSTR pBuf = NULL;
LPCTSTR mapFileName = TEXT("myFile");
LPCTSTR myMutexName = TEXT("NameOfMutexObject");
LPCTSTR WriteEventName = TEXT("WriteEvent");
LPCTSTR ReadEventName = TEXT("ReadEvent");
wchar_t * message = TEXT("\nFirst process\n");
int a, b; 

bool createAndMapFile(HANDLE &hMapFile, LPTSTR &pBuf) {
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE,    // use paging file
								 NULL,                    // default security
								 PAGE_READWRITE,          // read/write access
								 0,                       // maximum object size (high-order DWORD)
								 BUF_SIZE,                // maximum object size (low-order DWORD)
								 mapFileName);

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

bool myCreateEvent(HANDLE &hEvent, LPCTSTR myEventName) {
	hEvent = CreateEvent(NULL,               // default security attributes
		TRUE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		myEventName  // object name
	);
	if (hEvent == NULL)
	{
		printf("CreateEvent failed (%d)\n", GetLastError());
		return 1;
	}
	else {
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			printf("CreateEvent opened an existing event\n");
			return 1;
		}
		else {
			printf("CreateEvent created a new event.\n");
			return 0;
		}
	}
	return 0;
}

void write_to_pBuf() {
	pBuf++;
	int a = rand()%10;
	CopyMemory((PVOID)pBuf, &a, sizeof(DWORD));
	int b = a * 2;
	pBuf++;
	CopyMemory((PVOID)pBuf, &b, sizeof(DWORD));
	cout << "a=" << a << " b=" << b << "\n";
}

void write_data_mutex()
{
	for (int i = 0; i<300; i++)
	{
		//se acceseaza datele comune pentru scriere, deci se intra in sectiune critica
		WaitForSingleObject(hMutex, INFINITE);
		write_to_pBuf();
		
		ReleaseMutex(hMutex);
	}
	_getch();
	
}

void write_data_event()
{
	SetEvent(hWriteEvent);
	
	for (int i = 0; i<300; i++)
	{
		ResetEvent(hReadEvent);
		WaitForSingleObject(hWriteEvent, INFINITE);

		write_to_pBuf();
		_getch();

		SetEvent(hReadEvent);
		
	}
}

bool sync_mutex() {

	HANDLE hMapFile = NULL;

	if (createAndMapFile(hMapFile, pBuf) == 1)
		return 1;

	if (myCreateMutex(hMutex) == 1)
		return 1;
	
	write_data_mutex();

	UnmapViewOfFile(pBuf);
	CloseHandle(hMutex);
	CloseHandle(hMapFile);
	return 0;
}

bool sync_event() {

	HANDLE hMapFile = NULL;

	if (createAndMapFile(hMapFile, pBuf) == 1)
		return 1;

	if (myCreateEvent(hWriteEvent, WriteEventName) == 1)
		return 1;

	if (myCreateEvent(hReadEvent, ReadEventName) == 1)
		return 1;

	write_data_event();

	UnmapViewOfFile(pBuf);
	CloseHandle(hWriteEvent);
	CloseHandle(hReadEvent);
	CloseHandle(hMapFile);
	return 0;
}

int main(void)
{
	/*if (sync_mutex() == 1)
		return 1;*/	

	if (sync_event() == 1)
		return 1;

	return 0;
}


