#pragma once


// CDlgSet 对话框

class CDlgSet : public CDialog
{
	DECLARE_DYNAMIC(CDlgSet)

public:
	CDlgSet(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgSet();

// 对话框数据
	enum { IDD = IDD_DLG_SET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	int m_nScale;
	int m_nWidth;
	int m_nHeight;
	bool GetParm(void);
	bool SetParm(void);
	afx_msg void OnBnClickedOk();
};
