

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


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


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __CameraPhotoidl_h__
#define __CameraPhotoidl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef ___DCameraPhoto_FWD_DEFINED__
#define ___DCameraPhoto_FWD_DEFINED__
typedef interface _DCameraPhoto _DCameraPhoto;
#endif 	/* ___DCameraPhoto_FWD_DEFINED__ */


#ifndef ___DCameraPhotoEvents_FWD_DEFINED__
#define ___DCameraPhotoEvents_FWD_DEFINED__
typedef interface _DCameraPhotoEvents _DCameraPhotoEvents;
#endif 	/* ___DCameraPhotoEvents_FWD_DEFINED__ */


#ifndef __CameraPhoto_FWD_DEFINED__
#define __CameraPhoto_FWD_DEFINED__

#ifdef __cplusplus
typedef class CameraPhoto CameraPhoto;
#else
typedef struct CameraPhoto CameraPhoto;
#endif /* __cplusplus */

#endif 	/* __CameraPhoto_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __CameraPhotoLib_LIBRARY_DEFINED__
#define __CameraPhotoLib_LIBRARY_DEFINED__

/* library CameraPhotoLib */
/* [control][helpstring][helpfile][version][uuid] */ 


EXTERN_C const IID LIBID_CameraPhotoLib;

#ifndef ___DCameraPhoto_DISPINTERFACE_DEFINED__
#define ___DCameraPhoto_DISPINTERFACE_DEFINED__

/* dispinterface _DCameraPhoto */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__DCameraPhoto;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("4BF027D0-97A5-4A8B-979C-93594266C0FE")
    _DCameraPhoto : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _DCameraPhotoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _DCameraPhoto * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _DCameraPhoto * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _DCameraPhoto * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _DCameraPhoto * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _DCameraPhoto * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _DCameraPhoto * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _DCameraPhoto * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _DCameraPhotoVtbl;

    interface _DCameraPhoto
    {
        CONST_VTBL struct _DCameraPhotoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _DCameraPhoto_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define _DCameraPhoto_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define _DCameraPhoto_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define _DCameraPhoto_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define _DCameraPhoto_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define _DCameraPhoto_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define _DCameraPhoto_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___DCameraPhoto_DISPINTERFACE_DEFINED__ */


#ifndef ___DCameraPhotoEvents_DISPINTERFACE_DEFINED__
#define ___DCameraPhotoEvents_DISPINTERFACE_DEFINED__

/* dispinterface _DCameraPhotoEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__DCameraPhotoEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("96DD868A-6A81-4D73-900B-858E168213BC")
    _DCameraPhotoEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _DCameraPhotoEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _DCameraPhotoEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _DCameraPhotoEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _DCameraPhotoEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _DCameraPhotoEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _DCameraPhotoEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _DCameraPhotoEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _DCameraPhotoEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _DCameraPhotoEventsVtbl;

    interface _DCameraPhotoEvents
    {
        CONST_VTBL struct _DCameraPhotoEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _DCameraPhotoEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define _DCameraPhotoEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define _DCameraPhotoEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define _DCameraPhotoEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define _DCameraPhotoEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define _DCameraPhotoEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define _DCameraPhotoEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___DCameraPhotoEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_CameraPhoto;

#ifdef __cplusplus

class DECLSPEC_UUID("22FBAE15-359B-45B1-81A1-76ECA1969A71")
CameraPhoto;
#endif
#endif /* __CameraPhotoLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


