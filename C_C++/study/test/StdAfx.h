// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__62B77457_6A90_495C_8C37_075D52BF9318__INCLUDED_)
#define AFX_STDAFX_H__62B77457_6A90_495C_8C37_075D52BF9318__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>

// TODO: reference additional headers your program requires here
#include <stdio.h>
#include <wchar.h>
#include <wininet.h>
#include <stdlib.h>
#include <tchar.h>
#include <Wincrypt.h>
#include <locale.h>
#include <tlhelp32.h>
#include <string.h>
#include <iostream.h>
#include "BASE64_API.h"

#pragma comment(lib,"crypt32")
#define STR_MODULE_NAME (L"globalhook.dll")
#define STATUS_SUCCESS  (0x00000000L)

typedef LONG NTSTATUS;
// 客户ID
typedef struct _CLIENT_ID {
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} CLIENT_ID;

// 基本线程信息
typedef struct _THREAD_BASIC_INFORMATION {
	NTSTATUS ExitStatus;
	PVOID TebBaseAddress;
	CLIENT_ID ClientId;
	ULONG AffinityMask;
	LONG Priority;
	LONG BasePriority;
} THREAD_BASIC_INFORMATION;

// 恢复线程
typedef NTSTATUS(WINAPI *PFZWRESUMETHREAD)(HANDLE ThreadHandle, PULONG SuspendCount);

// 查询信息线程
typedef NTSTATUS(WINAPI *PFZWQUERYINFORMATIONTHREAD)
(
	HANDLE ThreadHandle,
	ULONG ThreadInformationClass,
	PVOID ThreadInformation,
	ULONG ThreadInformationLength,
	PULONG ReturnLength
	);

typedef DWORD(WINAPI *PFNTCREATETHREADEX)// pfnt create thread ex
(
	PHANDLE                 ThreadHandle,
	ACCESS_MASK             DesiredAccess,
	LPVOID                  ObjectAttributes,
	HANDLE                  ProcessHandle,
	LPTHREAD_START_ROUTINE  lpStartAddress,
	LPVOID                  lpParameter,
	BOOL	                CreateSuspended,
	DWORD                   dwStackSize,
	DWORD                   dw1,
	DWORD                   dw2,
	LPVOID                  Unknown
	);

typedef int (WINAPI *WIDECHARTOMULTIBYTE)(UINT CodePage, //指定执行转换的代码页
	DWORD dwFlags, //允许你进行额外的控制，它会影响使用了读音符号（比如重音）的字符
	LPCWSTR lpWideCharStr, //指定要转换为宽字节字符串的缓冲区
	int cchWideChar, //指定由参数lpWideCharStr指向的缓冲区的字符个数
	LPSTR lpMultiByteStr, //指向接收被转换字符串的缓冲区
	int cchMultiByte, //指定由参数lpMultiByteStr指向的缓冲区最大值
	LPCSTR lpDefaultChar, //遇到一个不能转换的宽字符，函数便会使用pDefaultChar参数指向的字符
	LPBOOL pfUsedDefaultChar);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
void DebugLog(const char *format, ...);
BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
BOOL hook_by_code(LPCSTR szDllName, LPCSTR szFuncName, PROC pfnNew, PBYTE pOrgBytes);
BOOL unhook_by_code(LPCSTR szDllName, LPCSTR szFuncName, PBYTE pOrgBytes);
BOOL IsVistaLater();
BOOL InjectDll(DWORD dwPID, LPCTSTR szDllPath);
BOOL MyCreateRemoteThread(HANDLE hProcess, LPTHREAD_START_ROUTINE pThreadProc, LPVOID pRemoteBuf);
NTSTATUS WINAPI NewZwResumeThread(HANDLE ThreadHandle, PULONG SuspendCount);
int WINAPI MyWideCharToMultiByte(UINT CodePage,
	DWORD dwFlags,
	LPCWSTR lpWideCharStr,
	int cchWideChar,
	LPSTR lpMultiByteStr,
	int cchMultiByte,
	LPCSTR lpDefaultChar,
	LPBOOL pfUsedDefaultChar);

// 这里应该换成当前进程的运行目录
#define FILE_PATH "F:\\image.jpg"	//定义文件路径已及名称

unsigned long GetFileSize(const char *path);
//char* GetBaes64(const char* path);//LPWSTR
char* GetBaes64(LPWSTR path);
//extern "C" __declspec (dllexport)void __stdcall SetData(WCHAR datas);
INT Save2File(char* datas,char* FileName);
//DWORD GetCurrentDirectory(DWORD nBufferLength, LPTSTR lpBuffer);
LPWSTR GetPath();
LPWSTR GetImageFilePath();
LPWSTR GetImageFilePath(LPWSTR ImggeName);
char* WcharToChar(const wchar_t* wp);
wchar_t* CharToWchar(const char* c);
WCHAR *char2wchar(char* data);
LPWSTR GetCurrentProcessPath(LPWSTR processName);

#endif // !defined(AFX_STDAFX_H__62B77457_6A90_495C_8C37_075D52BF9318__INCLUDED_)
