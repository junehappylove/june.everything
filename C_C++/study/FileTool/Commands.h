// Commands.h : header file
//

#if !defined(AFX_COMMANDS_H__33A4EAD6_A4DA_11D2_91A2_00A0C9D136D1__INCLUDED_)
#define AFX_COMMANDS_H__33A4EAD6_A4DA_11D2_91A2_00A0C9D136D1__INCLUDED_

#include "FileToolTypes.h"

class CCommands : 
	public CComDualImpl<ICommands, &IID_ICommands, &LIBID_FileTool>, 
	public CComObjectRoot,
	public CComCoClass<CCommands, &CLSID_Commands>
{
protected:
	IApplication* m_pApplication;

public:
	CCommands();
	~CCommands();
	void SetApplicationObject(IApplication* m_pApplication);

	BEGIN_COM_MAP(CCommands)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(ICommands)
	END_COM_MAP()
	DECLARE_NOT_AGGREGATABLE(CCommands)

public:
	STDMETHOD(OpenFile)(THIS);
	STDMETHOD(AddFileToProject)(THIS);
};

typedef CComObject<CCommands> CCommandsObj;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMANDS_H__33A4EAD6_A4DA_11D2_91A2_00A0C9D136D1__INCLUDED)
