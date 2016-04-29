/*******************************************************************************
    Title............... Video For Windows Class Interface
    Programmer.......... Ken Varn
    Date Created........ 9/20/2000
    Operating System.... Windows NT 4
    Compiler............ Microsoft Visual C++ 6
    File Type........... C++ Source

    Description:
       Class interface to Video For Windows.

       Before using any functions in this class, the Initialize() member
       function must be called on the instantiated object.

       When finished using this class, the Destroy() member function should
       be used.

    Revision History:
       Revision Date.... xx-xx-xx
       Programmer....... xxx
       Comments......... xxx
*******************************************************************************/


#include "stdafx.h"

#include "VFWImageProcessor.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma comment(lib,"vfw32")

UINT CVFWImageProcessor::m_ValidDriverIndex[MAX_VFW_DEVICES];
USHORT CVFWImageProcessor::m_TotalVideoDrivers = 0;

#define UM_VID_SOURCE      (WM_USER+1)
#define UM_VID_FORMAT      (WM_USER+2)
#define UM_VID_DISPLAY     (WM_USER+3)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CVFWImageProcessor::CVFWImageProcessor()

{
   m_TransferBitmapInfo = NULL;
   m_TransferBitmapInfoSize = 0;
   m_hWndVideo = NULL;
   m_CaptureThread = NULL;

   m_DriverIndex = -1;

   memset(&m_BitmapInfoHeader,0,sizeof(m_BitmapInfoHeader));
}

CVFWImageProcessor::~CVFWImageProcessor()
{
   Destroy();
}

/*******************************************************************************
   Function   : Initialize
   Arguments  : DriverIndex (input) - Index of VFW driver.
   Return     : TRUE Success, FALSE Failure
   Description: Inititlizes the object for using VFW interface to capture
                device.
*******************************************************************************/
BOOL CVFWImageProcessor::Initialize(SHORT DriverIndex)
{
   BOOL Ret = FALSE;
   SHORT Index;

   Destroy();

   // Reset any error conditions.
   GetPreviousError(NULL,NULL,TRUE);

   // This semaphore will temporarily be used to determine when the
   // capture thread is ready.
   m_ImageReady.ResetEvent();

   // Create message pump for window messages.
   m_CaptureThread = AfxBeginThread(CaptureThreadFunc,(void *) this);
   m_CaptureThread->m_bAutoDelete = FALSE;

   // Wait for event to determine when capture thread is ready.
   ::WaitForSingleObject(m_ImageReady, INFINITE);

   if (m_hWndVideo)
   {
      capSetUserData(m_hWndVideo,this);
      capSetCallbackOnError(m_hWndVideo,ErrorCallbackProc);
      capSetCallbackOnCapControl(m_hWndVideo,ControlCallbackProc);
      capSetCallbackOnStatus(m_hWndVideo,StatusCallbackProc);
      capSetCallbackOnFrame(m_hWndVideo, FrameCallbackProc);
      capSetCallbackOnVideoStream(m_hWndVideo, StreamCallbackProc);

      // Construct list of valid video drivers.
      // This creates a contiguous virtual driver table.

      if (!m_TotalVideoDrivers)
      {
         char szDeviceName[80];
         char szDeviceVersion[80];

         for (Index = 0; Index < MAX_VFW_DEVICES; Index++)
         {
            if (capGetDriverDescription(Index,
                                        szDeviceName,
                                        sizeof(szDeviceName),
                                        szDeviceVersion,
                                        sizeof(szDeviceVersion)))
            {
               try
               {
                  if (capDriverConnect(m_hWndVideo, Index))
                  {
                     m_ValidDriverIndex[m_TotalVideoDrivers] = Index;
                     m_TotalVideoDrivers++;
                     capDriverDisconnect(m_hWndVideo);
                  }
               }
               catch(CException *Ex)
               {
                  Ex->Delete();
               }
               catch(...)
               {

               }
            }
         }
      }

      // Reset any error conditions.
      GetPreviousError(NULL,NULL,TRUE);

      Ret = SetDriver(DriverIndex);
   }

   if (!Ret)
   {
      if (m_ErrorID == 0)
         m_ErrorID = DV_ERR_NONSPECIFIC;

      Destroy();
   }

   return Ret;
}


