
// CMainDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "CameraPhoto.h"
#include "MainDlg.h"
#include "DlgSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainDlg 对话框

IMPLEMENT_DYNAMIC(CMainDlg, CDialog)

CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMainDlg::IDD, pParent)
	, m_id(_T(""))
	, m_pWidth(201)
	, m_pHeight(201)
	, m_nScale(100)
	, m_pLeft(0)
	, m_pTop(0)
	, m_DefScale(0)
{
}

CMainDlg::~CMainDlg()
{
	
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VEDIO, m_ctl_vedio);
	DDX_Control(pDX, IDC_IMAGE, m_ctl_image);
	DDX_Control(pDX, IDC_PHOTO, m_ctl_photo);
	DDX_Control(pDX, IDC_STATICGP1, m_sxtgp);
	DDX_Text(pDX, IDC_ID, m_id);
	DDX_Text(pDX, IDC_EDIT_WIDTH, m_pWidth);
	DDV_MinMaxInt(pDX, m_pWidth, 1, 65536);
	DDX_Text(pDX, IDC_EDIT_HEIGHT, m_pHeight);
	DDV_MinMaxInt(pDX, m_pHeight, 1, 65536);
}

BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_SYSCOMMAND()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_BN_CLICKED(IDC_BUTTON1, &CMainDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMainDlg::OnBnClickedButton2)
	ON_STN_CLICKED(IDC_IMAGE, &CMainDlg::OnStnClickedImage)
	ON_BN_CLICKED(IDC_SET, &CMainDlg::OnBnClickedSet)
END_MESSAGE_MAP()


// CMainDlg 消息处理程序

BOOL CMainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: 在此添加额外的初始化代码
	this->m_ctl_vedio.GetClientRect(&(this->m_VedioRect));
	this->m_ctl_image.GetClientRect(&(this->m_ImageRect));
	this->m_ctl_photo.GetClientRect(&(this->m_PhotoRect));

	//m_PicEditStatus = false;
	//this->m_Factor_Radio = PICZOOM_RADIO;
  
	if( m_VMRCap.EnumDevices()>0 )
	{
		m_VMRCap.InitMaxDPI(0,m_ctl_vedio.GetSafeHwnd());
		this->m_VMRRect.SetRect(0,0,m_VMRCap.m_nWidth,m_VMRCap.m_nHeight);
		CString title,title1;
		this->m_sxtgp.GetWindowText(title1);
		title.Format(_T("%s(%d*%d)"),title1,m_VMRCap.m_nWidth,m_VMRCap.m_nHeight);
		this->m_sxtgp.SetWindowText(title.GetBuffer());
	}

	//m_pWidth = 201;
	//m_pHeight = 201;
	if (!GetParm())
	{
		m_pWidth = 201;
		m_pHeight = 201;
		m_DefScale = 50;
	}

	m_CutRect.left = (m_ImageRect.Width() - m_pWidth)/2;
	m_CutRect.top = (m_ImageRect.Height() - m_pHeight)/2;
	m_CutRect.right = m_CutRect.left + m_pWidth;
	m_CutRect.bottom = m_CutRect.top + m_pHeight;

	SetDlgItemInt(IDC_EDIT_WIDTH,m_pWidth,1);
	SetDlgItemInt(IDC_EDIT_HEIGHT,m_pHeight,1);


	//UpdateData(false);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMainDlg::OnPaint()
{
	
	CPaintDC dc(this); // 用于绘制的设备上下文


	if (m_VMRCap.m_nWidth == 0 && m_VMRCap.m_nHeight == 0)
	{
		CDC *pDC = m_ctl_vedio.GetDC();

		CFont wFont;
		CFont *pOldFont;

		wFont.CreateFont(12, 0, 0, 0, FW_BOLD,
			0, 0, 0,
			0, 0, 0, 0,
			VARIABLE_PITCH | FF_SWISS, _T("MS Sans Serif"));
		pOldFont = pDC->SelectObject(&wFont);
		pDC->SetTextColor(RGB(255,0,0));
		pDC->SetBkMode(TRANSPARENT);
		pDC->TextOut(m_VedioRect.Width()/2-55,m_VedioRect.Height()/2-10, _T("没有连接到摄像头"));
		pDC->SelectObject(pOldFont);

		ReleaseDC(pDC);
	}

	if (m_VMRCap.m_pWC)
	{
		CPaintDC dc(FromHandle(m_ctl_vedio.GetSafeHwnd()));
		m_VMRCap.m_pWC->RepaintVideo(m_ctl_vedio.GetSafeHwnd(),dc);
	}	

	DrawPicture();
	DrawPhoto();

	// 不为绘图消息调用 CDialog::OnPaint()
	//CDialog::OnPaint();
}

