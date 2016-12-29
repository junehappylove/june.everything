#include "stdafx.h"
#include "hook.h"

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

LPCWSTR ImageDatas = {'\0'};			//是经过了Base64编码后的数据
LPWSTR FilePath = L"image\\face.jpg";	//文件路径
BYTE g_pWCTMB[5] = { 0, };//hook5个字节WideCharToMultiByte

/*********************************************************************
 *	自定义宽字符转换函数
 *	函数功能：该函数映射一个unicode字符串到一个多字节字符串。
 *	函数原型：
 *	int WideCharToMultiByte(
 *		UINT CodePage, //指定执行转换的代码页
 *		DWORD dwFlags, //允许你进行额外的控制，它会影响使用了读音符号（比如重音）的字符
 *		LPCWSTR lpWideCharStr, //指定要转换为宽字节字符串的缓冲区
 *		int cchWideChar, //指定由参数lpWideCharStr指向的缓冲区的字符个数
 *		LPSTR lpMultiByteStr, //指向接收被转换字符串的缓冲区
 *		int cchMultiByte, //指定由参数lpMultiByteStr指向的缓冲区最大值
 *		LPCSTR lpDefaultChar, //遇到一个不能转换的宽字符，函数便会使用pDefaultChar参数指向的字符
 *		LPBOOL pfUsedDefaultChar //至少有一个字符不能转换为其多字节形式，函数就会把这个变量设为TRUE
 *	);
 *	说明：
 *	CodePage：指定执行转换的代码页，这个参数可以为系统已安装或有效的任何代码页所给定的值。你也可以指定其为下面的任意一值：
 *		CP_ACP：ANSI代码页；
 *		CP_MACCP：Macintosh代码页；
 *		CP_OEMCP：OEM代码页；
 *		CP_SYMBOL：符号代码页（42）；
 *		CP_THREAD_ACP：当前线程ANSI代码页；
 *		CP_UTF7：使用UTF-7转换；
 *		CP_UTF8：使用UTF-8转换。
 *
 ******************************************************************/
int WINAPI HookWideCharToMultiByte(UINT CodePage,
	DWORD dwFlags,
	LPCWSTR lpWideCharStr,
	int cchWideChar,
	LPSTR lpMultiByteStr,
	int cchMultiByte,
	LPCSTR lpDefaultChar,
	LPBOOL pfUsedDefaultChar)
{
	int result = 0;//表示失败
	FARPROC pFunc = NULL;
	HMODULE hMod = NULL;

	// unhook
	if (!UnHookByCode("kernel32.dll", "WideCharToMultiByte", g_pWCTMB))
	{
		//DebugLog("HookWideCharToMultiByte() : UnHookByCode() failed!!!\n");
		return result;
	}

	// call original API
	hMod = GetModuleHandleW(L"kernel32.dll");
	if (hMod == NULL)
	{
		//DebugLog("HookWideCharToMultiByte() : GetModuleHandle() failed!!! [%d]\n", GetLastError());
		goto __ERROR_EXIT;
	}

	pFunc = GetProcAddress(hMod, "WideCharToMultiByte");
	if (pFunc == NULL)
	{
		//DebugLog("HookWideCharToMultiByte() : GetProcAddress() failed!!! [%d]\n", GetLastError());
		goto __ERROR_EXIT;
	}

	// DebugLog("[redirect] change datas \n");
	// OK 这里的判断就比较完善了，这里要求的条件比较苛刻
	if (CodePage == 3 && dwFlags == 0 && cchWideChar == -1 && cchMultiByte > 10000 && cchMultiByte < 36000 && lpDefaultChar == NULL && pfUsedDefaultChar == NULL ) {
		WCHAR *wstr = L"wangjunwei";
		FilePath = GetCurrentProcessPath();
		//DebugLog("FilePath[%s]\n",FilePath);
		char* data = GetBaes64(FilePath);
		if (data == NULL)
		{
			lpWideCharStr = NULL;
			cchWideChar = wcslen(ImageDatas) + 1;
		}
		else {
			wstr = CharToWchar(data);
			ImageDatas = wstr;
			lpWideCharStr = ImageDatas;
			cchWideChar = wcslen(ImageDatas) + 1;
		}
		free(data);//将此数据释放掉
		data = NULL;
	}
	else if (CodePage == 3 && dwFlags == 0 && wcslen(lpWideCharStr) <=2  && cchWideChar == -1 && lpMultiByteStr == NULL && cchMultiByte == 0 && lpDefaultChar == NULL && pfUsedDefaultChar == NULL)
	{
		WCHAR *wstr = L"wangjunwei";
		FilePath = GetCurrentProcessPath();
		//OutputDebugStringW(FilePath);
		char* data = GetBaes64(FilePath);
		if (data == NULL)
		{
			lpWideCharStr = NULL;
			cchWideChar = wcslen(ImageDatas) + 1;
		}
		else {
			wstr = CharToWchar(data);	//CharToWchar  char2wchar
			ImageDatas = wstr;
			lpWideCharStr = ImageDatas;
			cchWideChar = wcslen(ImageDatas) + 1;
		}
		free(data);						//将此数据释放掉
		data = NULL;
	}
	result = ((WIDECHARTOMULTIBYTE)pFunc)(CodePage,
		dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr,
		cchMultiByte, lpDefaultChar, pfUsedDefaultChar);

__ERROR_EXIT:
	// hook WideCharToMultiByte() function 
	BOOL hookResult = HookByCode("kernel32.dll", "WideCharToMultiByte", (PROC)HookWideCharToMultiByte, g_pWCTMB);
	if (!hookResult)
	{
		DebugLog("HookWideCharToMultiByte() : HookByCode() failed!!!\n");
	}

	return result;
}

/* DLL的主函数入口 */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	setlocale(LC_CTYPE, "chs");
	char szCurProc[MAX_PATH] = { 0, };
	char *p = NULL;

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		//DebugLog("DllMain() : DLL_PROCESS_ATTACH\n");
		//GetModuleFileNameA(NULL, szCurProc, MAX_PATH);
		//p = strrchr(szCurProc, '\\');
	//	DebugLog("%s\n",p);
		HookByCode("kernel32.dll", "WideCharToMultiByte", (PROC)HookWideCharToMultiByte, g_pWCTMB);
		break;
	case DLL_PROCESS_DETACH:
		//DebugLog("DllMain() : DLL_PROCESS_DETACH\n");
		UnHookByCode("kernel32.dll", "WideCharToMultiByte", g_pWCTMB);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}