/*******************************************************************************
   Function   : GetCapWindow
   Arguments  : none
   Return     : HWND of VFW window.
   Description: Used to retrieve the handle used for VFW image processing.
*******************************************************************************/
HWND CVFWImageProcessor::GetCapWindow()
{
   return m_hWndVideo;
}

//
// Copy Constructor
//
CVFWImageProcessor::CVFWImageProcessor(const CVFWImageProcessor &CopyFrom)
{
   m_TransferBitmapInfo = NULL;
   m_TransferBitmapInfoSize = 0;

   m_hWndVideo = NULL;
   m_CaptureThread = NULL;
   m_DriverIndex = -1;
   memset(&m_BitmapInfoHeader,0,sizeof(m_BitmapInfoHeader));

   Copy(CopyFrom);
}


//
// Copy class using operator=
//
CVFWImageProcessor &CVFWImageProcessor::operator =(const CVFWImageProcessor &CopyFrom)
{
   return Copy(CopyFrom);
}


/*******************************************************************************
   Function   : Destroy
   Arguments  : none
   Return     : none
   Description: Closes up the interface for VFW of capture device.
*******************************************************************************/
VOID CVFWImageProcessor::Destroy()
{
   // Reset any error conditions.
   GetPreviousError(NULL,NULL,TRUE);

   if (m_hWndVideo)
   {
      DisablePreviewVideo();

      capCaptureAbort(m_hWndVideo);
      capSetCallbackOnError(m_hWndVideo,NULL);
      capSetCallbackOnCapControl(m_hWndVideo,NULL);
      capSetCallbackOnStatus(m_hWndVideo,NULL);
      capSetCallbackOnFrame(m_hWndVideo,NULL);
      capSetCallbackOnVideoStream(m_hWndVideo, NULL);

      capSetUserData(m_hWndVideo,NULL);
      capDriverDisconnect(m_hWndVideo);
   }

   if (m_CaptureThread)
   {
      DWORD ExitCode;
      INT Timeout = 50;   // Tenths of a Second

      // Terminate the message queue thread and wait for it to end.
      m_CaptureThread->PostThreadMessage(WM_QUIT,0,0);

      while(Timeout)
      {
         GetExitCodeThread(m_CaptureThread->m_hThread, &ExitCode);

         if (ExitCode != STILL_ACTIVE)
         {
            // Thread has ended.
            break;
         }
         else
         {
            Sleep(100);
         }

         --Timeout;
      }

      delete m_CaptureThread;

      m_hWndVideo = NULL;
   }

   m_TransferBitmapInfo = NULL;
   m_TransferBitmapInfoSize = 0;
   m_hWndVideo = NULL;
   m_CaptureThread = NULL;
   m_DriverIndex = -1;

   memset(&m_BitmapInfoHeader,0,sizeof(m_BitmapInfoHeader));
}