void CMainDlg::DrawPicture()
{
	int tX,tY;
	if (m_SBitmapEx.IsValid())
	{
		m_BitmapEx.Create(m_SBitmapEx.GetWidth(),m_SBitmapEx.GetHeight());
		m_BitmapEx.Draw(0,0,m_SBitmapEx.GetWidth(),m_SBitmapEx.GetHeight(),m_SBitmapEx,0,0);
		m_BitmapEx.Scale(m_nScale,m_nScale);

		CDC *pDC = m_ctl_image.GetDC();

		CDC memDC,memDC1;
		memDC.CreateCompatibleDC(pDC);
		memDC1.CreateCompatibleDC(pDC);

		CBitmap bmp;
		bmp.CreateCompatibleBitmap(pDC, m_BitmapEx.GetWidth(), m_BitmapEx.GetHeight());
		memDC.SelectObject(bmp);
		m_BitmapEx.Draw(memDC);

		CBitmap bmp1;
		bmp1.CreateCompatibleBitmap(pDC, m_ImageRect.Width(), m_ImageRect.Height());
		memDC1.SelectObject(bmp1);

		CPen pen;
		CPen* pOldPen;
		CFont wFont,wFont1;
		CFont *pOldFont;
		CString title;

		//清除背景
		CRgn rgn;
		rgn.CreateRectRgn(0,0,m_ImageRect.Width(),m_ImageRect.Height());
		CBrush bBlack(GetSysColor(COLOR_3DFACE));
		memDC1.FillRgn(&rgn, &bBlack);

		//memDC1.SetStretchBltMode(HALFTONE);
		tX = (m_ImageRect.Width() - m_BitmapEx.GetWidth())/2;
		tY = (m_ImageRect.Height() - m_BitmapEx.GetHeight())/2;

		memDC1.BitBlt(tX,tY,m_BitmapEx.GetWidth(),m_BitmapEx.GetHeight(),&memDC,m_pLeft,m_pTop,SRCCOPY);

		pen.CreatePen(PS_SOLID, 1, RGB (255, 0, 0));

		pOldPen = memDC1.SelectObject (&pen);
		memDC1.MoveTo(m_CutRect.left,m_CutRect.top);
		memDC1.LineTo(m_CutRect.right,m_CutRect.top);
		memDC1.LineTo(m_CutRect.right,m_CutRect.bottom);
		memDC1.LineTo(m_CutRect.left,m_CutRect.bottom);
		memDC1.LineTo(m_CutRect.left,m_CutRect.top);
		memDC1.SelectObject (pOldPen);

		/*
		if (m_PicEditStatus)
		{
			//清除背景
			
			if (m_DrawRect.left > 0)
			{
				CRgn rGap1,rGap2,rGap3,rGap4;
				rGap1.CreateRectRgn(0, 0, m_DrawRect.left+10,m_ImageRect.Height());
				rGap2.CreateRectRgn(m_DrawRect.right-10, 0, m_ImageRect.right,m_ImageRect.Height());
				rGap3.CreateRectRgn(0, 0, m_ImageRect.Width(),m_DrawRect.top+10);
				rGap4.CreateRectRgn(0, m_DrawRect.bottom-10, m_ImageRect.Width(),m_ImageRect.bottom);

				rGap1.CombineRgn(&rGap1,&rGap2,RGN_OR);
				rGap1.CombineRgn(&rGap1,&rGap3,RGN_OR);
				rGap1.CombineRgn(&rGap1,&rGap4,RGN_OR);

				CBrush bBlack(GetSysColor(COLOR_3DFACE));
				memDC1.FillRgn(&rGap1, &bBlack);
			}

			memDC1.SetStretchBltMode(HALFTONE);

			if (m_Factor>=0)
			{
				memDC1.StretchBlt(0,0,m_ImageRect.Width(), m_ImageRect.Height(),
					&memDC, 
					(m_BitmapEx.GetWidth())*m_Factor/2,(m_BitmapEx.GetHeight())*m_Factor/2
					,(m_BitmapEx.GetWidth())*(1-m_Factor), (m_BitmapEx.GetHeight())*(1-m_Factor)
					,SRCCOPY);
				m_DrawRect = m_ImageRect;
			}
			else
			{
				memDC1.StretchBlt((m_ImageRect.Width())*(-m_Factor)/2,(m_ImageRect.Height())*(-m_Factor)/2
					,(m_ImageRect.Width())*(1+m_Factor), (m_ImageRect.Height())*(1+m_Factor)
					,&memDC, 
					0,0,m_BitmapEx.GetWidth(),m_BitmapEx.GetHeight()
					,SRCCOPY);
				m_DrawRect.left = (m_ImageRect.Width())*(-m_Factor)/2;
				m_DrawRect.top = (m_ImageRect.Height())*(-m_Factor)/2;
				m_DrawRect.right = 
					(m_ImageRect.Width())*(-m_Factor)/2 + (m_ImageRect.Width())*(1+m_Factor) - 1;
				m_DrawRect.bottom = 
					(m_ImageRect.Height())*(-m_Factor)/2 + (m_ImageRect.Height())*(1+m_Factor) - 1; 
			}


			pen.CreatePen(PS_SOLID, 1, RGB (255, 0, 0));
			pOldPen = memDC1.SelectObject (&pen);
			memDC1.MoveTo(m_CutRect.left,m_CutRect.top);
			memDC1.LineTo(m_CutRect.right,m_CutRect.top);
			memDC1.LineTo(m_CutRect.right,m_CutRect.bottom);
			memDC1.LineTo(m_CutRect.left,m_CutRect.bottom);
			memDC1.LineTo(m_CutRect.left,m_CutRect.top);
			memDC1.SelectObject (pOldPen);

			
			title.Format(_T("%d*%d"),
				m_Factor>=0 ? (int)(((double)m_BitmapEx.GetWidth()*(1-m_Factor)) * ((double)m_CutRect.Width()/m_ImageRect.Width())) 
					: (int)(((double)m_BitmapEx.GetWidth()/(1+m_Factor)) * ((double)m_CutRect.Width()/m_ImageRect.Width())),
				m_Factor>=0 ? (int)(((double)m_BitmapEx.GetHeight()*(1-m_Factor)) * ((double)m_CutRect.Height()/m_ImageRect.Height()))  
					: (int)(((double)m_BitmapEx.GetHeight()/(1+m_Factor)) * ((double)m_CutRect.Height()/m_ImageRect.Height())));
			wFont1.CreateFont(12, 0, 0, 0, 0,
				0, 0, 0,
				0, 0, 0, 0,
				VARIABLE_PITCH | FF_SWISS, _T("MS Sans Serif"));
			pOldFont = memDC1.SelectObject(&wFont1);
			memDC1.SetTextColor(RGB(255,0,0));
			memDC1.SetBkMode(TRANSPARENT);
			memDC1.TextOut(m_CutRect.left+2,m_CutRect.top+2, title);
			memDC1.SelectObject(pOldFont);

			
			title.Format(_T("状态:编辑  分辨率:%d*%d"),
				m_Factor>=0 ? (int)((double)m_BitmapEx.GetWidth()*(1-m_Factor)) : (int)((double)m_BitmapEx.GetWidth()/(1+m_Factor)),
				m_Factor>=0 ? (int)((double)m_BitmapEx.GetHeight()*(1-m_Factor))  : (int)((double)m_BitmapEx.GetHeight()/(1+m_Factor)));
			
		}
		else
		{
			memDC1.SetStretchBltMode(HALFTONE);

			memDC1.StretchBlt(0,0,m_ImageRect.Width(), m_ImageRect.Height(),
					&memDC, 
					0,0,m_BitmapEx.GetWidth(),m_BitmapEx.GetHeight(),
					SRCCOPY);

			title.Format(_T("状态:显示  分辨率:%d*%d"),m_BitmapEx.GetWidth(),m_BitmapEx.GetHeight());
		}
		*/

		title.Format(_T("缩放:%d:100"),m_nScale);
		
		wFont.CreateFont(12, 0, 0, 0, 0,
			0, 0, 0,
			0, 0, 0, 0,
			VARIABLE_PITCH | FF_SWISS, _T("MS Sans Serif"));
		pOldFont = memDC1.SelectObject(&wFont);
		memDC1.SetTextColor(RGB(255,0,0));
		memDC1.SetBkMode(TRANSPARENT);
		memDC1.TextOut(2,m_ImageRect.bottom-14, title);
		memDC1.SelectObject(pOldFont);

		pDC->BitBlt(0,0,m_ImageRect.Width(), m_ImageRect.Height(),&memDC1,0,0,SRCCOPY);

		memDC.DeleteDC();
		memDC1.DeleteDC();

		ReleaseDC(pDC);

		tX = (m_BitmapEx.GetWidth() - m_CutRect.Width())/2;
		tY = (m_BitmapEx.GetHeight() - m_CutRect.Height())/2;

		m_PhotoBitmapEx.Create(m_CutRect.Width(),m_CutRect.Height());
		m_PhotoBitmapEx.Draw(0,0,m_CutRect.Width(),m_CutRect.Height(),m_BitmapEx,tX + m_pLeft,tY + m_pTop);

		DrawPhoto();

	}else
	{
		CDC *pDC = m_ctl_image.GetDC();
		CBrush bBlack(GetSysColor(COLOR_3DFACE));
		pDC->FillRect(m_ImageRect,&bBlack);

		//m_PhotoBitmapEx.Create(0,0);
		
		//DrawPhoto();
	}
}

