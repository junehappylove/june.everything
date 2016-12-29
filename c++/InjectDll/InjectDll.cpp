// InjectDll.cpp : Defines the entry point for the console application.
//
//#include "stdafx.h"
#include "windows.h"
#include "tchar.h"

BOOL InjectDll(DWORD dwPID, LPCTSTR szDllPath) {
	HANDLE hProcess =NULL, hThread = NULL;
	HMODULE hMod = NULL;
	LPVOID pRemoteBuf = NULL;
	DWORD dwBufSize = (DWORD)(_tcslen(szDllPath)+1)*sizeof(TCHAR);
	LPTHREAD_START_ROUTINE pThreadProc;
	// #1 使用dwPID获取目标进程句柄。
	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID))) {
		_tprintf(L"OpenProcess(%d) failed![%d]\n",dwPID,GetLastError());
		return FALSE;
	}
	// #2 在目标进程的内存中分配dwBufSize大小的内存空间
	pRemoteBuf = VirtualAllocEx(hProcess,NULL,dwBufSize,MEM_COMMIT,PAGE_READWRITE);
	// #3 将要注入的dll路径写入分配的内存。
	WriteProcessMemory(hProcess,pRemoteBuf,(LPVOID)szDllPath,dwBufSize,NULL);
	// #4 获取LoadLibraryW 的API地址
	hMod = GetModuleHandle(L"kernel32.dll");
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod,"LoadLibraryW");
	// #5 在目标进程中运行线程
	hThread = CreateRemoteThread(hProcess,NULL,0,pThreadProc,pRemoteBuf,0,NULL);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	CloseHandle(hProcess);
	return TRUE;
}

int _tmain(int argc,TCHAR *argv[]) {
	if (argc != 3) {
		_tprintf(L"USAGE:%s pid dll_path\n",argv[0]);
		return 1;
	}
	// inject dll
	if (InjectDll((DWORD)_tstol(argv[1]), argv[2])) 
		_tprintf(L"InjectDll(\"%s\") success!\n",argv[2]);
	else
		_tprintf(L"InjectDll(\"%s\") failed!\n", argv[2]);
	
	return 0;
}