/*******************************************************************************
   Function   : CaptureDIB
   Arguments  : Bitmap (output) - Pointer to bitmap to receive image.
                                  If *Bitmap = NULL, then allocation will
                                  be performed automatically.
                BitmapLength (input) - Size of Bitmap if *Bitmap is not NULL.
                RetBitmapLength (output) - Actual size of image.
   Return     : TRUE Success, FALSE Failed.
   Description: Captures a DIB image from video capture device.
*******************************************************************************/
BOOL CVFWImageProcessor::CaptureDIB(PBITMAPINFO *Bitmap,
                                    ULONG BitmapLength,
                                    ULONG *RetBitmapLength)
{
   BOOL Ret = FALSE;
   CSingleLock ImageLockObj(&m_ImageProtect);

   DWORD Size = 0;

   // Reset any error conditions.
   GetPreviousError(NULL,NULL,TRUE);

   if (*Bitmap == NULL)
   {
      AllocDIBImage(Bitmap,&Size);
      BitmapLength = Size;
   }
   else
   {
      AllocDIBImage(NULL,&Size);
   }

   if (*Bitmap && Size > 0)
   {
      if (RetBitmapLength)
      {
         *RetBitmapLength = Size;
      }

      // Must assign pointer to class member variable so that the
      // callback function can get to it.
      ImageLockObj.Lock();
      m_TransferBitmapInfo = *Bitmap;
      m_TransferBitmapInfoSize = BitmapLength;
      ImageLockObj.Unlock();

      // Reset event semaphore so we know when an image is ready.
      m_ImageReady.ResetEvent();

      // Start capturing now.  Callback function will capture and signal us when done.
      TRACE("Before Capture Start Call\n");
      Ret = capGrabFrame(m_hWndVideo);
      TRACE("After Capture Start Call\n");

      if (Ret)
      {
         Ret = FALSE;

         // Wait for capture to complete.
         if (::WaitForSingleObject(m_ImageReady,2000) == WAIT_OBJECT_0)
         {
            TRACE("Image Ready\n");
            Ret = TRUE;   // Success
         }
      }

      ImageLockObj.Lock();
      m_TransferBitmapInfo = NULL;
      m_TransferBitmapInfoSize = 0;
      ImageLockObj.Unlock();

      if (!Ret)
      {
         if (RetBitmapLength)
         {
            *RetBitmapLength = (ULONG) 0;
         }
      }
   }

   if (!Ret && m_ErrorID == 0)
   {
      m_ErrorID = DV_ERR_NONSPECIFIC;
   }

   return Ret;
}


/*******************************************************************************
   Function   : CaptureAVI
   Arguments  : Filename (input) - Name of file to capture AVI
                FramesPerSec (input) - Frames Per second of AVI.
                Duration (input) - How long to run it in seconds.
                Quality (input unused)
   Return     : TRUE Success, FALSE Failed.
   Description: Captures AVI to file from current video capture device.
*******************************************************************************/
BOOL CVFWImageProcessor::CaptureAVI(LPCTSTR Filename,
                                    FLOAT FramesPerSec,
                                    ULONG Duration,
                                    UINT Quality)
{
   BOOL Ret = FALSE;
   CAPTUREPARMS OrigCapParms;
   CAPTUREPARMS CapParms;
   // Reset any error conditions.
   GetPreviousError(NULL,NULL,TRUE);

   m_CancelCapture.ResetEvent();

   capCaptureGetSetup(m_hWndVideo,&OrigCapParms,sizeof(OrigCapParms));
   CapParms = OrigCapParms;
   CapParms.dwRequestMicroSecPerFrame = (DWORD) (1.0e6 / FramesPerSec);
   CapParms.fLimitEnabled = TRUE;
   CapParms.wTimeLimit = Duration;
   CapParms.fYield = FALSE;
   CapParms.wPercentDropForError = 100; //Quality;

   capCaptureSetSetup(m_hWndVideo, &CapParms, sizeof(CapParms));

   Ret = capCaptureSequence(m_hWndVideo);

   if (Ret)
   {
      Ret = capFileSaveAs(m_hWndVideo, Filename);
   }

   capCaptureSetSetup(m_hWndVideo, &OrigCapParms, sizeof (OrigCapParms));

   if (!Ret && m_ErrorID == 0)
   {
      m_ErrorID = DV_ERR_NONSPECIFIC;
   }

   return Ret;
}


//
// Private function used to copy objects.
//
CVFWImageProcessor &CVFWImageProcessor::Copy(const CVFWImageProcessor &CopyFrom)
{
   INT DeviceIdx;

   if (&CopyFrom != this)
   {
      Destroy();

      if (CopyFrom.m_hWndVideo)
      {
         CAPDRIVERCAPS DriverCaps;

         capDriverGetCaps(CopyFrom.m_hWndVideo,&DriverCaps,sizeof(DriverCaps));

         // Find the device id in the virtual device list.
         for (DeviceIdx=0;DeviceIdx<MAX_VFW_DEVICES;++DeviceIdx)
         {
            if (m_ValidDriverIndex[DeviceIdx] == DriverCaps.wDeviceIndex)
            {
               Initialize(DeviceIdx);
               break;
            }
         }
      }
   }

   return *this;
}


