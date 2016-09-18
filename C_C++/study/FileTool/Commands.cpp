// Commands.cpp : implementation file
//

#include "stdafx.h"
#include "FileTool.h"
#include "Commands.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCommands

CCommands::CCommands()
{
	m_pApplication = NULL;
}

CCommands::~CCommands()
{
	ASSERT (m_pApplication != NULL);
	m_pApplication->Release();
}

void CCommands::SetApplicationObject(IApplication* pApplication)
{
	// This function assumes pApplication has already been AddRef'd
	//  for us, which CDSAddIn did in its QueryInterface call
	//  just before it called us.
	m_pApplication = pApplication;
}

/////////////////////////////////////////////////////////////////////////////
// CCommands methods

STDMETHODIMP CCommands::AddFileToProject() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: Replace this with the actual code to execute this command
	//  Use m_pApplication to access the Developer Studio Application object,
	//  and VERIFY_OK to see error strings in DEBUG builds of your add-in
	//  (see stdafx.h)

	VERIFY_OK(m_pApplication->EnableModeless(VARIANT_FALSE));

	HRESULT hr;
	CComPtr<IDispatch> m_pDispatch(NULL);
	hr=m_pApplication->get_Projects(&m_pDispatch);
	if(FAILED(hr))
	{
	    AfxMessageBox("Unable to obtain the IProject's pointer");
	    return S_FALSE;
	}

	CComQIPtr<IProjects, &IID_IProjects> m_Project(m_pDispatch);

	long count;
	hr=m_Project->get_Count(&count);
	if(FAILED(hr))
	{
	    AfxMessageBox("Unable to obtain the number of projects in the workspace");
	    return S_FALSE;
	}

	if(count > 0)
	{
	    CFileDialog AddFiles(TRUE);
	    AddFiles.m_ofn.lpstrTitle = "Add Files To Project";
	    AddFiles.m_ofn.lpstrFilter = "All Files(*.*)\0*.*\0\0";
	    AddFiles.m_ofn.Flags =  AddFiles.m_ofn.Flags | OFN_ALLOWMULTISELECT;
	    char test[2000];
	    test[0]='\0';
	    AddFiles.m_ofn.lpstrFile = test;
	    AddFiles.m_ofn.nMaxFile = 2000;

	    if(AddFiles.DoModal() == IDOK)
	    {
		POSITION position=AddFiles.GetStartPosition();
		CString file_name;

		m_pDispatch=NULL;
	        hr=m_pApplication->get_ActiveProject(&m_pDispatch);
		if(FAILED(hr))
		{
		    AfxMessageBox("Unable to obtain the ActiveProject pointer");
		    return S_FALSE;
		}

		CComQIPtr<IGenericProject, &IID_IGenericProject> m_ActiveProject(m_pDispatch);
		CComBSTR type;

		hr=m_ActiveProject->get_Type(&type);
		if(FAILED(hr))
		{
		    AfxMessageBox("Unable to obtain the ActiveProject type");
		    return S_FALSE;
		}
		CString s_type(type);
		CComQIPtr<IBuildProject, &IID_IBuildProject> m_BuildProject;
		m_BuildProject=m_ActiveProject;

		CComVariant reserved=NULL;

		while(position != NULL)
		{
		    file_name=AddFiles.GetNextPathName(position);
		    if(s_type == "Build")
		    {
			    hr=m_BuildProject->AddFile(CComBSTR(file_name), reserved);
			    if(FAILED(hr))
			    {
				AfxMessageBox("Unable to add file to the ActiveProject");
				return S_FALSE;
			    }
		    }
		    else
		    {
			    AfxMessageBox("Error: Active project is not a Build Project!");
		    }
		}
	    }
	}
	else
		AfxMessageBox("There are no loaded projects");

	VERIFY_OK(m_pApplication->EnableModeless(VARIANT_TRUE));
	return S_OK;
}

STDMETHODIMP CCommands::OpenFile() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

        CFileDialog OpenFiles(TRUE);
	OpenFiles.m_ofn.lpstrTitle = "Open File";
	OpenFiles.m_ofn.lpstrFilter = "All Files(*.*)\0*.*\0\0";
	OpenFiles.m_ofn.Flags =  OpenFiles.m_ofn.Flags | OFN_ALLOWMULTISELECT;
	char test[2000];
	test[0]='\0';
	OpenFiles.m_ofn.lpstrFile = test;
	OpenFiles.m_ofn.nMaxFile = 2000;

	HRESULT hr;

	if(OpenFiles.DoModal() == IDOK)
	{
		POSITION position=OpenFiles.GetStartPosition();
		CString file_name;

		CComPtr<IDispatch> m_pDispatch(NULL);
		hr=m_pApplication->get_Documents(&m_pDispatch);
		if(FAILED(hr))
		{
		    AfxMessageBox("Unable to obtain the IDocument's pointer");
		    return S_FALSE;
		}
		CComQIPtr<IDocuments, &IID_IDocuments> m_Documents(m_pDispatch);

		CComVariant doc_type="Auto";
		CComVariant writable=FALSE;

		while(position != NULL)
		{
		    file_name=OpenFiles.GetNextPathName(position);
		    m_pDispatch=NULL;
		    hr=m_Documents->Open(CComBSTR(file_name),doc_type,writable,&m_pDispatch);
		    if(FAILED(hr))
		    {
			AfxMessageBox("Unable to obtain open the file");
			return S_FALSE;
		    }
		}
	}
	return S_OK;
}
