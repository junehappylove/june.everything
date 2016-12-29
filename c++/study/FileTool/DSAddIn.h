// DSAddIn.h : header file
//

#if !defined(AFX_DSADDIN_H__33A4EAD4_A4DA_11D2_91A2_00A0C9D136D1__INCLUDED_)
#define AFX_DSADDIN_H__33A4EAD4_A4DA_11D2_91A2_00A0C9D136D1__INCLUDED_

#include "commands.h"

// {33A4EAC1-A4DA-11D2-91A2-00A0C9D136D1}
DEFINE_GUID(CLSID_DSAddIn,
0x33a4eac1, 0xa4da, 0x11d2, 0x91, 0xa2, 0, 0xa0, 0xc9, 0xd1, 0x36, 0xd1);

/////////////////////////////////////////////////////////////////////////////
// CDSAddIn

class CDSAddIn : 
	public IDSAddIn,
	public CComObjectRoot,
	public CComCoClass<CDSAddIn, &CLSID_DSAddIn>
{
public:
	DECLARE_REGISTRY(CDSAddIn, "FileTool.DSAddIn.1",
		"FILETOOL Developer Studio Add-in", IDS_FILETOOL_LONGNAME,
		THREADFLAGS_APARTMENT)

	CDSAddIn() {}
	BEGIN_COM_MAP(CDSAddIn)
		COM_INTERFACE_ENTRY(IDSAddIn)
	END_COM_MAP()
	DECLARE_NOT_AGGREGATABLE(CDSAddIn)

// IDSAddIns
public:
	STDMETHOD(OnConnection)(THIS_ IApplication* pApp, VARIANT_BOOL bFirstTime,
		long dwCookie, VARIANT_BOOL* OnConnection);
	STDMETHOD(OnDisconnection)(THIS_ VARIANT_BOOL bLastTime);

protected:
	CCommandsObj* m_pCommands;
	DWORD m_dwCookie;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSADDIN_H__33A4EAD4_A4DA_11D2_91A2_00A0C9D136D1__INCLUDED)