/*******************************************************************************
   Function   : SetDriver
   Arguments  : DriverIndex (input) - Driver to set
   Return     : TRUE Success, FALSE Failed.
   Description: Sets curretn capture driver.
*******************************************************************************/
BOOL CVFWImageProcessor::SetDriver(SHORT DriverIndex)
{
   BOOL Ret = TRUE;
   CAPTUREPARMS CapParms = {0};

   // Reset any error conditions.
   GetPreviousError(NULL,NULL,TRUE);

   if (DriverIndex >= m_TotalVideoDrivers)
   {
      Ret = FALSE;
      m_ErrorID = DV_ERR_BADDEVICEID;
   }

   if (m_hWndVideo && m_DriverIndex != DriverIndex && Ret)
   {
      if (GetParent(m_hWndVideo) != NULL)
         capPreview(m_hWndVideo,FALSE);

      DisablePreviewVideo();
      capCaptureAbort(m_hWndVideo);

      Ret = capDriverConnect(m_hWndVideo, m_ValidDriverIndex[DriverIndex]);

      if (Ret)
      {
         capGetVideoFormat(m_hWndVideo,(PBITMAPINFO) &m_BitmapInfoHeader,sizeof(m_BitmapInfoHeader));
         capCaptureGetSetup(m_hWndVideo,&CapParms,sizeof(CapParms));
         CapParms.fAbortLeftMouse = FALSE;
         CapParms.fAbortRightMouse = FALSE;
         CapParms.fYield = TRUE;
         CapParms.fCaptureAudio = FALSE;
         CapParms.wPercentDropForError = 100;
         capCaptureSetSetup(m_hWndVideo,&CapParms,sizeof(CapParms));
         m_DriverIndex = DriverIndex;

         if (GetParent(m_hWndVideo) != NULL)
            capPreview(m_hWndVideo,TRUE);
      }
   }

   if (!Ret && m_ErrorID == 0)
   {
      m_ErrorID = DV_ERR_NONSPECIFIC;
   }

   return Ret;
}


/*******************************************************************************
   Function   : GetPreviousError
   Arguments  : ErrorID (output) - ID of Error
                ErrorString (output) - Description of error.
                ResetError (input) - TRUE Reset error condition.
   Return     : none
   Description: Gets the last Error ID and Error Description.
*******************************************************************************/
VOID CVFWImageProcessor::GetPreviousError(INT *ErrorID, CString *ErrorString, BOOL ResetError)
{
   if (ErrorID)
      *ErrorID = m_ErrorID;

   if (ErrorString)
      *ErrorString = m_ErrorText;

   if (ResetError)
   {
      m_ErrorID = 0;
      m_ErrorText.Empty();
   }
}


/*******************************************************************************
   Function   : EnablePreviewVideo
   Arguments  : Parent (input) - Parent window that will display video.
                x (input) - X Location in parent where video will be shown.
                y (input) - Y location in parent where video will be shown.
                PreviewRate (input) - Rate of preview in FPS.
   Return     : TRUE Success, FALSE Failed.
   Description: Enables preview video mode.
*******************************************************************************/
BOOL CVFWImageProcessor::EnablePreviewVideo(HWND Parent, INT x, INT y, INT PreviewRate)
{
   // Reset any error conditions.
   return EnablePreviewVideo(Parent,
                             x,y,
                             m_BitmapInfoHeader.biWidth,
                             m_BitmapInfoHeader.biHeight,
                             PreviewRate);
}


