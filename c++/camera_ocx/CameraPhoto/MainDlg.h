#pragma once

#include "afxwin.h"
#include "VMR_Capture.h"
#include "BitmapEx.h"

//#define PICCUT_WIDTH 146
//#define PICCUT_HEIGHT 146
//#define PICZOOM_RADIO 0.00

// CMainDlg 对话框

class CMainDlg : public CDialog
{
	DECLARE_DYNAMIC(CMainDlg)

public:
	CMainDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMainDlg();

// 对话框数据
	enum { IDD = IDD_LX_CPMAINDLG_DIALOG };
public:
	CVMR_Capture	m_VMRCap;
	CBitmapEx		m_BitmapEx;
	CBitmapEx		m_SBitmapEx;
	CBitmapEx		m_PhotoBitmapEx;

	CRect			m_VMRRect;
	CRect			m_VedioRect;
	CRect			m_ImageRect;
	CRect			m_DrawRect;
	CRect			m_PhotoRect;

	CRect			m_CutRect;
	CPoint			m_CutMousePoint;

	CPoint			m_pDownPoint;
	bool			m_bFocus;

	double			m_Factor;
	//double			m_Factor_Radio;

// 实现
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	HICON m_hIcon;

	void DrawPicture();
	void DrawPhoto();

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_ctl_vedio;
	CStatic m_ctl_image;
	CStatic m_ctl_photo;
	CStatic m_sxtgp;
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnBnClickedButton1();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButton2();
	CString m_id;
	void ReSet();
	afx_msg void OnEnChangeEdit2();
	int m_pWidth;
	int m_pHeight;
	int m_nScale;
	int m_pLeft;
	int m_pTop;
	void GetPhoto(LPTSTR pBuffer);
	afx_msg void OnStnClickedImage();
	int m_DefScale;
	afx_msg void OnBnClickedSet();
	int GetParm(void);
};
