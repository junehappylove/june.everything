/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Tue Mar 29 16:42:28 2016
 */
/* Compiler settings for F:\Workspaces\git\GitHub\C_C++\study\FileTool\FileTool.odl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID LIBID_FileTool = {0x33A4EAC6,0xA4DA,0x11D2,{0x91,0xA2,0x00,0xA0,0xC9,0xD1,0x36,0xD1}};


const IID IID_ICommands = {0x33A4EAC2,0xA4DA,0x11D2,{0x91,0xA2,0x00,0xA0,0xC9,0xD1,0x36,0xD1}};


const CLSID CLSID_Commands = {0x33A4EAC3,0xA4DA,0x11D2,{0x91,0xA2,0x00,0xA0,0xC9,0xD1,0x36,0xD1}};


#ifdef __cplusplus
}
#endif

