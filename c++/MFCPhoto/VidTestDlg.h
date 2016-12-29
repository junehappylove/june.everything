// VidTestDlg.h : header file
//

#if !defined(AFX_VIDTESTDLG_H__8E0613F7_4C4C_11D4_9EE9_000083A26E84__INCLUDED_)
#define AFX_VIDTESTDLG_H__8E0613F7_4C4C_11D4_9EE9_000083A26E84__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VFWImageProcessor.h"

/////////////////////////////////////////////////////////////////////////////
// CVidTestDlg dialog

class CVidTestDlg : public CDialog
{
// Construction
public:
   HWND m_hWndVideo;
   CVidTestDlg(CWnd* pParent = NULL);  // standard constructor

// Dialog Data
   //{{AFX_DATA(CVidTestDlg)
   enum { IDD = IDD_VIDTEST_DIALOG };
   CStatic  m_VideoDisplay;
   //}}AFX_DATA

   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CVidTestDlg)
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);   // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:
   HICON m_hIcon;

   // Generated message map functions
   //{{AFX_MSG(CVidTestDlg)
   virtual BOOL OnInitDialog();
   afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
   afx_msg void OnPaint();
   afx_msg HCURSOR OnQueryDragIcon();
   afx_msg void OnCapture();
   afx_msg void OnSettingsVideodisplay();
   afx_msg void OnSettingsVideoformat();
   afx_msg void OnSettingsVideosource();
	virtual void OnCancel();
	//}}AFX_MSG
   DECLARE_MESSAGE_MAP()

public:
   CVFWImageProcessor m_VFWImageProc;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDTESTDLG_H__8E0613F7_4C4C_11D4_9EE9_000083A26E84__INCLUDED_)