void CMainDlg::DrawPhoto()
{
	if (m_BitmapEx.IsValid())
	{

		//CBitmapEx m_PhotoBitmapEx;

		//int tX,tY;
		
		CDC *pDC = m_ctl_photo.GetDC();

		CDC memDC;
		memDC.CreateCompatibleDC(pDC);

		CBitmap bmp;
		bmp.CreateCompatibleBitmap(pDC, m_PhotoBitmapEx.GetWidth(), m_PhotoBitmapEx.GetHeight());
		memDC.SelectObject(bmp);

		CRect rect(0,0,m_PhotoBitmapEx.GetWidth(),m_PhotoBitmapEx.GetHeight());
		CBrush bBlack(GetSysColor(COLOR_3DFACE));

		memDC.FillRect(&rect, &bBlack);
		m_PhotoBitmapEx.Draw(memDC);

		pDC->FillRect(&m_PhotoRect, &bBlack);

		pDC->SetStretchBltMode(HALFTONE);
		pDC->StretchBlt(0,0,m_PhotoRect.Width(), m_PhotoRect.Height(),
			&memDC, 
			0,0,
			m_CutRect.Width(), m_CutRect.Height(),
			SRCCOPY);

		/*CDC *pDC = m_ctl_photo.GetDC();

		CDC memDC;
		memDC.CreateCompatibleDC(pDC);

		CBitmap bmp;
		bmp.CreateCompatibleBitmap(pDC, m_PhotoBitmapEx.GetWidth(), m_PhotoBitmapEx.GetHeight());
		memDC.SelectObject(bmp);

		CRect rect(0,0,m_PhotoBitmapEx.GetWidth(),m_PhotoBitmapEx.GetHeight());
		CBrush bBlack(GetSysColor(COLOR_3DFACE));

		memDC.FillRect(&rect, &bBlack);

		m_PhotoBitmapEx.Draw(memDC);

		pDC->FillRect(&m_PhotoRect, &bBlack);

		pDC->SetStretchBltMode(HALFTONE);
		pDC->StretchBlt(0,0,m_PhotoRect.Width(), m_PhotoRect.Height(),
			&memDC, 
			0,0,
			m_PhotoBitmapEx.GetWidth(), m_PhotoBitmapEx.GetHeight(),
			SRCCOPY);*/

		memDC.DeleteDC();
		ReleaseDC(pDC);
	}
	else
	{
		CDC *pDC = m_ctl_photo.GetDC();
		CBrush bBlack(GetSysColor(COLOR_3DFACE));
		pDC->FillRect(m_PhotoRect,&bBlack);
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMainDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMainDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnSysCommand(nID, lParam);
}

void CMainDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (this->m_VMRCap.GrabRudeFrame(&(this->m_SBitmapEx)))
	{
		//m_PicEditStatus = true;
		m_pLeft = 0;
		m_pTop = 0;
		m_nScale = m_DefScale;
	}

	DrawPicture();	
}

