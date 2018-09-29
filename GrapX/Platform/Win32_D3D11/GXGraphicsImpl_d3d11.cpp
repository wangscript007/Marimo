#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#define _GXGRAPHICS_INLINE_CANVAS_D3D11_
#define _GXGRAPHICS_INLINE_RENDERTARGET_D3D11_
#define _GXGRAPHICS_INLINE_TEXTURE_D3D11_
#define _GXGRAPHICS_INLINE_SET_VERTEX_DECLARATION_D3D11_

// ȫ��ͷ�ļ�
#include <GrapX.h>
#include <User/GrapX.Hxx>

// ��׼�ӿ�
//#include "GrapX/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GRegion.h"
#include "GrapX/GPrimitive.h"
#include "GrapX/GShader.h"
#include "GrapX/GTexture.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GXCanvas.h"
#include "GrapX/GXImage.h"
#include "GrapX/GXFont.h"
#include "GrapX/GXCanvas3D.h"
#include "GrapX/GStateBlock.h"
#include "GrapX/MOLogger.h"

// ƽ̨���
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D11.h"
#include "Platform/Win32_D3D11/GPrimitiveImpl_d3d11.h"
#include "Platform/Win32_D3D11/GShaderImpl_d3d11.h"
#include "Platform/Win32_D3D11/GShaderStubImpl_d3d11.h"
#include "Platform/Win32_D3D11/GVertexDeclImpl_d3d11.h"

// ˽��ͷ�ļ�
#include <clPathFile.h>
#include "Platform/Win32_D3D11/GStateBlock_d3d11.h"
#include <GrapX/VertexDecl.h>
#include "Canvas/GXResourceMgr.h"
#include "Canvas/GXEffectImpl.h"
//#include "Console.h"
#include <Smart/SmartStream.h>
#include <Smart/SmartProfile.h>

#include "GrapX/GXKernel.h"
#include "GrapX/GXUser.h"
#include <GDI/RegionFunc.h>
#include <GDI/GRegionImpl.h>
#include "Platform/Win32_D3D11/GTextureImpl_d3d11.h"

#include "Canvas/GXImageImpl.h"
#include "Platform/Win32_D3D11/GXCanvasImpl_d3d11.h"

#include "GrapX/gxError.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <Canvas/GFTFontImpl.h>
#include <GDI/GXShaderMgr.h>

#include "Platform/Win32_D3D11/GXGraphicsImpl_d3d11.h"

// Canvas3D�õ�
#include "GrapX/GCamera.h"
#include "GrapX/GrapVR.h"  // Canvas3D �õ�
#include "Canvas/GXMaterialImpl.h"
// </Canvas3D�õ�>
#ifdef ENABLE_GRAPHICS_API_DX11

//////////////////////////////////////////////////////////////////////////
#define D3D11_GRAPHICS_IMPL

namespace D3D11
{
  typedef LPD3DINCLUDE LPD3DXINCLUDE;

#include "Platform/CommonInline/GXGraphicsImpl_Inline.inl"
#include "Platform/CommonInline/GXGraphicsImpl.inl"

  GXGraphics* GXGraphicsImpl::Create(const GRAPHICS_CREATION_DESC* pDesc)
  {
    GXGraphicsImpl* pGraphics = new GXGraphicsImpl;
    if( ! InlCheckNewAndIncReference(pGraphics)) {
      return NULL;
    }

    if( ! pGraphics->Initialize(pDesc)) {
      pGraphics->Release();
      return NULL;
    }

    return (GXGraphics*)pGraphics;
  }

  GXGraphicsImpl::GXGraphicsImpl()
    : s_uCanvasCacheCount  (4)
    , m_hWnd                (NULL)
    , m_driverType          (D3D_DRIVER_TYPE_NULL)
    , m_featureLevel        (D3D_FEATURE_LEVEL_11_0)
    , m_pd3dDevice          (NULL)
    , m_pImmediateContext   (NULL)
    , m_pSwapChain          (NULL)
    , m_pRenderTargetView   (NULL)
    , m_pDepthStencil       (NULL)
    , m_pDepthStencilView   (NULL)
    , m_dwFlags             (NULL)
    , m_pIdentity           (GXPLATFORM_UNKNOWN)
    , m_pSimpleShader       (NULL)
    , m_pBaseEffect       (NULL)
    , m_pCurRenderTargetView  (NULL)
    , m_pCurDepthStencilView  (NULL)
    , m_pVertexLayout       (NULL)
    , m_eCurTopology        (D3D_PRIMITIVE_TOPOLOGY_UNDEFINED)
    , m_pDeviceOriginTex    (NULL)
    , m_pCurPrimitive       (NULL)
    , m_pCurRenderTarget    (NULL)
    , m_pCurDepthStencil    (NULL)
    , m_aCanvasPtrCache     (NULL)
    , m_pCurVertexDecl      (NULL)
    , m_pCurCanvasCore      (NULL)
    , m_pCurRasterizerState (NULL)
    , m_pDefaultBlendState  (NULL)
    , m_pDefaultDepthStencilState(NULL)
    , m_pDefaultRasterizerState(NULL)
    //, m_pCurRenderState     (NULL)
    , m_pCurBlendState      (NULL)
    , m_pCurDepthStencilState(NULL)
    , m_pCurSamplerState    (NULL)
    , m_pCurShader          (NULL)
    , m_pBackBufferTex      (NULL)
    , m_pBackBufferImg      (NULL)
    , m_dwBackBufferStencil (1)
    , m_pGraphicsLocker     (NULL)
    , m_nGraphicsCount      (0)
    , m_dwThreadId          (0)
    //, m_pConsole            (NULL)
    , m_pLogger             (NULL)
    , m_pShaderConstName    (NULL)
  {
    memset(m_pCurTexture, 0, sizeof(GTexture*) * MAX_TEXTURE_STAGE);
    m_pRgnAllocator = new GAllocator(NULL);
  }

