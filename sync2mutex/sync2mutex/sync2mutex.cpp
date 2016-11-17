// Syncronize2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <iostream>
using namespace std;
#define BUF_SIZE 1024
LPCTSTR mapFileName = TEXT("myFile");
LPCTSTR myMutexName = TEXT("NameOfMutexObject");
LPCTSTR ReadEventName = TEXT("ReadEvent");
LPCTSTR WriteEventName = TEXT("WriteEvent");
HANDLE hMutex = NULL;
HANDLE hReadEvent = NULL, hWriteEvent = NULL;
HANDLE hMapFile = NULL;
LPCTSTR pBuf = NULL;



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

bool myOpenEvent(HANDLE &hEvent, LPCTSTR myEventName) {
	hEvent = OpenEvent(EVENT_ALL_ACCESS,            // request full access
					   FALSE,                       // handle not inheritable
					   myEventName);
	if (hEvent == NULL) {
		printf("OpenEvent error: %d\n", GetLastError());
		return 1;
	}
	else {
		printf("OpenEvent successfully opened the event.\n");
		return 0;
	}
	return 0;
}

void check_nr(wchar_t* buffer) {
	char message[256], nr1[256], nr2[256];
	int a, b, i = 0;
	sprintf(message, "%ws", buffer);
	while (message[i] != ' ') {
		nr1[i] = message[i];
		i++;
	}
	nr1[i] = '\0';
	strcpy(nr2, message + i + 1);
	a = atoi(nr1);
	b = atoi(nr2);
	printf("%d %d\n", a, b);
	if (b != 2 * a)
		printf("incorrect");
	
}

void read_from_buf() {
	pBuf++;
	int a = *pBuf;
	++pBuf;
	int b = *pBuf;
	cout << a << " " << b;
	if (b == a * 2) {
		cout << " corect\n";
	}
	else {
		cout << " gresit\n";
	}
}

void read_data_mutex()
{
	for (int i = 0; i<300; i++)
	{
		//se acceseaza datele comune pentru citire, deci se intra in seciune critica
		WaitForSingleObject(hMutex, INFINITE);
		read_from_buf();

		ReleaseMutex(hMutex);
	}
}

void read_data_event()
{
	for (int i = 0; i<300; i++)
	{
		ResetEvent(hWriteEvent);
		WaitForSingleObject(hReadEvent, INFINITE);

		read_from_buf();

		SetEvent(hWriteEvent);
	}
}

bool sync_mutex() {

	if (myOpenFileMapping(hMapFile, pBuf) == 1)
		return 1;

	if (myOpenMutex(hMutex) == 1)
	return 1;

	read_data_mutex();

	CloseHandle(hMutex);
	CloseHandle(hMapFile);
	return 0;
}

bool sync_event() {

	if (myOpenFileMapping(hMapFile, pBuf) == 1)
		return 1;

	if (myOpenEvent(hReadEvent, ReadEventName) == 1)
		return 1;
	
	if (myOpenEvent(hWriteEvent, WriteEventName) == 1)
		return 1;
	WaitForSingleObject(hWriteEvent, INFINITE);

	read_data_event();
	
	CloseHandle(hReadEvent);
	CloseHandle(hWriteEvent);
	CloseHandle(hMapFile);
	return 0;
}

int main(void)
{	
	//if (sync_mutex() == 1)
	//	return 1;

	if (sync_event() == 1)
		return 1;

	return 0;
}
