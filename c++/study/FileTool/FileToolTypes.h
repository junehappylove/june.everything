/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Tue Mar 29 16:42:28 2016
 */
/* Compiler settings for F:\Workspaces\git\GitHub\C_C++\study\FileTool\FileTool.odl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __FileToolTypes_h__
#define __FileToolTypes_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __ICommands_FWD_DEFINED__
#define __ICommands_FWD_DEFINED__
typedef interface ICommands ICommands;
#endif 	/* __ICommands_FWD_DEFINED__ */


#ifndef __Commands_FWD_DEFINED__
#define __Commands_FWD_DEFINED__

#ifdef __cplusplus
typedef class Commands Commands;
#else
typedef struct Commands Commands;
#endif /* __cplusplus */

#endif 	/* __Commands_FWD_DEFINED__ */


void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __FileTool_LIBRARY_DEFINED__
#define __FileTool_LIBRARY_DEFINED__

/* library FileTool */
/* [helpstring][version][uuid] */ 


DEFINE_GUID(LIBID_FileTool,0x33A4EAC6,0xA4DA,0x11D2,0x91,0xA2,0x00,0xA0,0xC9,0xD1,0x36,0xD1);

#ifndef __ICommands_INTERFACE_DEFINED__
#define __ICommands_INTERFACE_DEFINED__

/* interface ICommands */
/* [object][dual][oleautomation][uuid] */ 


DEFINE_GUID(IID_ICommands,0x33A4EAC2,0xA4DA,0x11D2,0x91,0xA2,0x00,0xA0,0xC9,0xD1,0x36,0xD1);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("33A4EAC2-A4DA-11D2-91A2-00A0C9D136D1")
    ICommands : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddFileToProject( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OpenFile( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICommandsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICommands __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICommands __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICommands __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            ICommands __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            ICommands __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            ICommands __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            ICommands __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddFileToProject )( 
            ICommands __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenFile )( 
            ICommands __RPC_FAR * This);
        
        END_INTERFACE
    } ICommandsVtbl;

    interface ICommands
    {
        CONST_VTBL struct ICommandsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICommands_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICommands_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICommands_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICommands_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICommands_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICommands_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICommands_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICommands_AddFileToProject(This)	\
    (This)->lpVtbl -> AddFileToProject(This)

#define ICommands_OpenFile(This)	\
    (This)->lpVtbl -> OpenFile(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICommands_AddFileToProject_Proxy( 
    ICommands __RPC_FAR * This);


void __RPC_STUB ICommands_AddFileToProject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ICommands_OpenFile_Proxy( 
    ICommands __RPC_FAR * This);


void __RPC_STUB ICommands_OpenFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICommands_INTERFACE_DEFINED__ */


DEFINE_GUID(CLSID_Commands,0x33A4EAC3,0xA4DA,0x11D2,0x91,0xA2,0x00,0xA0,0xC9,0xD1,0x36,0xD1);

#ifdef __cplusplus

class DECLSPEC_UUID("33A4EAC3-A4DA-11D2-91A2-00A0C9D136D1")
Commands;
#endif
#endif /* __FileTool_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