/*******************************************************************************
   Function   : EnablePreviewVideo
   Arguments  : Parent (input) - Parent window that will display video.
                x (input) - X Location in parent where video will be shown.
                y (input) - Y location in parent where video will be shown.
                Width (input) - Width of preview window.
                Height (input) - Height of preview window.
                PreviewRate (input) - Rate of preview in FPS.
   Return     : TRUE Success, FALSE Failed.
   Description: Enables preview video mode.
*******************************************************************************/
BOOL CVFWImageProcessor::EnablePreviewVideo(HWND Parent, INT x, INT y, INT Width, INT Height, INT PreviewRate)
{
   // Reset any error conditions.
   GetPreviousError(NULL,NULL,TRUE);

   SetParent(m_hWndVideo,Parent);
   SetWindowLong(m_hWndVideo,GWL_STYLE,WS_CHILD);

   SetWindowPos(m_hWndVideo,NULL,x,y,
                Width,
                Height,
                SWP_NOZORDER);
   ShowWindow(m_hWndVideo,SW_SHOW);
   capPreviewRate(m_hWndVideo, PreviewRate);

   return capPreview(m_hWndVideo,TRUE);
}




/*******************************************************************************
   Function   : DisablePreviewVideo
   Arguments  : none
   Return     : TRUE Success, FALSE Failed.
   Description: Disables preview video.
*******************************************************************************/
BOOL CVFWImageProcessor::DisablePreviewVideo()
{
   // Reset any error conditions.
   GetPreviousError(NULL,NULL,TRUE);

   BOOL Ret = capPreview(m_hWndVideo,FALSE);

   SetWindowPos(m_hWndVideo,NULL,0,0,0,0,SWP_NOZORDER);
   SetParent(m_hWndVideo,NULL);
   SetWindowLong(m_hWndVideo,GWL_STYLE,WS_POPUP);

   return Ret;
}


/*******************************************************************************
   Function   : DriverGetCaps
   Arguments  : Caps (output)
   Return     : See capDriverGetCaps()
   Description: Wrapper function for capDriverGetCaps().
*******************************************************************************/
BOOL CVFWImageProcessor::DriverGetCaps(CAPDRIVERCAPS *Caps)
{
   // Reset any error conditions.
  GetPreviousError(NULL,NULL,TRUE);

  return capDriverGetCaps(m_hWndVideo,Caps,sizeof(*Caps));
}


/*******************************************************************************
   Function   : DlgVideoSource
   Arguments  : none
   Return     : See capDlgVideoSource()
   Description: Wrapper function for capDlgVideoSource().
*******************************************************************************/
BOOL CVFWImageProcessor::DlgVideoSource()
{
   // Reset any error conditions.
   GetPreviousError(NULL,NULL,TRUE);

   return PostThreadMessage(m_CaptureThread->m_nThreadID,UM_VID_SOURCE,0,0);

// return capDlgVideoSource(m_hWndVideo);
}


/*******************************************************************************
   Function   : DlgVideoFormat
   Arguments  : none
   Return     : See capDlgVideoFormat()
   Description: Wrapper function for capDlgVideoFormat()
*******************************************************************************/
BOOL CVFWImageProcessor::DlgVideoFormat()
{
   // Reset any error conditions.
   GetPreviousError(NULL,NULL,TRUE);

   return PostThreadMessage(m_CaptureThread->m_nThreadID,UM_VID_FORMAT,0,0);

//   return capDlgVideoFormat(m_hWndVideo);
}


/*******************************************************************************
   Function   : DlgVideoDisplay
   Arguments  : none
   Return     : See capDlgVideoDisplay()
   Description: Wrapper function for capDlgVideoDisplay()
*******************************************************************************/
BOOL CVFWImageProcessor::DlgVideoDisplay()
{
   // Reset any error conditions.
   GetPreviousError(NULL,NULL,TRUE);

   return PostThreadMessage(m_CaptureThread->m_nThreadID,UM_VID_DISPLAY,0,0);

// return capDlgVideoDisplay(m_hWndVideo);
}


/*******************************************************************************
   Function   : CancelCapture
   Arguments  : none
   Return     : none
   Description: Cancels current AVI capture.
*******************************************************************************/
VOID CVFWImageProcessor::CancelCapture()
{
   m_CancelCapture.SetEvent();
   capCaptureAbort(m_hWndVideo);
}