  HRESULT GXGraphicsImpl::Initialize(const GRAPHICS_CREATION_DESC* pDesc)
  {
    m_strResourceDir = pDesc->szRootDir;
    m_hWnd = pDesc->hWnd;
    m_dwFlags |= F_CREATEDEVICE;

    if(pDesc->pLogger) {
      m_pLogger = pDesc->pLogger;
      m_pLogger->AddRef();
    }
    else {
      clStringW strFilePath;
      clpathfile::CombinePath(strFilePath, pDesc->szRootDir, L"grapx.log");
      MOCreateFileLoggerW(&m_pLogger, strFilePath, FALSE);
    }


    //// �������
    //m_pConsole = new GXConsole;
    //if(m_pConsole->Initialize(this) != 0) {
    //  TRACE("Console ����ʧ��!\n");
    //  ASSERT(0);
    //}

    // ������־
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    static D3D_DRIVER_TYPE driverTypes[] =
    {
      D3D_DRIVER_TYPE_HARDWARE,
      D3D_DRIVER_TYPE_WARP,
      D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = CLARRAYSIZE( driverTypes );

    static D3D_FEATURE_LEVEL featureLevels[] =
    {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = CLARRAYSIZE( featureLevels );

    RECT rcWndClient;
    GetClientRect(pDesc->hWnd, &rcWndClient);
    ASSERT(rcWndClient.left == 0 && rcWndClient.top == 0);

    DXGI_SWAP_CHAIN_DESC& sd = m_SwapChainDesc;
    InlSetZeroT(sd);
    sd.BufferCount        = 1;
    sd.BufferDesc.Width   = rcWndClient.right;
    sd.BufferDesc.Height  = rcWndClient.bottom;
    sd.BufferDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow       = m_hWnd;
    sd.SampleDesc.Count   = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed           = TRUE;

    HRESULT hval = E_FAIL;

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
      m_driverType = driverTypes[driverTypeIndex];
      hval = D3D11CreateDeviceAndSwapChain( NULL, m_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
        D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext );
      if( SUCCEEDED( hval ) )
        break;
    }
    if( FAILED( hval ) )
      return hval;

    //////////////////////////////////////////////////////////////////////////
    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
    hval = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if( FAILED( hval ) )
      return hval;

    hval = m_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hval ) )
      return hval;
    // ==========
    m_pCurRenderTargetView = m_pRenderTargetView;
    m_pCurRenderTargetView->AddRef();

    //////////////////////////////////////////////////////////////////////////
    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = rcWndClient.right;
    descDepth.Height = rcWndClient.bottom;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hval = m_pd3dDevice->CreateTexture2D( &descDepth, NULL, &m_pDepthStencil );
    if( FAILED( hval ) )
      return hval;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hval = m_pd3dDevice->CreateDepthStencilView( m_pDepthStencil, &descDSV, &m_pDepthStencilView );
    if( FAILED( hval ) )
      return hval;

    // ==========
    m_pCurDepthStencilView = m_pDepthStencilView;
    m_pCurDepthStencilView->AddRef();


    //////////////////////////////////////////////////////////////////////////
    m_pImmediateContext->OMSetRenderTargets( 1, &m_pCurRenderTargetView, m_pCurDepthStencilView );
    m_pImmediateContext->ClearDepthStencilView( m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)sd.BufferDesc.Width;
    vp.Height = (FLOAT)sd.BufferDesc.Height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_pImmediateContext->RSSetViewports( 1, &vp );

    m_pIdentity = GXPLATFORM_WIN32_DIRECT3D11;
    //m_pGraphicsLocker = new clstd::Locker;
    InitCommon();

    if(GXSUCCEEDED(CreateShaderFromFileA(&m_pSimpleShader, "Shader\\Simple.shader.txt")))
    {
      CreateEffect(&m_pBaseEffect, m_pSimpleShader);
    }
    else
    {
      TRACE("��ʼ��SimpleShaderʧ��!\n");
    }

    return TRUE;
  }


