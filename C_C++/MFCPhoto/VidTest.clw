; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CVidTestDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "vidtest.h"
LastPage=0

ClassCount=3
Class1=CVidTestApp
Class2=CAboutDlg
Class3=CVidTestDlg

ResourceCount=3
Resource1=IDD_VIDTEST_DIALOG (English (U.S.))
Resource2=IDD_ABOUTBOX (English (U.S.))
Resource3=IDR_MENU1 (English (U.S.))

[CLS:CVidTestApp]
Type=0
BaseClass=CWinApp
HeaderFile=VidTest.h
ImplementationFile=VidTest.cpp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=VidTestDlg.cpp
ImplementationFile=VidTestDlg.cpp

[CLS:CVidTestDlg]
Type=0
BaseClass=CDialog
HeaderFile=VidTestDlg.h
ImplementationFile=VidTestDlg.cpp
LastObject=ID_SETTINGS_VIDEOSOURCE

[DLG:IDD_VIDTEST_DIALOG]
Type=1
Class=CVidTestDlg

[DLG:IDD_ABOUTBOX (English (U.S.))]
Type=1
Class=?
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MENU1 (English (U.S.))]
Type=1
Class=?
Command1=ID_SETTINGS_VIDEOSOURCE
Command2=ID_SETTINGS_VIDEOFORMAT
Command3=ID_SETTINGS_VIDEODISPLAY
CommandCount=3

[DLG:IDD_VIDTEST_DIALOG (English (U.S.))]
Type=1
Class=?
ControlCount=3
Control1=IDC_VIDEO,static,1073741831
Control2=IDCAPTURE,button,1342242817
Control3=IDCANCEL,button,1342242816