/*******************************************************************************
   Function   : AllocDIBImage
   Arguments  : ppImageData (output)     - Return pointer to allocated
                                           memory.  If passed as NULL,
                                           not used.
                AllocatedSize (output)   - Size of allocated block.
                                           If passed as NULL, not used.
   Return     : none
   Description: Alllocates image buffer for DIB capture.
*******************************************************************************/
BOOL CVFWImageProcessor::AllocDIBImage(PBITMAPINFO *ppImageData,
                                       ULONG *AllocatedSize)
{
   BOOL Ret = TRUE;
   DWORD Size = 0;

   // Reset any error conditions.
   GetPreviousError(NULL,NULL,TRUE);

   Size = CalcBitmapInfoSize(m_BitmapInfoHeader) + CalcBitmapSize(m_BitmapInfoHeader);

   if (Size > 0)
   {
      if (ppImageData)
      {
         *ppImageData = (BITMAPINFO *) new BYTE[Size];
         (**ppImageData).bmiHeader = m_BitmapInfoHeader;
      }
   }
   else
   {
      Ret = FALSE;
   }

   if (AllocatedSize)
   {
      *AllocatedSize = Size;
   }

   return Ret;
}



/*******************************************************************************
   Function   : GetBitmapInfoHeader()
   Arguments  : none
   Return     : BitmapInfo of capture device.
   Description: See return.
*******************************************************************************/
BITMAPINFOHEADER CVFWImageProcessor::GetBitmapInfoHeader()
{
   return m_BitmapInfoHeader;
}



/*******************************************************************************
   Function   : CalcBitmapSize()
   Arguments  : bmiHeader (input) - BITMAPINFOHEADER from which to calculate
                                    bitmap size.
   Return     : Size of Bitmap.
   Description: Calculates the size of a bitmap based upon the contents of
                the BITMAPINFOHEADER passed in.
*******************************************************************************/
ULONG CVFWImageProcessor::CalcBitmapSize(const BITMAPINFOHEADER &bmiHeader)

{
   ULONG Size = 0;

   if (bmiHeader.biSizeImage == 0)
   {
      Size = bmiHeader.biWidth *
             bmiHeader.biHeight *
             bmiHeader.biBitCount / 8;
   }
   else
   {
      Size = bmiHeader.biSizeImage;
   }

   return Size;
}


/*******************************************************************************
   Function   : CalcBitmapInfoSize()
   Arguments  : bmiHeader (input) - BITMAPINFOHEADER from which to calculate
                                    bitmap size.
   Return     : Size of Bitmap Info Header.
   Description: Calculates the size of a bitmap info header based upon the
                contents of the BITMAPINFOHEADER passed in.  This function
                can be used to determine the offset from the BITMAPINFOHEADER
                to the actual bitmap data.
*******************************************************************************/
ULONG CVFWImageProcessor::CalcBitmapInfoSize(const BITMAPINFOHEADER &bmiHeader)

{
   UINT bmiSize = (bmiHeader.biSize != 0) ? bmiHeader.biSize : sizeof(BITMAPINFOHEADER);
   return bmiSize + bmiHeader.biClrUsed * sizeof (RGBQUAD);
}


//
// Internal callback functions.
//
static LRESULT CALLBACK ErrorCallbackProc(HWND hWnd, int nErrID, LPSTR lpErrorText)
{
   CVFWImageProcessor *VFWObj = (CVFWImageProcessor *) capGetUserData(hWnd);

   if (VFWObj)
   {
      VFWObj->m_ErrorID = nErrID;
      VFWObj->m_ErrorText = lpErrorText;
   }

   return (LRESULT) TRUE;
}