#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  GXHRESULT GXGraphicsImpl::AddRef()
  {
    return gxInterlockedIncrement(&m_nRefCount);
  }
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  GXHRESULT GXGraphicsImpl::Release()
  {
    GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);

    if(nRefCount > 0) {
      return nRefCount;
    }

    SAFE_RELEASE(m_pBaseEffect);
    SAFE_RELEASE(m_pSimpleShader);

    ReleaseCommon();

    SAFE_RELEASE(m_pBackBufferTex);
    SAFE_RELEASE(m_pBackBufferImg);

    SAFE_RELEASE(m_pCurShader);
    //SAFE_RELEASE(m_pCurRenderState);
    SAFE_RELEASE(m_pCurPrimitive);
    for(int i = 0; i < MAX_TEXTURE_STAGE; i++) {
      SAFE_RELEASE(m_pCurTexture[i]);
    }
    SAFE_RELEASE(m_pCurDepthStencil);
    SAFE_RELEASE(m_pCurRenderTarget);
    SAFE_RELEASE(m_pCurCanvasCore);

    SAFE_RELEASE(m_pCurVertexDecl);
    //OnDeviceEvent(DE_LostDevice);
    INVOKE_LOST_DEVICE;
    if(m_dwFlags & F_CREATEDEVICE)
    {
      SAFE_RELEASE(m_pd3dDevice);
      m_hWnd = NULL;
    }
    SAFE_RELEASE(m_pVertexLayout);
    SAFE_RELEASE(m_pDepthStencil);
    SAFE_RELEASE(m_pDepthStencilView);
    SAFE_RELEASE(m_pCurDepthStencilView);
    SAFE_RELEASE(m_pCurRenderTargetView);
    SAFE_RELEASE(m_pImmediateContext);
    SAFE_RELEASE(m_pSwapChain);
    SAFE_RELEASE(m_pRenderTargetView);
    SAFE_DELETE(m_pRgnAllocator);

