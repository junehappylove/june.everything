// VidTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include <vfw.h>
#include <afxole.h>
#include "VidTest.h"
#include "VidTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL Capture(CVFWImageProcessor &ImageProc, LPCTSTR Filename);

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
   CAboutDlg();

// Dialog Data
   //{{AFX_DATA(CAboutDlg)
   enum
   {
      IDD = IDD_ABOUTBOX
   };
   //}}AFX_DATA

   // ClassWizard generated virtual function overrides
   //{{AFX_VIRTUAL(CAboutDlg)
protected:
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   //}}AFX_VIRTUAL

// Implementation
protected:
   //{{AFX_MSG(CAboutDlg)
   //}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
   //{{AFX_DATA_INIT(CAboutDlg)
   //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CAboutDlg)
   //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
//{{AFX_MSG_MAP(CAboutDlg)
// No message handlers
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVidTestDlg dialog

CVidTestDlg::CVidTestDlg(CWnd* pParent /*=NULL*/)
: CDialog(CVidTestDlg::IDD, pParent)
{
   //{{AFX_DATA_INIT(CVidTestDlg)
   //}}AFX_DATA_INIT
   // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
   m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVidTestDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CVidTestDlg)
   DDX_Control(pDX, IDC_VIDEO, m_VideoDisplay);
   //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVidTestDlg, CDialog)
//{{AFX_MSG_MAP(CVidTestDlg)
ON_WM_SYSCOMMAND()
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_BN_CLICKED(IDCAPTURE, OnCapture)
   ON_COMMAND(ID_SETTINGS_VIDEODISPLAY, OnSettingsVideodisplay)
   ON_COMMAND(ID_SETTINGS_VIDEOFORMAT, OnSettingsVideoformat)
   ON_COMMAND(ID_SETTINGS_VIDEOSOURCE, OnSettingsVideosource)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVidTestDlg message handlers

BOOL CVidTestDlg::OnInitDialog()
{
   CDialog::OnInitDialog();

   // Add "About..." menu item to system menu.

   // IDM_ABOUTBOX must be in the system command range.
   ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
   ASSERT(IDM_ABOUTBOX < 0xF000);

   CMenu* pSysMenu = GetSystemMenu(FALSE);
   if (pSysMenu != NULL)
   {
      CString strAboutMenu;
      strAboutMenu.LoadString(IDS_ABOUTBOX);
      if (!strAboutMenu.IsEmpty())
      {
         pSysMenu->AppendMenu(MF_SEPARATOR);
         pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
      }
   }

   // Set the icon for this dialog.  The framework does this automatically
   //  when the application's main window is not a dialog
   SetIcon(m_hIcon, TRUE);       // Set big icon
   SetIcon(m_hIcon, FALSE);      // Set small icon

   CRect Rect;
   CAPDRIVERCAPS Caps;
   CMenu *Menu;

   m_VideoDisplay.GetWindowRect(Rect);
   ScreenToClient(Rect);

   m_VFWImageProc.Initialize();
   m_VFWImageProc.DriverGetCaps(&Caps);

   Menu = GetMenu();

   if (!Caps.fHasDlgVideoSource)
      Menu->EnableMenuItem(ID_SETTINGS_VIDEOSOURCE,MF_GRAYED);

   if (!Caps.fHasDlgVideoFormat)
      Menu->EnableMenuItem(ID_SETTINGS_VIDEOFORMAT,MF_GRAYED);

   if (!Caps.fHasDlgVideoDisplay)
      Menu->EnableMenuItem(ID_SETTINGS_VIDEODISPLAY,MF_GRAYED);

   m_VFWImageProc.EnablePreviewVideo(*this, Rect.TopLeft().x,Rect.TopLeft().y);

   return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVidTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
   if ((nID & 0xFFF0) == IDM_ABOUTBOX)
   {
      CAboutDlg dlgAbout;
      dlgAbout.DoModal();
   }
   else
   {
      CDialog::OnSysCommand(nID, lParam);
   }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVidTestDlg::OnPaint()
{
   if (IsIconic())
   {
      CPaintDC dc(this); // device context for painting

      SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

      // Center icon in client rectangle
      int cxIcon = GetSystemMetrics(SM_CXICON);
      int cyIcon = GetSystemMetrics(SM_CYICON);
      CRect rect;
      GetClientRect(&rect);
      int x = (rect.Width() - cxIcon + 1) / 2;
      int y = (rect.Height() - cyIcon + 1) / 2;

      // Draw the icon
      dc.DrawIcon(x, y, m_hIcon);
   }
   else
   {
      CDialog::OnPaint();
   }
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVidTestDlg::OnQueryDragIcon()
{
   return(HCURSOR) m_hIcon;
}

//拍照按钮事件
void CVidTestDlg::OnCapture()
{
   CString Filter;
   CString Filename;
   CRect Rect;

   Filter = "Bitmap Files (*.bmp)|*.bmp|AVI Files (*.avi)|*.avi||";

   CFileDialog FileDlg(FALSE, "BMP", NULL,
                       OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                       Filter,
                       this);

   if (FileDlg.DoModal() == IDOK)
   {
      RedrawWindow();

      Filename = FileDlg.GetPathName();

      Capture(m_VFWImageProc, Filename);

      m_VideoDisplay.GetWindowRect(Rect);
      ScreenToClient(Rect);

      m_VFWImageProc.EnablePreviewVideo(*this, Rect.TopLeft().x,Rect.TopLeft().y);
   }
}


//拍照函数调用
BOOL Capture(CVFWImageProcessor &ImageProc, LPCTSTR Filename)
{
   ULONG Length = 0;
   CFile File;
   BOOL Ret = FALSE;
   CHAR Ext[_MAX_EXT];

   _splitpath(Filename,NULL,NULL,NULL,Ext);

   if (stricmp(Ext,".avi") == 0)
   {
      Ret = ImageProc.CaptureAVI(Filename,4.0,10,10);
   }
   else if (stricmp(Ext,".bmp") == 0)
   {
      BITMAPINFO *Bitmap = NULL;
      BITMAPFILEHEADER bfh;

      ImageProc.CaptureDIB(&Bitmap,0,&Length);

      if (Bitmap)
      {
         File.Open(Filename,CFile::modeCreate |
                                    CFile::modeWrite |
                                    CFile::shareDenyNone |
                                    CFile::typeBinary);

         bfh.bfType = 0x4d42;    // 0x42 = "B" 0x4d = "M"
         bfh.bfSize = (DWORD) Length + sizeof(BITMAPFILEHEADER);
         bfh.bfOffBits = (DWORD)   sizeof(BITMAPFILEHEADER) +
                                   sizeof(BITMAPINFOHEADER) +
                                    Bitmap->bmiHeader.biClrUsed * sizeof (RGBQUAD);
         bfh.bfReserved1 = 0;
         bfh.bfReserved2 = 0;

         File.Write(&bfh,sizeof(bfh));
         File.Write(Bitmap,Length);
         File.Close();

         Ret = TRUE;

         delete Bitmap;
      }
   }

   return Ret;
}

void CVidTestDlg::OnSettingsVideodisplay()
{
   m_VFWImageProc.DlgVideoDisplay();
}

void CVidTestDlg::OnSettingsVideoformat()
{
   m_VFWImageProc.DlgVideoFormat();
}

void CVidTestDlg::OnSettingsVideosource()
{
   m_VFWImageProc.DlgVideoSource();
}

void CVidTestDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
   m_VFWImageProc.Destroy();

	CDialog::OnCancel();
}