static LRESULT CALLBACK ControlCallbackProc(HWND hWnd, int nState)
{
   CVFWImageProcessor *VFWObj = (CVFWImageProcessor *) capGetUserData(hWnd);
   LRESULT Ret = TRUE;

   switch(nState)
   {
      case CONTROLCALLBACK_PREROLL:
         if (VFWObj)
         {
            VFWObj->m_CancelCapture.ResetEvent();
         }

         Ret = TRUE;
         break;

      case CONTROLCALLBACK_CAPTURING:
         // if m_CancelCapture is posted, then we cancel AVI capture by returning FALSE.
         if (VFWObj)
         {
            Ret = (::WaitForSingleObject(VFWObj->m_CancelCapture,0) != WAIT_OBJECT_0);

            if (!Ret)
               TRACE("Callback Canceled Capture\n");
         }

         break;
   }

   return Ret;
}


static LRESULT CALLBACK StatusCallbackProc(HWND hWnd, int nID, LPCSTR lpsz)

{
   CVFWImageProcessor *VFWObj = (CVFWImageProcessor *) capGetUserData(hWnd);

   switch(nID)
   {
      case IDS_CAP_BEGIN:
         break;

      case IDS_CAP_END:
         break;
   }

   return (LRESULT) TRUE;
}


static LRESULT CALLBACK FrameCallbackProc(HWND hWnd, LPVIDEOHDR lpVHdr)

{
   CVFWImageProcessor *VFWObj = (CVFWImageProcessor *) capGetUserData(hWnd);
   LRESULT Ret = TRUE;

   if (VFWObj)
   {
      if (!VFWObj->m_hWndVideo)
      {
         Ret = FALSE;
      }
      else
      {
         CSingleLock ImageLockObj(&VFWObj->m_ImageProtect, TRUE);

         if (VFWObj->m_TransferBitmapInfo)
         {
            ULONG Size;

            VFWObj->m_TransferBitmapInfo->bmiHeader = VFWObj->m_BitmapInfoHeader;

            Size =  min(VFWObj->m_TransferBitmapInfoSize - VFWObj->CalcBitmapInfoSize(VFWObj->m_TransferBitmapInfo->bmiHeader),
                        lpVHdr->dwBytesUsed);

            memcpy(((CHAR *) VFWObj->m_TransferBitmapInfo) + VFWObj->CalcBitmapInfoSize(VFWObj->m_TransferBitmapInfo->bmiHeader),
                   lpVHdr->lpData,
                   Size);
         }

         ImageLockObj.Unlock();

         VFWObj->m_ImageReady.SetEvent();
      }
   }
   else
   {
      Ret = FALSE;
   }

   return Ret;
}


static LRESULT CALLBACK StreamCallbackProc(HWND hWnd, LPVIDEOHDR lpVHdr)

{
   CVFWImageProcessor *VFWObj = (CVFWImageProcessor *) capGetUserData(hWnd);
   LRESULT Ret = TRUE;

   return Ret;
}



static UINT CaptureThreadFunc(VOID *pCVFWImageProcessor)
{
   CVFWImageProcessor *pImageProc = (CVFWImageProcessor *) pCVFWImageProcessor;
   UINT Ret = 0;
   HWND VideoWindow;
   MSG Msg;

   VideoWindow = capCreateCaptureWindow(NULL,WS_POPUP,
                                        0,0,
                                        1,1,0,0);

   pImageProc->m_hWndVideo = VideoWindow;

   pImageProc->m_ImageReady.SetEvent();

   // Process window messages for VFW window until a WM_QUIT is posted.
   while(GetMessage(&Msg,NULL,0,0))
   {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);

      switch(Msg.message)
      {
         case UM_VID_SOURCE:
            capDlgVideoSource(pImageProc->m_hWndVideo);
            break;

         case UM_VID_FORMAT:
            capDlgVideoFormat(pImageProc->m_hWndVideo);
            break;

         case UM_VID_DISPLAY:
            capDlgVideoDisplay(pImageProc->m_hWndVideo);
            break;
      }
   }

   Ret = Msg.wParam;

// Ret = AfxGetThread()->Run();

   if (VideoWindow)
   {
      DestroyWindow(VideoWindow);
   }

   return Ret;
}

