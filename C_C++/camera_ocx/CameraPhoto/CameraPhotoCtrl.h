#pragma once

// CameraPhotoCtrl.h : CCameraPhotoCtrl ActiveX 控件类的声明。
#include "MainDlg.h"

// CCameraPhotoCtrl : 有关实现的信息，请参阅 CameraPhotoCtrl.cpp。

class CCameraPhotoCtrl : public COleControl
{
	DECLARE_DYNCREATE(CCameraPhotoCtrl)

// 构造函数
public:
	CCameraPhotoCtrl();

// 重写
public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual DWORD GetControlFlags();

	CMainDlg m_MainDialog;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();

// 实现
protected:
	~CCameraPhotoCtrl();

	DECLARE_OLECREATE_EX(CCameraPhotoCtrl)    // 类工厂和 guid
	DECLARE_OLETYPELIB(CCameraPhotoCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CCameraPhotoCtrl)     // 属性页 ID
	DECLARE_OLECTLTYPE(CCameraPhotoCtrl)		// 类型名称和杂项状态

	BOOL IsSubclassedControl();
	LRESULT OnOcmCommand(WPARAM wParam, LPARAM lParam);

// 消息映射
	DECLARE_MESSAGE_MAP()

// 调度映射
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// 事件映射
	DECLARE_EVENT_MAP()

// 调度和事件 ID
public:
	enum {
		dispidGetVersion = 4L,
		dispidGetPhoto = 3L,
		dispidReset = 2L,
		dispidSetID = 1L
	};
protected:
	void SetID(LPCTSTR id);
	void Reset(void);
	BSTR GetPhoto(void);
	BSTR GetVersion(void);
};

