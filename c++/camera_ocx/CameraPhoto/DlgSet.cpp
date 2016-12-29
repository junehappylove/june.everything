// DlgSet.cpp : 实现文件
//

#include "stdafx.h"
#include "CameraPhoto.h"
#include "DlgSet.h"


// CDlgSet 对话框

IMPLEMENT_DYNAMIC(CDlgSet, CDialog)

CDlgSet::CDlgSet(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSet::IDD, pParent)
	, m_nScale(0)
	, m_nWidth(0)
	, m_nHeight(0)
{
	if (!GetParm())
	{
		m_nScale = 50;
		m_nWidth = 201;
		m_nHeight = 201;
	}

	//UpdateData(false);
}

CDlgSet::~CDlgSet()
{
}

void CDlgSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SCALE, m_nScale);
	DDV_MinMaxLong(pDX, m_nScale, 1, 1000);
	DDX_Text(pDX, IDC_WIDTH, m_nWidth);
	DDX_Text(pDX, IDC_HEIGHT, m_nHeight);
}


BEGIN_MESSAGE_MAP(CDlgSet, CDialog)
	ON_BN_CLICKED(IDOK, &CDlgSet::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgSet 消息处理程序

bool CDlgSet::GetParm()
{
	HKEY hKey;
	DWORD dwBufLen=sizeof(int);
	LONG lRet;
	DWORD dwType=0;

	lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,L"SOFTWARE\\ICFK\\CameraPhoto\\Parm", 0, KEY_READ, &hKey );
	if( lRet != ERROR_SUCCESS )
		return FALSE;

	lRet = RegQueryValueEx( hKey, TEXT("Scale"),NULL, &dwType, (LPBYTE)&m_nScale, &dwBufLen);
	if( lRet != ERROR_SUCCESS )
		return FALSE;

	lRet = RegQueryValueEx( hKey, TEXT("PictureWidth"),NULL, &dwType, (LPBYTE)&m_nWidth, &dwBufLen);
	if( lRet != ERROR_SUCCESS )
		return FALSE;

	lRet = RegQueryValueEx( hKey, TEXT("PictureHeight"),NULL, &dwType,(LPBYTE) &m_nHeight, &dwBufLen);
	if( lRet != ERROR_SUCCESS )
		return FALSE;

	RegCloseKey( hKey );

	return TRUE;
}

bool CDlgSet::SetParm(void)
{
	HKEY hKey;
	DWORD dwBufLen=sizeof(int);
	//DWORD dwDisp;
	LONG lRet;

	UpdateData(true);

	DWORD	dwDisposition;

	lRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE,L"SOFTWARE\\ICFK\\CameraPhoto\\Parm",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,&dwDisposition);
	if(lRet != ERROR_SUCCESS){
		CString strErr;
		strErr.Format(L"ErrCode is:%d",lRet);
		MessageBox(strErr);

		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,NULL,lRet,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR)&lpMsgBuf,0,NULL );
		MessageBox((LPCTSTR)lpMsgBuf);
		LocalFree( lpMsgBuf );
		return FALSE;
	}

	lRet = RegSetValueEx( hKey, L"Scale",0, REG_DWORD,(LPBYTE) &m_nScale, dwBufLen);
	if( lRet != ERROR_SUCCESS )
		return FALSE;

	lRet = RegSetValueEx( hKey, L"PictureWidth",0, REG_DWORD,(LPBYTE) &m_nWidth, dwBufLen);
	if( lRet != ERROR_SUCCESS )
		return FALSE;

	//MessageBox(L"PictureWidth OK");

	lRet = RegSetValueEx( hKey, L"PictureHeight",0,REG_DWORD, (LPBYTE)&m_nHeight, dwBufLen);
	if( lRet != ERROR_SUCCESS )
		return FALSE;

	//MessageBox(L"PictureHeight OK");

	RegCloseKey( hKey );

	return TRUE;
}

void CDlgSet::OnBnClickedOk()
{
	if (SetParm())
		OnOK();
}