BOOL CMainDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	/*
	if (m_BitmapEx.IsValid())
	{
		if (m_PicEditStatus)
		{
			if (zDelta>0)
			{
				this->m_Factor = this->m_Factor + m_Factor_Radio;
				if (this->m_Factor < 1)
				{
					DrawPicture();
				}
				else
				{
					this->m_Factor = 1 - m_Factor_Radio;
				}
			}
			else
			{
				this->m_Factor = this->m_Factor - m_Factor_Radio;
				if (this->m_Factor > -1)
				{
					DrawPicture();
				}
				else
				{
					this->m_Factor = -1 + m_Factor_Radio;
				}
			}
		}
	}
	*/

	if (m_BitmapEx.IsValid())
	{
		if (zDelta>0){
			m_nScale = m_nScale + 2;
		}else
		{
			m_nScale = m_nScale - 2;
			if (m_nScale < 1) m_nScale = 1;
		}
		DrawPicture();
	}

	return true;

	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}

void CMainDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_BitmapEx.IsValid() && m_bFocus)
	{
		CRect myRect;
		m_ctl_image.GetWindowRect(myRect);
		ScreenToClient(myRect);

		if (PtInRect(&myRect,point))
		{
			SetCursor(LoadCursor(NULL,IDC_CROSS));
			m_pLeft = m_pLeft + (m_pDownPoint.x - point.x);
			m_pTop = m_pTop + (m_pDownPoint.y - point.y);
			m_pDownPoint.x = point.x;
			m_pDownPoint.y = point.y;
			DrawPicture();
		}	
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CMainDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_bFocus = false;
	if (m_BitmapEx.IsValid())
	{
		CRect myRect;
		m_ctl_image.GetWindowRect(myRect);
		ScreenToClient(myRect);

		if (PtInRect(&myRect,point))
		{
			m_bFocus = true;
			m_pDownPoint = point;
		}
	}
	/*
	if (m_BitmapEx.IsValid())
	{
		CRect myRect;
		m_ctl_image.GetWindowRect(myRect);
		ScreenToClient(myRect);

		if (PtInRect(&myRect,point))
		{
			if (m_PicEditStatus)
			{
				double mX,mY,mW,mH;

				if (m_Factor >= 0)
				{
					int mXpixs,mYpixs;

					mXpixs =(m_BitmapEx.GetWidth()/((int)((double)(1/m_Factor_Radio)))) * (m_Factor/m_Factor_Radio);
					mYpixs = (m_BitmapEx.GetHeight()/((int)((double)(1/m_Factor_Radio)))) * (m_Factor/m_Factor_Radio);
					mW = ((double)m_CutRect.Width() / m_ImageRect.Width()) * (m_BitmapEx.GetWidth()- mXpixs);
					mH = ((double)m_CutRect.Height() / m_ImageRect.Height()) * (m_BitmapEx.GetHeight()- mYpixs);
					mX = ((double)m_CutRect.left / m_ImageRect.Width()) * (m_BitmapEx.GetWidth()- mXpixs) + mXpixs / 2;
					mY = ((double)m_CutRect.top / m_ImageRect.Height()) * (m_BitmapEx.GetHeight()- mYpixs) + mYpixs / 2;
				}
				else
				{
					CRect myRect;
					myRect.IntersectRect(&m_DrawRect, &m_CutRect);

					mW = ((double)myRect.Width() / m_DrawRect.Width()) * m_BitmapEx.GetWidth();
					mH = ((double)myRect.Height() / m_DrawRect.Height()) * m_BitmapEx.GetHeight();
					mX = ((double)(myRect.left>m_DrawRect.left ? myRect.left- m_DrawRect.left : 0) / m_DrawRect.Width()) * m_BitmapEx.GetWidth();
					mY = ((double)(myRect.top>m_DrawRect.top ? myRect.top-m_DrawRect.top : 0) / m_DrawRect.Height()) * m_BitmapEx.GetHeight();
				}

				m_PhotoBitmapEx.Create(mW,mH);
				m_PhotoBitmapEx.Draw(0,0,mW,mH,m_BitmapEx,mX,mY);

				DrawPhoto();

				m_xs.Format(_T("%d*%d"),m_PhotoBitmapEx.GetWidth(),m_PhotoBitmapEx.GetHeight());
				CTime time = CTime::GetCurrentTime();
				m_sj = time.Format(_T("%Y%m%d %H%M%S"));
				this->UpdateData(FALSE);
			}
		}	
	}
	*/
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CMainDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bFocus = false;
}

void CMainDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	/*if (m_BitmapEx.IsValid())
	{
		CRect myRect;
		m_ctl_image.GetWindowRect(myRect);
		ScreenToClient(myRect);

		if (PtInRect(&myRect,point))
		{
			m_PicEditStatus = !m_PicEditStatus;

			if (m_PicEditStatus)
			{
				ClientToScreen(myRect);
				SetCursorPos(myRect.left+m_ImageRect.Width()/2,
								myRect.top+m_ImageRect.Height()/2);
				m_CutMousePoint.SetPoint(
					myRect.left+m_ImageRect.Width()/2,myRect.top+m_ImageRect.Height()/2);
				ScreenToClient(myRect);

				m_Factor = 0;
				m_DrawRect = m_ImageRect;

				m_CutRect.SetRect((m_ImageRect.Width() - PICCUT_WIDTH)/2,
							(m_ImageRect.Height() - PICCUT_HEIGHT)/2,
							(m_ImageRect.Width() - PICCUT_WIDTH)/2 + PICCUT_WIDTH ,
							(m_ImageRect.Height() - PICCUT_HEIGHT)/2 + PICCUT_HEIGHT);

			}

			DrawPicture();
		}	
	}*/


	CDialog::OnLButtonDblClk(nFlags, point);
}

void CMainDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	CString m_sFilename;

	CFileDialog dialog(TRUE, NULL, m_sFilename, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		_T("Images (*.bmp, *.gif, *.jpeg, *.jpg)|*.bmp;*.gif;*.jpeg;*.jpg||"));

	if (IDOK == dialog.DoModal())
	{
		m_sFilename = dialog.GetPathName();
		m_SBitmapEx.LoadImageFile((LPTSTR)m_sFilename.GetBuffer());

		m_pLeft = 0;
		m_pTop = 0;
		m_nScale = 100;
	}

	//m_PicEditStatus = true;
	DrawPicture();
}

void CMainDlg::ReSet()
{
	this->m_SBitmapEx.Create(0,0);

	this->m_BitmapEx.Create(0,0);
	//this->DrawPicture();

	//this->m_PhotoBitmapEx.Create(0,0);
	//this->DrawPhoto();
	
	m_id = "";
	m_pLeft = 0;
	m_pTop = 0;
	m_nScale = 100;

	this->DrawPicture();
	//this->DrawPhoto();
	this->UpdateData(FALSE);
}


void CMainDlg::OnEnChangeEdit2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

void CMainDlg::GetPhoto(LPTSTR pBuffer)
{
}

void CMainDlg::OnStnClickedImage()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CMainDlg::OnBnClickedSet()
{
	//CString m_sFilename;

	CDlgSet dialog;

	if (IDOK == dialog.DoModal())
	{
		if (GetParm()){
			m_CutRect.left = (m_ImageRect.Width() - m_pWidth)/2;
			m_CutRect.top = (m_ImageRect.Height() - m_pHeight)/2;
			m_CutRect.right = m_CutRect.left + m_pWidth;
			m_CutRect.bottom = m_CutRect.top + m_pHeight;
			SetDlgItemInt(IDC_EDIT_WIDTH,m_pWidth,1);
			SetDlgItemInt(IDC_EDIT_HEIGHT,m_pHeight,1);
		}
	}

	DrawPicture();
}

int CMainDlg::GetParm(void)
{
	HKEY hKey;
	DWORD dwBufLen=sizeof(int);
	LONG lRet;
	DWORD dwType=0;

	lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,TEXT("SOFTWARE\\ICFK\\CameraPhoto\\Parm"), 0, KEY_READ, &hKey );
	if( lRet != ERROR_SUCCESS )
		return FALSE;

	lRet = RegQueryValueEx( hKey, TEXT("Scale"),NULL, &dwType,(LPBYTE) &m_DefScale, &dwBufLen);
	if( lRet != ERROR_SUCCESS )
		return FALSE;

	lRet = RegQueryValueEx( hKey, TEXT("PictureWidth"),NULL, &dwType, (LPBYTE)&m_pWidth, &dwBufLen);
	if( lRet != ERROR_SUCCESS )
		return FALSE;

	lRet = RegQueryValueEx( hKey, TEXT("PictureHeight"),NULL, &dwType, (LPBYTE)&m_pHeight, &dwBufLen);
	if( lRet != ERROR_SUCCESS )
		return FALSE;

	RegCloseKey( hKey );

	return TRUE;
}
