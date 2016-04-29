// VidTest.h : main header file for the VIDTEST application
//

#if !defined(AFX_VIDTEST_H__8E0613F5_4C4C_11D4_9EE9_000083A26E84__INCLUDED_)
#define AFX_VIDTEST_H__8E0613F5_4C4C_11D4_9EE9_000083A26E84__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
   #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"      // main symbols

/////////////////////////////////////////////////////////////////////////////
// CVidTestApp:
// See VidTest.cpp for the implementation of this class
//

class CVidTestApp : public CWinApp
{
public:
   CVidTestApp();

// Overrides
   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CVidTestApp)
   public:
   virtual BOOL InitInstance();
   //}}AFX_VIRTUAL

// Implementation

   //{{AFX_MSG(CVidTestApp)
      // NOTE - the ClassWizard will add and remove member functions here.
      //    DO NOT EDIT what you see in these blocks of generated code !
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIDTEST_H__8E0613F5_4C4C_11D4_9EE9_000083A26E84__INCLUDED_)