//#ifdef _DEBUG
//    if(m_aResource.size() > 0)
//      TRACE("GXGraphics �ͷź��� %d ������û�������ͷ�.\n", m_aResource.size());
//#endif // _DEBUG
    //m_pConsole->Finalize();
    //delete m_pConsole;
    SAFE_RELEASE(m_pLogger);

    SAFE_DELETE(m_pGraphicsLocker);
    delete this;
    return GX_OK;
  }

  GXHRESULT GXGraphicsImpl::Invoke(GRESCRIPTDESC* pDesc)
  {
    INVOKE_DESC_CHECK(pDesc);
    switch(pDesc->dwCmdCode)
    {
    case RC_LostDevice:
      {
        InlSetCanvas(NULL);
        InlSetShader(NULL);
        InlSetVertexDecl(NULL);
        SetPrimitiveVI(NULL, 0);
        for(int i = 0; i < MAX_TEXTURE_STAGE; i++) {
          InlSetTexture(NULL, i);
        }

        SAFE_RELEASE(m_pDeviceOriginTex);
      }
      break;
    case RC_ResetDevice:
      {
        //m_pCurRenderState->Update(NULL);
        m_pCurSamplerState->Activate(NULL);
      }
      break;
    case RC_ResizeDevice:
      {

      }
      break;
    }
    return FALSE;
  }
  void GXGraphicsImpl::GetPlatformID(GXPlaformIdentity* pIdentity)
  {
    ASSERT(m_pIdentity == GXPLATFORM_WIN32_DIRECT3D11);
    *pIdentity = m_pIdentity;
  }

  GXBOOL GXGraphicsImpl::Activate(GXBOOL bActive)
  {
    if(bActive != FALSE) {
      SET_FLAG(m_dwFlags, F_ACTIVATE);
    }
    else {
      RESET_FLAG(m_dwFlags, F_ACTIVATE);

      // TODO: ��������Ϊʲô����Ҫ�ָ�ԭ����BackBuffer
      if(m_pCurRenderTarget != m_pDeviceOriginTex) {
        InlSetCanvas(NULL);
      }

#if defined(_DEBUG) && 0
      LPDIRECT3DSURFACE9 lpRenderTar;
      m_pd3dDevice->GetRenderTarget(0, &lpRenderTar);
      ASSERT(lpRenderTar == m_pCurRenderTarget->D3DSurface());
      lpRenderTar->Release();
#endif

      // 2012-08-04 û�б�Ҫ�ָ���Ⱦ�����!
      //GXREGN regn = {0,0,m_d3dpp.BackBufferWidth,m_d3dpp.BackBufferHeight};
      //SetSafeClip(&regn);
    }
    return TRUE;
  }

  GXHRESULT GXGraphicsImpl::Test()
  {
    //const GXHRESULT hr = m_pd3dDevice->TestCooperativeLevel();

    //switch(hr)
    //{
    //case D3DERR_DEVICELOST:
    //  {
    //    m_pGraphicsLocker->Lock();
    //    TRACE("D3DERR_DEVICELOST\n");
    //    OnDeviceEvent(DE_LostDevice);
    //    CallResourceEvent(DE_LostDevice);
    //    m_dwFlags |= F_DEVICEHASLOST;
    //  }
    //  break;
    //case D3DERR_DEVICENOTRESET:
    //  {
    //    TRACE("D3DERR_DEVICENOTRESET\n");
    //    if((m_dwFlags & F_DEVICEHASLOST) == 0)
    //    {
    //      OnDeviceEvent(DE_LostDevice);
    //      CallResourceEvent(DE_LostDevice);
    //    }

    //    // Reset ��Ҫ�ڴ���D3D�豸���߳�������
    //    if(m_dwFlags & F_CREATEDEVICE)
    //    {
    //      GXHRESULT hr = SendMessage(m_hWnd, WM_GX_RESETDEVICE, NULL, (LPARAM)&m_d3dpp);
    //      V(hr);
    //    }
    //    OnDeviceEvent(DE_ResetDevice);
    //    CallResourceEvent(DE_ResetDevice);
    //    m_dwFlags &= (~F_DEVICEHASLOST);
    //    m_pGraphicsLocker->Unlock();
    //  }
    //  break;
    //case D3DERR_DRIVERINTERNALERROR:
    //  {
    //    TRACE("D3DERR_DRIVERINTERNALERROR\n");
    //  }
    //  break;
    //}
    //return hr;
    return 0;
  }
  GXHRESULT GXGraphicsImpl::Begin()
  {
    m_pGraphicsLocker->Lock();
    if((++m_nGraphicsCount) == 1)
    {
      if(GXSUCCEEDED(Test()) /*&&
        /*GXSUCCEEDED(m_pImmediateContext->Begin())*/)
      {
        Activate(TRUE);
        m_dwThreadId = GetCurrentThreadId();
        return m_nGraphicsCount;
      }
      --m_nGraphicsCount;
      m_pGraphicsLocker->Unlock();
      return GX_FAIL;
    }
    return m_nGraphicsCount;
  }
  GXHRESULT GXGraphicsImpl::End()
  {
    GXINT nCount = --m_nGraphicsCount;
    if(nCount == 0)
    {
      if(TEST_FLAG(m_dwFlags, F_SHOWCONSOLE))
      {
        GXCanvas* pCanvas = LockCanvas(NULL, NULL, NULL);
        //m_pConsole->Render(pCanvas);
        SAFE_RELEASE(pCanvas);
      }

      ASSERT(m_dwThreadId == GetCurrentThreadId());
      m_dwThreadId = NULL;
      Activate(FALSE);
      //HRESULT hval = m_pd3dDevice->EndScene();
      //ASSERT(SUCCEEDED(hval));
    }
    m_pGraphicsLocker->Unlock();
    return nCount;
  }
  GXHRESULT GXGraphicsImpl::Present()
  {
    m_pGraphicsLocker->Lock();  // TODO: ������ô��ʡ���Lock
    HRESULT hval =  m_pSwapChain->Present(0, 0);
#ifdef _DEBUG
    // ���߳�ʱ�ı��豸��С���ܻᵼ��ʧ��
    if(GXFAILED(hval)) {
      TRACE("Warning: Present failed(Code: %d).\n", hval);
    }
#endif // _DEBUG
    m_pGraphicsLocker->Unlock();
    return hval;
  }

  GXHRESULT GXGraphicsImpl::Resize(int nWidth, int nHeight)
  {
    ASSERT(0);
    //OnDeviceEvent(DE_LostDevice);
    //CallResourceEvent(DE_LostDevice);

    //m_d3dpp.BackBufferWidth = nWidth;
    //m_d3dpp.BackBufferHeight = nHeight;
    //m_pd3dDevice->Reset(&m_d3dpp);

    //OnDeviceEvent(DE_ResetDevice);
    //CallResourceEvent(DE_ResetDevice);

    //Activate(FALSE);
    return GX_OK;
  }

  GXHRESULT GXGraphicsImpl::SetPrimitiveV(GPrimitiveV* pPrimitive, GXUINT uStreamSource)
  {
    if(m_pCurPrimitive == pPrimitive)
      return S_OK;

    if(m_pCurPrimitive != NULL)
      m_pCurPrimitive->Release();

    m_pCurPrimitive = pPrimitive;
    if(m_pCurPrimitive == NULL)
      return S_OK;

    m_pCurPrimitive->AddRef();

    // Ӧ�ö�������
    GPrimitiveVImpl* pPrimImpl = static_cast<GPrimitiveVImpl*>(pPrimitive);
    if(pPrimImpl->m_pVertexDecl != NULL) {
      InlSetVertexDecl(pPrimImpl->m_pVertexDecl);
    }

    GPrimitiveVImpl* pPrimitiveImpl = (GPrimitiveVImpl*) pPrimitive;
    UINT offset = 0;
    m_pImmediateContext->IASetVertexBuffers( 0, 1, &pPrimitiveImpl->m_pVertexBuffer, &pPrimitiveImpl->m_uElementSize, &offset );
    //m_pd3dDevice->SetStreamSource(uStreamSource, pPrimitiveImpl->m_pVertexBuffer, 
    //  0, pPrimitiveImpl->m_uElementSize);

    return S_OK;
  }
  
  GXHRESULT GXGraphicsImpl::SetPrimitiveVI(GPrimitiveVI* pPrimitive, GXUINT uStreamSource)
  {
    if(m_pCurPrimitive == pPrimitive)
      return S_OK;

    SAFE_RELEASE(m_pCurPrimitive);
    m_pCurPrimitive = pPrimitive;

    if(m_pCurPrimitive == NULL)
      return S_OK;

    m_pCurPrimitive->AddRef();

    // Ӧ�ö�������
    GPrimitiveVIImpl* pPrimImpl = static_cast<GPrimitiveVIImpl*>(pPrimitive);
    if(pPrimImpl->m_pVertexDecl != NULL) {
      InlSetVertexDecl(pPrimImpl->m_pVertexDecl);
    }


    GPrimitiveVIImpl* pPrimitiveImpl = (GPrimitiveVIImpl*)pPrimitive;
    UINT offset = 0;
    m_pImmediateContext->IASetVertexBuffers( 0, 1, &pPrimitiveImpl->m_pVertexBuffer, 
      &pPrimitiveImpl->m_uElementSize, &offset );

    m_pImmediateContext->IASetIndexBuffer(pPrimitiveImpl->m_pIndexBuffer, DXGI_FORMAT_R16_UINT, offset);
    //m_pd3dDevice->SetStreamSource(uStreamSource, pPrimitiveImpl->m_pVertexBuffer, 
    //  0, pPrimitiveImpl->m_uElementSize);

    //m_pd3dDevice->SetIndices(pPrimitiveImpl->m_pIndexBuffer);

    return S_OK;
  }
  
  GXHRESULT GXGraphicsImpl::SetTexture(GTextureBase* pTexture, GXUINT uStage)
  {
    return InlSetTexture(reinterpret_cast<GTexBaseImpl*>(pTexture), uStage);
  }

  GXBOOL GXGraphicsImpl::SetSafeClip(const GXREGN* pRegn)
  {
#ifdef _43A2DE06_6673_4ddd_9C1C_881493B776A0_
    D3DVIEWPORT9 Viewport;
    Viewport.X      = (GXDWORD)pRegn->left;
    Viewport.Y      = (GXDWORD)pRegn->top;
    Viewport.Width  = (GXDWORD)pRegn->width;
    Viewport.Height = (GXDWORD)pRegn->height;
    Viewport.MinZ   = 0.0f;
    Viewport.MaxZ   = 1.0f;

    GXUINT nWidth, nHeight;
    m_pCurRenderTarget->GetDimension(&nWidth, &nHeight);

    //m_matWorld._11 = ( 2.0f / (float)nWidth)  * ((float)nWidth  / (float)Viewport.Width);
    //m_matWorld._22 = (-2.0f / (float)nHeight) * ((float)nHeight / (float)Viewport.Height);
    m_matWorld._11 = ( 2.0f / (float)Viewport.Width);
    m_matWorld._22 = (-2.0f / (float)Viewport.Height);
    m_matWorld._41 = -1.0f - ((float)pRegn->left * 2.0f / (float)Viewport.Width);
    m_matWorld._42 =  1.0f + ((float)pRegn->top  * 2.0f / (float)Viewport.Height);

    //SetVertexShaderConstantF(FXVCOMMREG(matWVProj), (const float*)&m_matWorld, 4);

#ifdef _DEBUG
    if(m_pCurCanvas != NULL && m_pCurCanvas->m_pTargetTex != NULL)
    {
      ASSERT(pRegn->left >= 0 && pRegn->top >= 0);
      GXLONG nScreenWidth, nScreenHeight;
      m_pCurCanvas->m_pTargetTex->GetDimension((GXUINT*)&nScreenWidth, (GXUINT*)&nScreenHeight);
      ASSERT(pRegn->left + pRegn->width <= nScreenWidth && 
        pRegn->top + pRegn->height <= nScreenHeight);
    }
#endif // _DEBUG
    return GXSUCCEEDED(m_pd3dDevice->SetViewport(&Viewport));
#else
    GXRECT rect;
    gxRegnToRect(&rect, (const GXLPREGN)pRegn);
    //m_pCurRenderState->Set(GXRS_SCISSORTESTENABLE, TRUE);
    m_pImmediateContext->RSSetScissorRects(1, (RECT*)&rect);

    //HRESULT hval = m_pd3dDevice->SetScissorRect((RECT*)&rect);
//#ifdef _DEBUG
//    if(GXFAILED(hval))
//    {
//      TRACE(__FUNCTION__" failed:%d", hval);
//    }
//#endif // _DEBUG
//    return GXSUCCEEDED(hval);
    return TRUE;
#endif
  }
  GXBOOL GXGraphicsImpl::SetViewport(const GXVIEWPORT* pViewport)
  {
    D3D11_VIEWPORT Viewport;
    Viewport.TopLeftX = (FLOAT)pViewport->regn.left;
    Viewport.TopLeftY = (FLOAT)pViewport->regn.top;
    Viewport.Width    = (FLOAT)pViewport->regn.width;
    Viewport.Height   = (FLOAT)pViewport->regn.height;
    Viewport.MinDepth = pViewport->fNear;
    Viewport.MaxDepth = pViewport->fFar;

    HRESULT hval = GX_OK;
    m_pImmediateContext->RSSetViewports(1, &Viewport);
//#ifdef _DEBUG
//    if(GXFAILED(hval))
//    {
//      TRACE(__FUNCTION__" failed:%d", hval);
//    }
//#endif // _DEBUG
    return GXSUCCEEDED(hval);
  }
  //GXBOOL GXGraphicsImpl::Clear(const GXRECT*lpRects, GXUINT nCount, D3DCOLOR crClear)
  //{
  //  return GXSUCCEEDED(m_pd3dDevice->Clear(nCount, (const D3DRECT*)lpRects, D3DCLEAR_TARGET, crClear, 1.0f, NULL));
  //}
  GXBOOL GXGraphicsImpl::IntCheckSizeOfTargetAndDepth()
  {
    if(m_pCurDepthStencil == 0 && m_pCurRenderTarget == 0) {
      return TRUE;
    }
    GXINT nDepthSurWidth;
    GXINT nDepthSurHeight;
    GXINT nColorSurWidth;
    GXINT nColorSurHeight;

    if(m_pCurDepthStencil != NULL) {
      nDepthSurWidth = m_pCurDepthStencil->m_nWidth;
      nDepthSurHeight = m_pCurDepthStencil->m_nHeight;
    }
    else {
      nDepthSurWidth = m_SwapChainDesc.BufferDesc.Width;//m_d3dpp.BackBufferWidth;
      nDepthSurHeight = m_SwapChainDesc.BufferDesc.Height;
    }

    if(m_pCurRenderTarget != NULL) {
      nColorSurWidth = m_pCurRenderTarget->m_nWidth;
      nColorSurHeight = m_pCurRenderTarget->m_nHeight;
    }
    else {
      nColorSurWidth = m_SwapChainDesc.BufferDesc.Width;//m_d3dpp.BackBufferWidth;
      nColorSurHeight = m_SwapChainDesc.BufferDesc.Height;//m_d3dpp.BackBufferHeight;
    }

    if(nDepthSurWidth >= nColorSurWidth &&
      nDepthSurHeight >= nColorSurHeight)
    {
      return TRUE;
    }
    TRACE(" ! Depth and stencil buffer size is less than back buffer\n");
    return FALSE;    
  }

  GXHRESULT GXGraphicsImpl::Clear(const GXRECT*lpRects, GXUINT nCount, GXDWORD dwFlags, GXCOLOR crClear, float z, GXDWORD dwStencil)
  {
    if(TEST_FLAG(dwFlags, GXCLEAR_TARGET)) {
      GXColor color = crClear;
      m_pImmediateContext->ClearRenderTargetView(m_pCurRenderTargetView, (const FLOAT*)&color);
    }

    // TODO: ���ԵĴ���
    m_pImmediateContext->ClearDepthStencilView( m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
    // /���ԵĴ���

    // �������� "GXCLEAR_*" �� "D3D11_CLEAR_*" �Ķ�����ֵ��Ҫ��, �������ı�
    UINT uClearFlags = (dwFlags & (GXCLEAR_DEPTH|GXCLEAR_STENCIL)) >> 1;

    if(uClearFlags != 0) {
      m_pImmediateContext->ClearDepthStencilView(m_pCurDepthStencilView, uClearFlags, z, (UINT8)dwStencil);
    }
    return 0;

    STATIC_ASSERT((GXCLEAR_DEPTH >> 1) == D3D11_CLEAR_DEPTH);
    STATIC_ASSERT((GXCLEAR_STENCIL >> 1) == D3D11_CLEAR_STENCIL);
  }

  GXVOID GXGraphicsImpl::BuildInputLayout()
  {
    ASSERT(m_pVertexLayout == NULL);
    GShaderImpl* pShaderImpl = static_cast<GShaderImpl*>(m_pCurShader);
    D3D11_INPUT_ELEMENT_DESC* pDesc = (D3D11_INPUT_ELEMENT_DESC*)&m_pCurVertexDecl->m_aDescs.front();
    m_pd3dDevice->CreateInputLayout(
      pDesc, m_pCurVertexDecl->m_NumDescs, pShaderImpl->m_pVertexBuf->GetPtr(), 
      pShaderImpl->m_pVertexBuf->GetSize(), &m_pVertexLayout);
    m_pImmediateContext->IASetInputLayout(m_pVertexLayout);
  }

  GXHRESULT GXGraphicsImpl::DrawPrimitive(const GXPrimitiveType eType, const GXUINT StartVertex, 
    const GXUINT PrimitiveCount)
  {
    ASSERT(TEST_FLAG(m_dwFlags, F_ACTIVATE));
#ifdef _DEBUG
    IntCheckSizeOfTargetAndDepth();
#endif // IntCheckSizeOfTargetAndDepth

    GXUINT nVertCount = 0;
    InlUpdateTopology(eType, PrimitiveCount, &nVertCount);

    if(m_pVertexLayout == NULL) {
      BuildInputLayout();
    }
    
    GShaderImpl* pShaderImpl = static_cast<GShaderImpl*>(m_pCurShader);
    pShaderImpl->CheckUpdateConstBuf();
    //m_pCurRenderState->IntCheckUpdate();

    // NOTE: Ϊë����Ҫ�ύ����״̬��?
    //m_pImmediateContext->PSSetSamplers(0, SAMPLERCOUNT, m_pCurSamplerState->GetSamplers());

    m_pImmediateContext->Draw(nVertCount, StartVertex); // FIXME: ��һ����������
    return 0;
  }

  GXHRESULT GXGraphicsImpl::DrawPrimitive(
    const GXPrimitiveType eType, const GXINT BaseVertexIndex, 
    const GXUINT MinIndex, const GXUINT NumVertices, 
    const GXUINT StartIndex, const GXUINT PrimitiveCount)
  {
#ifdef D3D11_LOW_DEBUG
    LPDIRECT3DPIXELSHADER9 pPixelShader;
    LPDIRECT3DVERTEXSHADER9 pVertexShader;
    m_pd3dDevice->GetPixelShader(&pPixelShader);
    m_pd3dDevice->GetVertexShader(&pVertexShader);
    ASSERT(pPixelShader != NULL && pVertexShader != NULL);
    SAFE_RELEASE(pPixelShader);
    SAFE_RELEASE(pVertexShader);

    LPDIRECT3DINDEXBUFFER9 pIndexBuf;
    m_pd3dDevice->GetIndices(&pIndexBuf);
    ASSERT(pIndexBuf != NULL);
    SAFE_RELEASE(pIndexBuf);

#endif // D3D11_LOW_DEBUG
#ifdef _DEBUG
    IntCheckSizeOfTargetAndDepth();
#endif // IntCheckSizeOfTargetAndDepth

    UINT nIndexCount = 0;
    InlUpdateTopology(eType, PrimitiveCount, &nIndexCount);

    if(m_pVertexLayout == NULL) {
      BuildInputLayout();
    }
    else {
      m_pImmediateContext->IASetInputLayout(m_pVertexLayout);
    }

    GShaderImpl* pShaderImpl = static_cast<GShaderImpl*>(m_pCurShader);
    pShaderImpl->CheckUpdateConstBuf();
    //m_pCurRenderState->IntCheckUpdate();

    // NOTE: Ϊë����Ҫ�ύ����״̬��?
    //m_pImmediateContext->PSSetSamplers(0, SAMPLERCOUNT, m_pCurSamplerState->GetSamplers());

    ASSERT(TEST_FLAG(m_dwFlags, F_ACTIVATE));

    m_pImmediateContext->DrawIndexed(nIndexCount, StartIndex, BaseVertexIndex); // FIXME: ��һ����������
    return 0;
  }


  GXHRESULT GXGraphicsImpl::CreateTexture(GTexture** ppTexture, GXLPCSTR szName, GXUINT Width, GXUINT Height, GXUINT MipLevels, 
    GXFormat Format, GXDWORD ResUsage)
  {
    GTextureFromUser *pGTex = NULL;
    *ppTexture = NULL;

    if(TEST_FLAG(ResUsage, GXRU_TEX_RENDERTARGET)) {
      pGTex = new GTextureFromUserRT(this);
    }
    else {
      pGTex = new GTextureFromUser(this);
    }
    
    if( ! InlCheckNewAndIncReference(pGTex)) {
      return GX_FAIL;
    }

    if( ! pGTex->Initialize(Width, Height, MipLevels, Format, ResUsage))
    {
      pGTex->Release();
      pGTex = NULL;
      return GX_FAIL;
    }

    //if(pGTex != NULL)
    //{
    //  pGTex->m_emType       = GTextureImpl::User;
    //  pGTex->m_nWidth       = 0;
    //  pGTex->m_nHeight      = 0;
    //  pGTex->m_nMipLevels   = MipLevels;
    //  pGTex->m_Format       = Format;
    //  pGTex->m_dwResUsage   = ResUsage;
    //  pGTex->m_nWidthRatio  = (GXWORD)Width;
    //  pGTex->m_nHeightRatio = (GXWORD)Height;

    //  GXHRESULT hval = pGTex->AddRef();
    //  if(GXFAILED(hval))
    //  {
    //    SAFE_RELEASE(pGTex);
    //    return GX_FAIL;
    //  }
    RegisterResource(pGTex, NULL);
    *ppTexture = pGTex;
    return GX_OK;
    //}
    //return GX_FAIL;
  }

  GXHRESULT GXGraphicsImpl::CreateTextureFromFileW(GTexture** ppTexture, GXLPCWSTR pSrcFile)
  {
    GXHRESULT lr = CreateTextureFromFileExW(ppTexture, pSrcFile, 
      GX_DEFAULT, GX_DEFAULT, GX_FROM_FILE, GXFMT_UNKNOWN, 
      GXRU_DEFAULT, GX_DEFAULT, GX_DEFAULT, 0, NULL);
    if(GXSUCCEEDED(lr))
    {
      ((GTextureFromFile*)*ppTexture)->m_emType = GTextureImpl::File;
    }
    return lr;
  }

  GXHRESULT GXGraphicsImpl::CreateTextureFromFileExW(
    GTexture** ppTexture, GXLPCWSTR pSrcFile, GXUINT Width, GXUINT Height, 
    GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, GXDWORD Filter /* = D3DX_FILTER_NONE */, 
    GXDWORD MipFilter /* = D3DX_FILTER_NONE */, GXCOLORREF ColorKey /* = 0 */, 
    OUT LPGXIMAGEINFOX pSrcInfo)
  {
    *ppTexture = NULL;
    if(!MarimoVerifier::Texture::CreateFromFileParam("CreateTextureFromFileExW Error: ",
      Width, Height, 1, MipLevels, Format, ResUsage, Filter, MipFilter)) {
        return GX_FAIL;
    }

    clstd::ScopedLocker sl(m_pGraphicsLocker);
    GTextureFromFile *pGTex;
    pGTex = new GTextureFromFile(pSrcFile, Width, Height, MipLevels, Format, ResUsage, Filter, MipFilter, ColorKey, this);
    m_pLogger->OutputFormatW(L"Load texture from file: %s", pSrcFile);
    if(pGTex != NULL)
    {
      pGTex->AddRef();

      if(GXFAILED(pGTex->Create(pSrcInfo)))
      {
        SAFE_RELEASE(pGTex);
        ASSERT(FALSE);
        m_pLogger->OutputFormatW(L"...Failed.\n");
        return GX_FAIL;
      }

      RegisterResource(pGTex, NULL);
      m_pLogger->OutputFormatW(L"...Succeeded.\n");
      *ppTexture = pGTex;
      return GX_OK;
    }  
    m_pLogger->OutputFormatW(L"...Failed.\n");
    ASSERT(FALSE);
    return GX_FAIL;
  }

  GXHRESULT GXGraphicsImpl::CreateTexture3D(
    GTexture3D** ppTexture, GXLPCSTR szName, 
    GXUINT Width, GXUINT Height, GXUINT Depth, 
    GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage)
  {
    CLBREAK;
    return GX_OK;
  }

  GXHRESULT GXGraphicsImpl::CreateTexture3DFromFileW(
    GTexture3D** ppTexture, GXLPCWSTR pSrcFile)
  {
    return CreateTexture3DFromFileExW(ppTexture, pSrcFile, GX_DEFAULT, GX_DEFAULT, GX_DEFAULT,
      GX_FROM_FILE,  GXFMT_UNKNOWN, GXRU_DEFAULT, GX_DEFAULT, GX_DEFAULT, 0, NULL);
  }

  GXHRESULT GXGraphicsImpl::CreateTexture3DFromFileExW(
    GTexture3D** ppTexture, GXLPCWSTR pSrcFile, 
    GXUINT Width, GXUINT Height, GXUINT Depth, 
    GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, 
    GXDWORD Filter, GXDWORD MipFilter, GXCOLORREF ColorKey, GXOUT LPGXIMAGEINFOX pSrcInfo)
  {
    CLBREAK;
    return GX_OK;
  }

  GXHRESULT GXGraphicsImpl::CreateTextureCube(GTextureCube** ppTexture, 
    GXLPCSTR szName, GXUINT Size, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage)
  {
    CLBREAK;
    return GX_OK;
  }

  GXHRESULT GXGraphicsImpl::CreateTextureCubeFromFileW(GTextureCube** ppTexture, GXLPCWSTR pSrcFile)
  {
    CLBREAK;
    return GX_OK;
  }

  GXHRESULT GXGraphicsImpl::CreateTextureCubeFromFileExW(GTextureCube** ppTexture, 
    GXLPCWSTR pSrcFile, GXUINT Size, GXUINT MipLevels, GXFormat Format, GXDWORD ResUsage, 
    GXDWORD Filter, GXDWORD MipFilter, GXCOLORREF ColorKey, GXOUT LPGXIMAGEINFOX pSrcInfo)
  {
    CLBREAK;
    return GX_OK;
  }


  //GXLRESULT GXGraphicsImpl::CreateOffscreenPlainSurface(
  //  GTexture** ppTexture, GXUINT Width, GXUINT Height, GXFormat Format, GXDWORD ResUsage)
  //{
  //  *ppTexture = NULL;
  //  GTextureOffscreenPlainSur *pGTex;
  //  pGTex = new GTextureOffscreenPlainSur(this);
  //  if(pGTex)
  //  {
  //    pGTex->AddRef();
  //    RegisterResource(pGTex, NULL);
  //    pGTex->m_emType     = GTextureImpl::OffscreenPlainSur;
  //    pGTex->m_nWidth     = Width;
  //    pGTex->m_nHeight    = Height;
  //    pGTex->m_nMipLevels = 1;
  //    pGTex->m_Format     = Format;
  //    pGTex->m_dwResUsage = ResUsage;

  //    GRESCRIPTDESC Desc;
  //    InlSetZeroT(Desc);
  //    Desc.dwCmdCode = RC_ResetDevice;
  //    if(pGTex->Invoke(&Desc) == FALSE)
  //    {
  //      SAFE_DELETE(pGTex);
  //      return GX_FAIL;
  //    }
  //    *ppTexture = pGTex;
  //    return GX_OK;
  //  }
  //  return GX_FAIL;
  //}

//////////////////////////////////////////////////////////////////////////

  GXBOOL GXGraphicsImpl::D3DGetSwapChainDesc(DXGI_SWAP_CHAIN_DESC* pSwapChainDesc)
  {
    *pSwapChainDesc = m_SwapChainDesc;
    return TRUE;
  }

  GXBOOL GXGraphicsImpl::GetDesc(GXGRAPHICSDEVICE_DESC* pDesc)
  {
    pDesc->cbSize             = sizeof(GXGRAPHICSDEVICE_DESC);
    pDesc->BackBufferWidth    = m_SwapChainDesc.BufferDesc.Width;
    pDesc->BackBufferHeight   = m_SwapChainDesc.BufferDesc.Height;
    pDesc->BackBufferCount    = m_SwapChainDesc.BufferCount;
    pDesc->BackBufferFormat   = (GXFormat)GXFMT_UNKNOWN;//m_SwapChainDesc.BufferDesc.Format;  // FIXME: �������!
    pDesc->DepthStencilFormat = (GXFormat)GXFMT_UNKNOWN;
    pDesc->RefreshRateInHz    = m_SwapChainDesc.BufferDesc.RefreshRate.Numerator;
    pDesc->dwFlags            = NULL;
    return TRUE;
  }

  // ��˽�� inline �����Լ��� cpp �ļ���
  inline void GXGraphicsImpl::InlUpdateTopology(GXPrimitiveType eType, GXUINT nPrimCount, GXUINT* pVertCount)
  {
    const D3D_PRIMITIVE_TOPOLOGY eTopology = GrapXToDX11::PrimitiveTopology(eType, nPrimCount, pVertCount);
    if(m_eCurTopology != eTopology) {
      m_pImmediateContext->IASetPrimitiveTopology(eTopology);
      m_eCurTopology = eTopology;
    }
  }

} // namespace D3D11
#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)