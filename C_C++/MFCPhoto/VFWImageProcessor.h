// VFWImageProcessor.h: interface for the CVFWImageProcessor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VFWIMAGEPROCESSOR_H__635099B7_63C1_11D4_9EF0_000083A26E84__INCLUDED_)
#define AFX_VFWIMAGEPROCESSOR_H__635099B7_63C1_11D4_9EF0_000083A26E84__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <afxmt.h>
#include <vfw.h>
#include <afxole.h>

#define MAX_VFW_DEVICES    10

//class AFX_EXT_CLASS CVFWImageProcessor
class CVFWImageProcessor
{
public:
   CVFWImageProcessor();
   virtual ~CVFWImageProcessor();
   CVFWImageProcessor(const CVFWImageProcessor &CopyFrom);
   CVFWImageProcessor &operator =(const CVFWImageProcessor &CopyFrom);

   BOOL Initialize(SHORT DriverIndex = 0);
   VOID Destroy();

   BOOL SetDriver(SHORT DriverIndex);

   HWND GetCapWindow();

   BOOL CaptureDIB(PBITMAPINFO *Bitmap, ULONG BitmapLength, ULONG *RetBitmapLength);
   BOOL CaptureAVI(LPCTSTR Filename, FLOAT FramesPerSec, ULONG Duration, UINT Quality);                            
   BOOL EnablePreviewVideo(HWND Parent, INT x, INT y, INT PreviewRate = 30);
   BOOL EnablePreviewVideo(HWND Parent, INT x, INT y, INT Width, INT Height, INT PreviewRate = 30);
   BOOL DisablePreviewVideo();
   BOOL DriverGetCaps(CAPDRIVERCAPS *Caps);
   BOOL DlgVideoSource();
   BOOL DlgVideoFormat();
   BOOL DlgVideoDisplay();
   VOID CancelCapture();
   BOOL AllocDIBImage(PBITMAPINFO *ppImageData, ULONG *AllocatedSize);

   BITMAPINFOHEADER GetBitmapInfoHeader();

   VOID GetPreviousError(INT *ErrorID, CString *ErrorString, BOOL ResetError = FALSE);

   static ULONG CalcBitmapSize(const BITMAPINFOHEADER &bmiHeader);
   static ULONG CalcBitmapInfoSize(const BITMAPINFOHEADER &bmiHeader);

   friend LRESULT CALLBACK ErrorCallbackProc(HWND hWnd, int nErrID, LPSTR lpErrorText);
   friend LRESULT CALLBACK ControlCallbackProc(HWND hWnd, int nState);
   friend LRESULT CALLBACK StatusCallbackProc(HWND hWnd, int nID, LPCSTR lpsz);
   friend LRESULT CALLBACK FrameCallbackProc(HWND hWnd, LPVIDEOHDR lpVHdr);
   friend LRESULT CALLBACK StreamCallbackProc(HWND hWnd, LPVIDEOHDR lpVHdr);
 
   friend UINT CaptureThreadFunc(VOID *pCVFWImageProcessor);

private:    // Data
   HWND m_hWndVideo;
   CMutex m_ImageProtect;
   CEvent m_ImageReady;
   CEvent m_CancelCapture;
   CEvent m_CaptureEnded;

   BITMAPINFOHEADER m_BitmapInfoHeader;      // Used to store image dimensions.

   PBITMAPINFO m_TransferBitmapInfo;
   ULONG m_TransferBitmapInfoSize;

   CWinThread *m_CaptureThread;
   INT m_DriverIndex;

   INT m_ErrorID;
   CString m_ErrorText;

   static UINT m_ValidDriverIndex[MAX_VFW_DEVICES];
   static USHORT m_TotalVideoDrivers;

private:    // Functions
   CVFWImageProcessor &Copy(const CVFWImageProcessor &CopyFrom);
};

#endif // !defined(AFX_VFWIMAGEPROCESSOR_H__635099B7_63C1_11D4_9EF0_000083A26E84__INCLUDED_)
