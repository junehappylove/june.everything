// CameraPhoto.cpp : CCameraPhotoApp 和 DLL 注册的实现。

#include "stdafx.h"
#include "CameraPhoto.h"

#include "comcat.h"
#include "strsafe.h"
#include "objsafe.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CCameraPhotoApp theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0x54A399EC, 0x2329, 0x4EEC, { 0x86, 0xE3, 0x73, 0x0, 0x47, 0x11, 0x95, 0x9 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;

// CLSID_SafeItem - Necessary for safe ActiveX control
const CATID CLSID_SafeItem =
{0x28996908, 0x1121, 0x40f4,{ 0xa8, 0x39, 0x3d, 0x85, 0x10, 0xba, 0x94, 0xd9 }};

// CCameraPhotoApp::InitInstance - DLL 初始化

BOOL CCameraPhotoApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();

	if (bInit)
	{
		// TODO: 在此添加您自己的模块初始化代码。
	}

	return bInit;
}



// CCameraPhotoApp::ExitInstance - DLL 终止

int CCameraPhotoApp::ExitInstance()
{
	// TODO: 在此添加您自己的模块终止代码。

	return COleControlModule::ExitInstance();
}

// HRESULT CreateComponentCategory - Used to register ActiveX control as safe
HRESULT CreateComponentCategory(CATID catid, WCHAR *catDescription)
{
    ICatRegister *pcr = NULL ;
    HRESULT hr = S_OK ;

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
            NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
    if (FAILED(hr))
        return hr;

    // Make sure the HKCR\Component Categories\{..catid...}
    // key is registered.
    CATEGORYINFO catinfo;
    catinfo.catid = catid;
    catinfo.lcid = 0x0409 ; // english
    size_t len;
    // Make sure the provided description is not too long.
    // Only copy the first 127 characters if it is.
    // The second parameter of StringCchLength is the maximum
    // number of characters that may be read into catDescription.
    // There must be room for a NULL-terminator. The third parameter
    // contains the number of characters excluding the NULL-terminator.
	hr = StringCchLengthW(catDescription, STRSAFE_MAX_CCH, &len);
	if (SUCCEEDED(hr))
	   {
        if (len>127)
		   {
           len = 127;
		   }
		}   
    else
	    {
		// TODO: Write an error handler;
		}
	// The second parameter of StringCchCopy is 128 because you need 
    // room for a NULL-terminator.    
	hr = StringCchCopyW(catinfo.szDescription, len + 1, 
           catDescription);
	// Make sure the description is null terminated.
        catinfo.szDescription[len + 1] = '\0';

    hr = pcr->RegisterCategories(1, &catinfo);
        pcr->Release();

    return hr;
}

// HRESULT RegisterCLSIDInCategory - Register your component categories information
HRESULT RegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
// Register your component categories information.
    ICatRegister *pcr = NULL ;
    HRESULT hr = S_OK ;
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
                NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
    if (SUCCEEDED(hr))
    {
       // Register this category as being "implemented" by the class.
       CATID rgcatid[1] ;
       rgcatid[0] = catid;
       hr = pcr->RegisterClassImplCategories(clsid, 1, rgcatid);
    }

    if (pcr != NULL)
        pcr->Release();
		
    return hr;
}


// DllRegisterServer - 将项添加到系统注册表
STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);

	HRESULT hr;    // HResult used by Safety Functions

	// Mark the control as safe for initializing.
	hr = CreateComponentCategory(CATID_SafeForInitializing, L"Controls safely initializable from persistent data!");
	if (FAILED(hr))
        return hr;

    hr = RegisterCLSIDInCategory(CLSID_SafeItem, CATID_SafeForInitializing);
    if (FAILED(hr))
        return hr;

    // Mark the control as safe for scripting.
	hr = CreateComponentCategory(CATID_SafeForScripting, L"Controls safely scriptable!");
    if (FAILED(hr))
        return hr;

    hr = RegisterCLSIDInCategory(CLSID_SafeItem, CATID_SafeForScripting);
    if (FAILED(hr))
        return hr;

	return NOERROR;
}

// HRESULT UnRegisterCLSIDInCategory - Remove entries from the registry
HRESULT UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
    ICatRegister *pcr = NULL ;
    HRESULT hr = S_OK ;

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
            NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
    if (SUCCEEDED(hr))
    {
       // Unregister this category as being "implemented" by the class.
       CATID rgcatid[1] ;
       rgcatid[0] = catid;
       hr = pcr->UnRegisterClassImplCategories(clsid, 1, rgcatid);
    }

    if (pcr != NULL)
        pcr->Release();

    return hr;
}

// DllUnregisterServer - 将项从系统注册表中移除
STDAPI DllUnregisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);

	HRESULT hr;    // HResult used by Safety Functions

	// Remove entries from the registry.
	hr = UnRegisterCLSIDInCategory(CLSID_SafeItem, CATID_SafeForInitializing);
	if (SUCCEEDED(hr))
		hr = UnRegisterCLSIDInCategory(CLSID_SafeItem, CATID_SafeForScripting); 

	return NOERROR;
}
