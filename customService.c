/*
 * customService.cpp
 *
 *  Created on: 30 мар. 2024 г.
 *      Author: Администратор
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
//#include "test.h"

SERVICE_STATUS        ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

void ServiceMain(int argc, char** argv);
void ControlHandler(DWORD request);

void event(HANDLE hFile , HANDLE hEventSource, LPCSTR lpszStrings[2], char* buffer );
char* readFromFile(const char* filename);
void writeInLog( HANDLE hEventSource, LPCSTR lpszStrings[2], char* buffer);


int main()
{
    SERVICE_TABLE_ENTRY ServiceTable[2];
    ServiceTable[0].lpServiceName = "MyCustomService";
    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
    ServiceTable[1].lpServiceName = NULL;
    ServiceTable[1].lpServiceProc = NULL;

    StartServiceCtrlDispatcher(ServiceTable);
    return 0;
}

void ServiceMain(int argc, char** argv)
{
    ServiceStatus.dwServiceType        = SERVICE_WIN32;
    ServiceStatus.dwCurrentState       = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    ServiceStatus.dwWin32ExitCode      = 0;
    ServiceStatus.dwServiceSpecificExitCode = 0;
    ServiceStatus.dwCheckPoint         = 0;

    hStatus = RegisterServiceCtrlHandler("MyCustomService", (LPHANDLER_FUNCTION)ControlHandler);

    char* buffer = (void*)NULL;
    HANDLE hEventSource;
    LPCSTR lpszStrings[2];
    HANDLE hFile = CreateFile(TEXT("C:\\textic.txt"),
    	    		(GENERIC_READ | GENERIC_WRITE), 0,
    	    		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    hEventSource = RegisterEventSource(NULL, "MyCustomService");


    if (hStatus == (SERVICE_STATUS_HANDLE)0)
    {
    	sprintf(buffer, "error status handle");
    	lpszStrings[0] = buffer;
    	ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, lpszStrings, NULL);
        return;
    }

    ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(hStatus, &ServiceStatus);

    event(hFile, hEventSource, lpszStrings, buffer);

    CloseHandle(hFile);
    DeregisterEventSource(hEventSource);
}

void ControlHandler(DWORD request)
{
    switch(request)
    {
        case SERVICE_CONTROL_STOP:
            ServiceStatus.dwWin32ExitCode = 0;
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED;
            SetServiceStatus(hStatus, &ServiceStatus);
            return;

        case SERVICE_CONTROL_SHUTDOWN:
            ServiceStatus.dwWin32ExitCode = 0;
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED;
            SetServiceStatus(hStatus, &ServiceStatus);
            return;

        default:
            break;
    }

    SetServiceStatus(hStatus, &ServiceStatus);
    return;
}

void event(HANDLE hFile , HANDLE hEventSource, LPCSTR lpszStrings[2], char* buffer )
{
	if(hFile == INVALID_HANDLE_VALUE)
	{
		writeInLog(hEventSource, lpszStrings, "Invalid handle value!");
		return;
	}
	else
	{
		writeInLog(hEventSource, lpszStrings,"handle is valid");
		system("dir > textic.txt");
		buffer = readFromFile("textic.txt");
	}

	DWORD dvBytesToWrite = strlen(buffer) * sizeof(char);
	DWORD dvBytesWritten = 0;

	WriteFile(hFile,
			(void*)buffer,
			dvBytesToWrite,
			&dvBytesWritten,
			NULL);

	if(dvBytesWritten == dvBytesToWrite)
	{
		writeInLog(hEventSource, lpszStrings,"file write successful");
	}
	else if(dvBytesWritten > 0)
	{
		writeInLog(hEventSource, lpszStrings,"file write partially successful");
	}
	else
	{
		writeInLog(hEventSource, lpszStrings,"file write failed");
	}
}

void writeInLog( HANDLE hEventSource, LPCSTR lpszStrings[2], char* buffer){
	lpszStrings[0] =  buffer;
	ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, lpszStrings, NULL);
}

char* readFromFile(const char* filename){
	FILE* fp;
	char buffer[1024];
	char* result = NULL;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Ошибка при открытии файла.\n");
		return NULL;
	}

	size_t totalSize = 0;
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		size_t bufferSize = strlen(buffer);
		void* re = realloc(result, totalSize + bufferSize + 1);
		char* newResult = (char*)re;
		if (newResult == NULL) {
			free(result);
			fclose(fp);
			return NULL;
		}
		result = newResult;
		strcpy(result + totalSize, buffer);
		totalSize += bufferSize;
	}

	fclose(fp);
	return result;
}

