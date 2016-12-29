// stdafx.cpp : source file that includes just the standard includes
//	hook.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

LPWSTR ImageFile = L"\\image\\face.jpg";

/* 调试日志函数 */
void DebugLog(const char *format, ...)
{
	va_list vl;
	FILE *pf = NULL;
	char szLog[512] = { 0, };

	va_start(vl, format);
	wsprintfA(szLog, format, vl);
	va_end(vl);

	OutputDebugStringA(szLog);
}

/* 设置权限函数 */
BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp;
	HANDLE hToken;
	LUID luid;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		DebugLog("OpenProcessToken error: %u\n", GetLastError());
		return FALSE;
	}

	if (!LookupPrivilegeValue(NULL, // lookup privilege on local system
		lpszPrivilege,				// privilege to lookup 
		&luid))						// receives LUID of privilege
	{
		DebugLog("LookupPrivilegeValue error: %u\n", GetLastError());
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	// Enable the privilege or disable all privileges.
	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
	{
		DebugLog("AdjustTokenPrivileges error: %u\n", GetLastError());
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		DebugLog("The token does not have the specified privilege. \n");
		return FALSE;
	}

	return TRUE;
}

/* 开启挂钩 */
BOOL hook_by_code(LPCSTR szDllName, LPCSTR szFuncName, PROC pfnNew, PBYTE pOrgBytes)
{
	FARPROC pFunc = NULL;
	DWORD	dwOldProtect = 0, dwAddress = 0;
	BYTE	pBuf[5] = { 0xE9, 0, };
	PBYTE	pByte = NULL;
	HMODULE hMod = NULL;

	hMod = GetModuleHandleA(szDllName);
	if (hMod == NULL)
	{
		DebugLog("hook_by_code() : GetModuleHandle(\"%s\") failed!!! [%d]\n", szDllName, GetLastError());
		return FALSE;
	}

	pFunc = (FARPROC)GetProcAddress(hMod, szFuncName);
	if (pFunc == NULL)
	{
		DebugLog("hook_by_code() : GetProcAddress(\"%s\") failed!!! [%d]\n", szFuncName, GetLastError());
		return FALSE;
	}

	pByte = (PBYTE)pFunc;
	if (pByte[0] == 0xE9)
	{
		DebugLog("hook_by_code() : The API is hooked already!!!\n");
		return FALSE;
	}

	if (!VirtualProtect((LPVOID)pFunc, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect))
	{
		DebugLog("hook_by_code() : VirtualProtect(#1) failed!!! [%d]\n", GetLastError());
		return FALSE;
	}

	memcpy(pOrgBytes, pFunc, 5);

	dwAddress = (DWORD)pfnNew - (DWORD)pFunc - 5;
	memcpy(&pBuf[1], &dwAddress, 4);

	memcpy(pFunc, pBuf, 5);

	if (!VirtualProtect((LPVOID)pFunc, 5, dwOldProtect, &dwOldProtect))
	{
		DebugLog("hook_by_code() : VirtualProtect(#2) failed!!! [%d]\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

/* 卸载挂钩 */
BOOL unhook_by_code(LPCSTR szDllName, LPCSTR szFuncName, PBYTE pOrgBytes)
{
	FARPROC pFunc = NULL;
	DWORD	dwOldProtect = 0;
	PBYTE	pByte = NULL;
	HMODULE hMod = NULL;

	hMod = GetModuleHandleA(szDllName);
	if (hMod == NULL)
	{
		DebugLog("unhook_by_code() : GetModuleHandle(\"%s\") failed!!! [%d]\n", szDllName, GetLastError());
		return FALSE;
	}

	pFunc = (FARPROC)GetProcAddress(hMod, szFuncName);
	if (pFunc == NULL)
	{
		DebugLog("unhook_by_code() : GetProcAddress(\"%s\") failed!!! [%d]\n", szFuncName, GetLastError());
		return FALSE;
	}

	pByte = (PBYTE)pFunc;
	if (pByte[0] != 0xE9)
	{
		DebugLog("unhook_by_code() : The API is unhooked already!!!");
		return FALSE;
	}

	if (!VirtualProtect((LPVOID)pFunc, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect))
	{
		DebugLog("unhook_by_code() : VirtualProtect(#1) failed!!! [%d]\n", GetLastError());
		return FALSE;
	}

	memcpy(pFunc, pOrgBytes, 5);

	if (!VirtualProtect((LPVOID)pFunc, 5, dwOldProtect, &dwOldProtect))
	{
		DebugLog("unhook_by_code() : VirtualProtect(#2) failed!!! [%d]\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

/*检测是否Vista以后的系统版本*/
BOOL IsVistaLater()
{
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	if (osvi.dwMajorVersion >= 6)
		return TRUE;
	return FALSE;
}


/* 自定义创建远程线程函数
   主要是根据不同系统版本来创建相应的远程线程
*/
BOOL MyCreateRemoteThread(HANDLE hProcess, LPTHREAD_START_ROUTINE pThreadProc, LPVOID pRemoteBuf)
{
	HANDLE      hThread = NULL;
	FARPROC     pFunc = NULL;
	// Vista, 7, Server2008
	if (IsVistaLater())    
	{
		// GetProcAddress函数检索指定的动态链接库(DLL)中的输出库函数地址。
		// GetModuleHandleA说明:获取一个应用程序或动态链接库的模块句柄
		pFunc = GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtCreateThreadEx");
		if (pFunc == NULL)
		{
			DebugLog("MyCreateRemoteThread() : GetProcAddress() failed!!! [%d]\n", GetLastError());
			return FALSE;
		}
		((PFNTCREATETHREADEX)pFunc)(&hThread, 0x1FFFFF,
			NULL, hProcess, pThreadProc, pRemoteBuf,
			FALSE, NULL, NULL, NULL, NULL);
		if (hThread == NULL)
		{
			DebugLog("MyCreateRemoteThread() : NtCreateThreadEx() failed!!! [%d]\n", GetLastError());
			return FALSE;
		}
	}
	else	// 2000, XP, Server2003
	{
		// CreateRemoteThread是创建一个在其它进程地址空间中运行的线程(也称:创建远程线程).。
		hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, pRemoteBuf, 0, NULL);
		if (hThread == NULL)
		{
			DebugLog("MyCreateRemoteThread() : CreateRemoteThread() failed!!! [%d]\n", GetLastError());
			return FALSE;
		}
	}
	// Windows API函数。当等待仍在挂起状态时，句柄被关闭，那么函数行为是未定义的。
	// 该句柄必须具有 SYNCHRONIZE 访问权限。
	if (WAIT_FAILED == WaitForSingleObject(hThread, INFINITE))
	{
		DebugLog("MyCreateRemoteThread() : WaitForSingleObject() failed!!! [%d]\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

/**
 * Dll注入函数
 * @param dwPID 要注入进程的ID
 * @param szDllPath 要注入的DLL路径
 * @ret 返回注入是否成功
*/
BOOL InjectDll(DWORD dwPID, LPCTSTR szDllPath)
{
	HANDLE                  hProcess = NULL;
	HANDLE                  hThread = NULL;
	LPVOID                  pRemoteBuf = NULL;
	DWORD                   dwBufSize = (DWORD)(_tcslen(szDllPath) + 1) * sizeof(TCHAR);
	LPTHREAD_START_ROUTINE  pThreadProc = NULL;
	BOOL                    bRet = FALSE;
	HMODULE                 hMod = NULL;
	// 1. 打开进程.OpenProcess 函数用来打开一个已存在的进程对象，并返回进程的句柄。
	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
	{
		DebugLog("InjectDll() : OpenProcess(%d) failed!!! [%d]\n", dwPID, GetLastError());
		goto INJECTDLL_EXIT;
	}
	// 2. 申请进程内存空间.
	// VirtualAllocEx 函数的作用是在指定进程的虚拟空间保留或提交内存区域，除非指定MEM_RESET参数，否则将该内存区域置0。
	pRemoteBuf = VirtualAllocEx(hProcess, NULL, dwBufSize, MEM_COMMIT, PAGE_READWRITE);
	if (pRemoteBuf == NULL)
	{
		DebugLog("InjectDll() : VirtualAllocEx() failed!!! [%d]\n", GetLastError());
		goto INJECTDLL_EXIT;
	}
	// 3. 将DLL写入.此函数能写入某一进程的内存区域。入口区必须可以访问，否则操作将失败。
	if (!WriteProcessMemory(hProcess, pRemoteBuf, (LPVOID)szDllPath, dwBufSize, NULL))
	{
		DebugLog("InjectDll() : WriteProcessMemory() failed!!! [%d]\n", GetLastError());
		goto INJECTDLL_EXIT;
	}
	// 4. 获取一个应用程序或动态链接库的模块句柄
	hMod = GetModuleHandle("kernel32.dll");
	if (hMod == NULL)
	{
		DebugLog("InjectDll() : GetModuleHandle() failed!!! [%d]\n", GetLastError());
		goto INJECTDLL_EXIT;
	}
	// 5. GetProcAddress函数检索指定的动态链接库(DLL)中的输出库函数地址
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod, "LoadLibraryW");
	if (pThreadProc == NULL)
	{
		DebugLog("InjectDll() : GetProcAddress() failed!!! [%d]\n", GetLastError());
		goto INJECTDLL_EXIT;
	}
	// 6. CreateRemoteThread是创建一个在其它进程地址空间中运行的线程(也称:创建远程线程).。
	if (!MyCreateRemoteThread(hProcess, pThreadProc, pRemoteBuf))
	{
		DebugLog("InjectDll() : MyCreateRemoteThread() failed!!!\n");
		goto INJECTDLL_EXIT;
	}

	bRet = TRUE;

INJECTDLL_EXIT:

	if (pRemoteBuf)
		VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);

	if (hThread)
		CloseHandle(hThread);

	if (hProcess)
		CloseHandle(hProcess);

	return bRet;
}

/********************************
 * 获取文件的大小
 * @param char* path 文件的路径
 * @ret long size 文件的大小
 ********************************/
unsigned long GetFileSize(LPWSTR path) 
{
	unsigned long filesize = -1L;
	FILE *fp = NULL;
	fp = _wfopen(path, L"r");
	if (fp == NULL)
		return filesize;
	fseek(fp,0L,SEEK_END);
	filesize = ftell(fp);
	fclose(fp);
	fp = NULL;
	return filesize;
}

/*****************************
 * 字符串过滤，去掉换行符号
 * 注意使用完成后将其释放掉
 * @param data 源数据
 * @return 过滤掉换行符的数据
******************************/
char* filter(char* data)
{
	char *tp;
	unsigned int i = 0, j = 0;
	tp = (char*)calloc(strlen(data), sizeof(char));
	while (i < strlen(data)) {
		if (data[i] == '\n' || data[i] == '\r')
			i++;				//过虑掉回车和换行
		else
			tp[j++] = data[i++];//否则记录此数据
	}
	tp[j] = '\0';	//最后的一个结束符
	return tp;		//注意此数据使用完成后要释放掉
}

/*********************************
 * 获取文件的base64字符串
 * @param char* path 文件的路径
 * @retn char* 计算文件的Base64值
 *********************************/
//char* GetBaes64(const char* path) 
char* GetBaes64(LPWSTR path) 
{
	ULONG usize = GetFileSize(path);
	FILE *fp = NULL;
	fp = _wfopen(path, L"rb");
	if (fp == NULL)
		return NULL;
	char* inBuffer = new char[usize];
	INT uReaded = fread(inBuffer, 1, usize, fp);
	const BYTE* pbBinary = (const BYTE *)inBuffer;//图片的二进制数据

	INT needsize = BASE64_Encode(pbBinary,uReaded,NULL);
	//printf("Base64 needs size = %d\n",needsize);
	char *pCrypt1 = new char[needsize+1];
	BASE64_Encode(pbBinary,uReaded,pCrypt1);
	free(inBuffer);//释放掉inBuffer
	inBuffer = NULL; 
	fclose(fp);
	fp = NULL;
	return pCrypt1;//使用完成后，请释放掉
}

BOOL Save2File(char* datas,char* FileName)
{
	if(datas == '\0'){
		return FALSE;
	}
	FILE *fp = fopen(FileName, "w+");
	if (fp==0) {
		return FALSE;
	}
	fwrite(datas, sizeof(char)*(strlen(datas)), 1, fp);
	fclose(fp);
	return TRUE;
}

/*********************************
 * char型转成wchar型
 * @param 
 * @ret 
**********************************/
WCHAR *char2wchar(char* data)
{
	size_t len = strlen(data)+1;
	size_t converted = 0;
	WCHAR *wStr = L"";
	//wStr = (WCHAR*)malloc(len*sizeof(WCHAR));
	//mbstowcs_s(&converted,wStr,len,data,_TRUNCATE);
	return wStr;
}
/****************************
 *	获取当前进程的所在目录
 * @ret LPWSTR 
 ****************************/
LPWSTR GetPath() 
{
	setlocale(LC_CTYPE, "chs");		//引入支持中文库 #include <locale.h>
	LPWSTR data = NULL;
	DWORD dwRet;
	int size = GetCurrentDirectory(0, NULL);
	data = (LPWSTR)malloc(size*sizeof(LPWSTR));
	dwRet = GetCurrentDirectoryW(size, data);
	//DebugLog("FilePath : %S\n", data);
	return data;
}

/**************************************
*	获取当前进程的所在目录下的文件地址
* @ret LPWSTR
***************************************/
LPWSTR GetImageFilePath() 
{
	//宽字符打印用wprintf
	return GetImageFilePath(ImageFile);
}

LPWSTR GetImageFilePath(LPWSTR ImggeName) 
{
	//宽字符打印用wprintf
	return lstrcatW(GetPath(), ImggeName);
}

/**************************************
* wchar字符转换成char型字符函数
* @param wchar_t * 
* @ret char *
***************************************/
char* WcharToChar(const wchar_t* wp)
{
	char *m_char;
	int len = WideCharToMultiByte(CP_ACP, 0, wp, wcslen(wp), NULL, 0, NULL, NULL);
	m_char = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, wp, wcslen(wp), m_char, len, NULL, NULL);
	m_char[len] = '\0';
	return m_char;
}

/**************************************
* char字符转换成wchar型字符函数
* @param char *
* @ret wchar_t *
***************************************/
wchar_t* CharToWchar(const char* c)
{
	wchar_t *m_wchar;
	int len = MultiByteToWideChar(CP_ACP, 0, c, strlen(c), NULL, 0);
	m_wchar = new wchar_t[len + 1];
	MultiByteToWideChar(CP_ACP, 0, c, strlen(c), m_wchar, len);
	m_wchar[len] = '\0';
	return m_wchar;
}

/*******************************************************
* 根据进程名称获取指定进程信息
* 注意最好保证进程名称的唯一性
* @param HANDLE hProcess		进程句柄
* @param HANDLE hModule			模块句柄
* @param PROCESSENTRY32* pinfo	进程
* @param MODULEENTRY32* minfo	模块
* @param LPWSTR processName		进程的名称
* @ret LPWSTR					返回找到进程信息的首址
********************************************************/
LPWSTR GetProcessInfo(HANDLE hProcess, HANDLE hModule, PROCESSENTRY32* pinfo, MODULEENTRY32* minfo, LPWSTR processName)
{
	BOOL report;			//是否遍历结束标志
	LPWSTR shortpath = L""; //保存路径变量
	shortpath = (LPWSTR)malloc(MAX_PATH*sizeof(LPWSTR));//这里要分配以下内存空间
	report = Process32First(hProcess, pinfo);			//获取第一个进程信息
	while (report)
	{
		hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pinfo->th32ProcessID);	//对进程ID构进模块信息
		Module32First(hModule, minfo);													//获取每一个模块信息
		GetShortPathNameW((const unsigned short *)minfo->szExePath, shortpath, 256);	//将查找到的路径赋值到变量shortpath
		if (lstrcmpW((const unsigned short *)pinfo->szExeFile, processName) == 0)		//指定的进程
		{
			//DebugLog("shortpath:%S", shortpath);
			break;
		}
		report = Process32Next(hProcess, pinfo); //继续查找下一个进程
	}
	return shortpath;
}

/***************************************************
 * 获取当前进程的地址，注意此地址是保含了图片名称的
 * @param	LPWSTR processName 进程名称
 * @ret		LPWSTR 返回当前进程的信息首址
****************************************************/
LPWSTR GetCurrentProcessPath(LPWSTR processName)
{
	HANDLE hModule = NULL; //模块句柄
	LPWSTR path = L"";
	HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //初始化进程快照
	PROCESSENTRY32* pinfo = new PROCESSENTRY32;		//进程信息描述
	pinfo->dwSize = sizeof(PROCESSENTRY32);			//给定空间大小
	MODULEENTRY32* minfo = new MODULEENTRY32;		//模块信息描述
	minfo->dwSize = sizeof(MODULEENTRY32);			//给定空间大小

	path = GetProcessInfo(hProcess, hModule, pinfo, minfo, processName); //主体处理

	CloseHandle(hProcess);	//关闭进程句柄 
	CloseHandle(hModule);	//关闭模块句柄
	//过滤掉文件名称
	lstrcpynW(path, path, lstrlen((const char *)path) - lstrlen((const char *)processName) + 1);
	path = lstrcatW(path, ImageFile);//生成新的图片路径
	//DebugLog("image_path:%S",path);
	return path;
}

