

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Wed Apr 06 10:15:54 2011
 */
/* Compiler settings for .\CameraPhoto.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

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

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, LIBID_CameraPhotoLib,0x54A399EC,0x2329,0x4EEC,0x86,0xE3,0x73,0x00,0x47,0x11,0x95,0x09);


MIDL_DEFINE_GUID(IID, DIID__DCameraPhoto,0x4BF027D0,0x97A5,0x4A8B,0x97,0x9C,0x93,0x59,0x42,0x66,0xC0,0xFE);


MIDL_DEFINE_GUID(IID, DIID__DCameraPhotoEvents,0x96DD868A,0x6A81,0x4D73,0x90,0x0B,0x85,0x8E,0x16,0x82,0x13,0xBC);


MIDL_DEFINE_GUID(CLSID, CLSID_CameraPhoto,0x22FBAE15,0x359B,0x45B1,0x81,0xA1,0x76,0xEC,0xA1,0x96,0x9A,0x71);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



