#pragma once

// CameraPhotoPropPage.h : CCameraPhotoPropPage 属性页类的声明。


// CCameraPhotoPropPage : 有关实现的信息，请参阅 CameraPhotoPropPage.cpp。

class CCameraPhotoPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CCameraPhotoPropPage)
	DECLARE_OLECREATE_EX(CCameraPhotoPropPage)

// 构造函数
public:
	CCameraPhotoPropPage();

// 对话框数据
	enum { IDD = IDD_PROPPAGE_CAMERAPHOTO };

// 实现
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 消息映射
protected:
	DECLARE_MESSAGE_MAP()
};

