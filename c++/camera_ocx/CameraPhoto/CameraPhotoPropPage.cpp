// CameraPhotoPropPage.cpp : CCameraPhotoPropPage 属性页类的实现。

#include "stdafx.h"
#include "CameraPhoto.h"
#include "CameraPhotoPropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CCameraPhotoPropPage, COlePropertyPage)



// 消息映射

BEGIN_MESSAGE_MAP(CCameraPhotoPropPage, COlePropertyPage)
END_MESSAGE_MAP()



// 初始化类工厂和 guid

IMPLEMENT_OLECREATE_EX(CCameraPhotoPropPage, "CAMERAPHOTO.CameraPhotoPropPage.1",
	0x5fcc58c6, 0xd78b, 0x4966, 0x9c, 0xaf, 0x2d, 0x6c, 0x35, 0x11, 0xab, 0x1)



// CCameraPhotoPropPage::CCameraPhotoPropPageFactory::UpdateRegistry -
// 添加或移除 CCameraPhotoPropPage 的系统注册表项

BOOL CCameraPhotoPropPage::CCameraPhotoPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_CAMERAPHOTO_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}



// CCameraPhotoPropPage::CCameraPhotoPropPage - 构造函数

CCameraPhotoPropPage::CCameraPhotoPropPage() :
	COlePropertyPage(IDD, IDS_CAMERAPHOTO_PPG_CAPTION)
{
}



// CCameraPhotoPropPage::DoDataExchange - 在页和属性间移动数据

void CCameraPhotoPropPage::DoDataExchange(CDataExchange* pDX)
{
	DDP_PostProcessing(pDX);
}



// CCameraPhotoPropPage 消息处理程序
