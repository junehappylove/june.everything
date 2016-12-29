//////////////////////////////////////////////////////////////////////
//
//  This class is designed to provide simple interface for 
//  simultaneous Video Capture & Preview using DirectShow
//
//////////////////////////////////////////////////////////////////////
//
//	References: MS DirectShow Samples
//
//		
//////////////////////////////////////////////////////////////////////
//
//	This class was written by Sagar K.R . 
//  Use of this class is not restricted in any
//	way whatsoever.Please report the bugs to krssagar@firsteccom.co.kr
//
//	Special thanks to all the members at The Code Project! 
//	(www.codeproject.com)
//
//////////////////////////////////////////////////////////////////////

// VMR_Capture.h: interface for the CVMR_Capture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VMR_CAPTURE_H__186091F3_30FA_4FAA_AC8B_EF25E8463B9A__INCLUDED_)
#define AFX_VMR_CAPTURE_H__186091F3_30FA_4FAA_AC8B_EF25E8463B9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <atlbase.h>
#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>

#include "BitmapEx.h"

//#define WM_GRAPHNOTIFY  WM_USER+13
enum PLAYSTATE {Stopped, Paused, Running, Init};


class CVMR_Capture  
{
public:

	CVMR_Capture();

	int EnumDevices();
	int EnumDevices(HWND hList);
	HRESULT Init(int iDeviceID,HWND hWnd,int iWidth,int iHeight);
	HRESULT InitMaxDPI(int iDeviceID,HWND hWnd);
	DWORD GetFrame(BYTE ** pFrame);
	BOOL StopVMR();	
	BOOL Pause();
	DWORD ImageCapture(LPCTSTR szFile);
	DWORD GrabFrame();
	bool GrabRudeFrame(CBitmapEx *pCBitmapEx);
	
	int m_nWidth;
	int m_nHeight;
	
	virtual ~CVMR_Capture();
	

//protected:
public:
	
	IGraphBuilder *m_pGB ;
	IMediaControl *m_pMC;
	IMediaEventEx *m_pME ;
	//IMediaEvent *pME ;
	
	
	IVMRWindowlessControl9 *m_pWC;
	IPin * m_pCamOutPin;
	IBaseFilter *m_pDF;

	PLAYSTATE m_psCurrent;

	BYTE *m_pFrame;
	long m_nFramelen;

	
	bool BindFilter(int deviceId, IBaseFilter **pFilter);
	void StopCapture();
	void CloseInterfaces(void);
	void DeleteMediaType(AM_MEDIA_TYPE *pmt);
	bool Convert24Image(BYTE *p32Img,BYTE *p24Img,DWORD dwSize32);

	HRESULT InitVideoWindow(HWND hWnd,int width_, int height_);
	HRESULT InitMaxDPIVideoWindow(HWND hWnd);
	HRESULT InitializeWindowlessVMR(HWND hWnd);


private:
	
};

#endif // !defined(AFX_VMR_CAPTURE_H__186091F3_30FA_4FAA_AC8B_EF25E8463B9A__INCLUDED_)
