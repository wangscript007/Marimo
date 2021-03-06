#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

// 全局头文件
#include "GrapX.h"
#include "GXApp.h"
#include "User/GrapX.Hxx"

// 标准接口
//#include "Include/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/MOLogger.h"

// 平台相关
#include "GrapX/Platform.h"
#include "Platform/Win32_XXX.h"
#include "Platform/Win32_D3D11.h"

// 私有头文件
#include <User32Ex.h>
//#ifdef _ENABLE_STMT
//#include <clstdcode\stmt\stmt.h>
//#else
//#include <clMessageThread.h>
//#endif // #ifdef _ENABLE_STMT
#include <GrapX/gxDevice.h>
#include "Canvas/GXResourceMgr.h"
#include "Platform/CommonBase/GXGraphicsBaseImpl.h"
#include "Platform/Win32_D3D11/GXGraphicsImpl_d3d11.h"
#include "GrapX/GXUser.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DXGI.lib")
//#pragma comment(lib, "d3dx11d.lib")
#ifdef ENABLE_GRAPHICS_API_DX11
//////////////////////////////////////////////////////////////////////////

IGXPlatform_Win32D3D11::IGXPlatform_Win32D3D11()
{
  m_pApp = NULL;
}

GXHRESULT IGXPlatform_Win32D3D11::Initialize(GXApp* pApp, GXAPP_DESC* pDesc, GrapX::Graphics** ppGraphics)
{
  const static LPWSTR lpClassName = _T("GrapX_Win32_D3D11_Class");
  //WNDCLASSEX wcex;

  m_pApp = pApp;
  GrapX::Graphics* pGraphics = NULL;

  if(CreateWnd(lpClassName, WndProc, pDesc, pApp) != 0) {
    return GX_FAIL;
  }


  //m_hInstance        = GetModuleHandle(NULL);
  //wcex.cbSize        = sizeof(WNDCLASSEX);
  //wcex.style         = GXCS_HREDRAW | GXCS_VREDRAW;
  //wcex.lpfnWndProc   = WndProc;
  //wcex.cbClsExtra    = 0;
  //wcex.cbWndExtra    = sizeof(GXApp*);
  //wcex.hInstance     = m_hInstance;
  //wcex.hIcon         = NULL;
  //wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  //wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  //wcex.lpszMenuName  = NULL;
  //wcex.lpszClassName = lpClassName;
  //wcex.hIconSm       = NULL;

  //if(RegisterClassEx(&wcex) == 0)
  //{
  //  return GX_FAIL;
  //}

  //if(TEST_FLAG(pDesc->dwStyle, GXADS_SIZABLE) && pDesc->nWidth == 0)
  //{
  //  GXREGN regnNewWin;
  //  // 可调整窗口
  //  // 根据宽度决定是默认尺寸还是用户指定尺寸
  //  //if(pDesc->nWidth == 0) {
  //    gxSetRegn(&regnNewWin, GXCW_USEDEFAULT, 0, GXCW_USEDEFAULT, 0);
  //  //}
  //  //else {
  //  //  RECT rcWorkArea;
  //  //  SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcWorkArea, NULL);
  //  //  gxSetRegn(&regnNewWin, 
  //  //    rcWorkArea.left + (rcWorkArea.right - rcWorkArea.left - pDesc->nWidth) / 2, 
  //  //    rcWorkArea.top + (rcWorkArea.bottom - rcWorkArea.top - pDesc->nHeight) / 2, pDesc->nWidth, pDesc->nHeight);
  //  //}
  //  m_hWnd = CreateWindowEx(
  //    NULL, lpClassName, pDesc->lpName, WS_OVERLAPPEDWINDOW,
  //    regnNewWin.left, regnNewWin.top, regnNewWin.width, regnNewWin.height, NULL, NULL, 
  //    m_hInstance, NULL);
  //}
  //else
  //{
  //  m_hWnd = CreateWindowEx(
  //    NULL, lpClassName, pDesc->lpName, WS_CAPTION | WS_SYSMENU,
  //    0, 0, 100, 100, NULL, NULL, 
  //    m_hInstance, NULL);
  //  RECT rectWorkarea;
  //  SystemParametersInfo(SPI_GETWORKAREA, 0, &rectWorkarea, 0);
  //  SetClientSize(m_hWnd, NULL, 
  //    ((rectWorkarea.right - rectWorkarea.left) - pDesc->nWidth) / 2 + rectWorkarea.left,
  //    ((rectWorkarea.bottom - rectWorkarea.top) - pDesc->nHeight) / 2 + rectWorkarea.top,
  //    pDesc->nWidth, pDesc->nHeight, NULL);
  //}
  //SetWindowLong(m_hWnd, 0, (GXLONG)pApp);

  //if (m_hWnd == NULL)
  //{
  //  return GX_FAIL;
  //}
  GrapX::D3D11::GRAPHICS_CREATION_DESC sDesc = {0};
  sDesc.hWnd          = m_hWnd;
  sDesc.bWaitForVSync = pDesc->dwStyle & GXADS_WAITFORVSYNC;
  sDesc.szRootDir     = m_strRootDir;
  sDesc.pLogger       = pDesc->pLogger;
  //sDesc.dwCreateFlags      = CheckBoolean();
  if(pDesc->pParameter)
  {
    for(int pidx = 0; pDesc->pParameter[pidx].szName != NULL; pidx++)
    {
      if(clstd::strcmpT(pDesc->pParameter[pidx].szName, "GraphicsEnvSet/Debug") == 0) {
        if(clstd::CheckBoolean(pDesc->pParameter[pidx].szValue, clstd::strlenT(pDesc->pParameter[pidx].szValue))) {
          sDesc.dwCreateFlags |= GRAPHICS_CREATION_FLAG_DEBUG;
        }
      }
    }
  }
  
  pGraphics = GrapX::D3D11::GraphicsImpl::Create(&sDesc);

  *ppGraphics = pGraphics;
  m_pLogger = pDesc->pLogger;
  if(m_pLogger) {
    m_pLogger->AddRef();
  }
  
  GXCREATESTATION stCrateStation;
  stCrateStation.cbSize = sizeof(GXCREATESTATION);
  stCrateStation.hWnd = m_hWnd;
  stCrateStation.lpPlatform = this;
  stCrateStation.lpAppDesc = pDesc;

  GXUICreateStation(&stCrateStation);

  //pGraphics->Activate(TRUE);  // 开始捕获Graphics状态
  //m_pApp->OnCreate();
  //pGraphics->Activate(FALSE);

//#ifndef _DEV_DISABLE_UI_CODE
#ifdef _ENABLE_STMT
  STMT::CreateTask(1024 * 1024, UITask, NULL);
#else
  
  //GXSTATION* pStation = IntGetStationPtr();
  //pStation->m_pMsgThread = new GXUIMsgThread(this);
  //pStation->m_pMsgThread->Start();

    //static_cast<MessageThread*>(MessageThread::CreateThread((CLTHREADCALLBACK)UITask, this));
#endif // #ifdef _ENABLE_STMT
//#endif // _DEV_DISABLE_UI_CODE

  // 这个必须放在最后, 所有初始化完毕, 刷新窗口
  ShowWindow(m_hWnd, GXSW_SHOWDEFAULT);
  UpdateWindow(m_hWnd);

  return GX_OK;
}
GXHRESULT IGXPlatform_Win32D3D11::Finalize(GXINOUT GrapX::Graphics** ppGraphics)
{
  GXUIDestroyStation();

  SAFE_RELEASE(*ppGraphics);
  return IMOPlatform_Win32Base::Finalize(ppGraphics);
}

GXPlatformIdentity IGXPlatform_Win32D3D11::GetPlatformID() const
{
  return GXPLATFORM_WIN32_DIRECT3D9;
}

//GXDWORD GXCALLBACK IGXPlatform_Win32D3D11::UITask(GXLPVOID lParam)
//{
//#ifndef _DEV_DISABLE_UI_CODE
//  CLMTCREATESTRUCT* pCreateParam = (CLMTCREATESTRUCT*)lParam;
//  IGXPlatform_Win32D3D11* pPlatform = (IGXPlatform_Win32D3D11*)pCreateParam->pUserParam;
//  GXApp* pGXApp = (GXApp*)pPlatform->m_pApp;
//  
//  GXGraphics* pGraphics = pGXApp->GetGraphicsUnsafe();
//  pGraphics->Activate(TRUE);  // 开始捕获Graphics状态
//  GXHRESULT hval = pGXApp->OnCreate();
//  pGraphics->Activate(FALSE);
//
//  if(GXFAILED(hval)) {
//    return hval;
//  }
//
//  GXMSG gxmsg;
//  while(1)
//  {
//    gxGetMessage(&gxmsg, NULL);
//    gxDispatchMessageW(&gxmsg);
//    pPlatform->AppHandle(gxmsg.message, gxmsg.wParam, gxmsg.lParam);
//
//    if(gxmsg.message == GXWM_QUIT)
//      break;
//  }
//#endif // _DEV_DISABLE_UI_CODE
//
//  pGXApp->OnDestroy();
//  return NULL;
//}

GXLPCWSTR IGXPlatform_Win32D3D11::GetRootDir()
{
  return m_strRootDir;
}

//////////////////////////////////////////////////////////////////////////

namespace GrapXToDX11
{
              
//GXFMT_R8G8B8               
//GXFMT_A8R8G8B8             
//GXFMT_X8R8G8B8             
//GXFMT_R5G6B5               
//GXFMT_X1R5G5B5             
//GXFMT_A1R5G5B5             
//GXFMT_A4R4G4B4             
//GXFMT_R3G3B2               
//GXFMT_A8                   
//GXFMT_A8R3G3B2             
//GXFMT_X4R4G4B4             
//GXFMT_A2B10G10R10          
//GXFMT_A8B8G8R8             
//GXFMT_X8B8G8R8             
//GXFMT_G16R16               
//GXFMT_A2R10G10B10          
//GXFMT_A16B16G16R16         
//GXFMT_A8P8                 
//GXFMT_P8                   
//GXFMT_L8                   
//GXFMT_A8L8                 
//GXFMT_A4L4                 
//GXFMT_V8U8                 
//GXFMT_L6V5U5               
//GXFMT_X8L8V8U8             
//GXFMT_Q8W8V8U8             
//GXFMT_V16U16               
//GXFMT_A2W10V10U10          
//GXFMT_UYVY                 
//GXFMT_R8G8_B8G8            
//GXFMT_YUY2                 
//GXFMT_G8R8_G8B8            
//GXFMT_DXT1                 
//GXFMT_DXT2                 
//GXFMT_DXT3                 
//GXFMT_DXT4                 
//GXFMT_DXT5                 
//GXFMT_D16_LOCKABLE         
//GXFMT_D32                  
//GXFMT_D15S1                
//GXFMT_D24S8                
//GXFMT_D24X8                
//GXFMT_D24X4S4              
//GXFMT_D16                  
//GXFMT_D32F_LOCKABLE        
//GXFMT_D24FS8               
//GXFMT_D32_LOCKABLE         
//GXFMT_S8_LOCKABLE          
//GXFMT_L16                  
//GXFMT_VERTEXDATA           
//GXFMT_INDEX16              
//GXFMT_INDEX32              
//GXFMT_Q16W16V16U16         
//GXFMT_MULTI2_ARGB8
//GXFMT_R16F                 
//GXFMT_G16R16F              
//GXFMT_A16B16G16R16F        
//GXFMT_R32F                 
//GXFMT_G32R32F              
//GXFMT_A32B32G32R32F        
//GXFMT_CxV8U8               
//GXFMT_A1                   
//GXFMT_A2B10G10R10_XR_BIAS  
//GXFMT_BINARYBUFFER        

#define FORMAT_PARIS \
  DEF_FORMAT_PAIR(Format_R8G8B8A8, DXGI_FORMAT_R8G8B8A8_UNORM);                        \
  DEF_FORMAT_PAIR(Format_R8G8, DXGI_FORMAT_R8G8_UNORM);                                \
  DEF_FORMAT_PAIR(Format_B8G8R8A8, DXGI_FORMAT_B8G8R8A8_UNORM);                        \
  DEF_FORMAT_PAIR(Format_B8G8R8X8, DXGI_FORMAT_B8G8R8X8_UNORM);                        \
  DEF_FORMAT_PAIR(Format_D32, DXGI_FORMAT_D32_FLOAT);                                  \
  DEF_FORMAT_PAIR(Format_D16, DXGI_FORMAT_D16_UNORM);                                  \
  DEF_FORMAT_PAIR(Format_D24S8, DXGI_FORMAT_D24_UNORM_S8_UINT);                        \
  /*DEF_FORMAT_PAIR(Format_B8G8R8, DXGI_FORMAT_B8G8R8X8_TYPELESS);*/                   \
  DEF_FORMAT_PAIR(Format_R16G16, DXGI_FORMAT_R16G16_FLOAT);                            \
  DEF_FORMAT_PAIR(Format_R8, DXGI_FORMAT_R8_UNORM);                                    \
  DEF_FORMAT_PAIR(Format_BC1, DXGI_FORMAT_BC1_UNORM);                                  \
  DEF_FORMAT_PAIR(Format_BC2, DXGI_FORMAT_BC2_UNORM);                                  \
  DEF_FORMAT_PAIR(Format_BC3, DXGI_FORMAT_BC3_UNORM);                                  \
  DEF_FORMAT_PAIR(Format_R16, DXGI_FORMAT_R16_FLOAT);                                  \
  DEF_FORMAT_PAIR(Format_R32, DXGI_FORMAT_R32_FLOAT);                                  \
  DEF_FORMAT_PAIR(Format_A8, DXGI_FORMAT_A8_UNORM);                                    \
  DEF_FORMAT_PAIR(Format_R32G32B32A32_Float, DXGI_FORMAT_R32G32B32A32_FLOAT);          \
  DEF_FORMAT_PAIR(Format_R32G32B32_Float, DXGI_FORMAT_R32G32B32_FLOAT);                \


#define DEF_FORMAT_PAIR(_FMTA, _FMTB) case _FMTA: return _FMTB

  DXGI_FORMAT FormatFrom(GXFormat eFormat)
  {
    switch (eFormat)
    {
      DEF_FORMAT_PAIR(GXFMT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
      DEF_FORMAT_PAIR(GXFMT_A8R8G8B8, DXGI_FORMAT_B8G8R8A8_UNORM);
      DEF_FORMAT_PAIR(GXFMT_A8B8G8R8, DXGI_FORMAT_R8G8B8A8_UNORM);
      DEF_FORMAT_PAIR(GXFMT_A8, DXGI_FORMAT_A8_UNORM);
      DEF_FORMAT_PAIR(Format_D24X8, DXGI_FORMAT_D24_UNORM_S8_UINT);
      FORMAT_PARIS
    default:
      ASSERT(0);
    }
    return DXGI_FORMAT_UNKNOWN;
  }

#undef DEF_FORMAT_PAIR
#define DEF_FORMAT_PAIR(_FMTA, _FMTB) case _FMTB: return _FMTA

  GXFormat FormatFrom(DXGI_FORMAT eFormat)
  {
    switch (eFormat)
    {
      FORMAT_PARIS
    default:
      ASSERT(0);
    }
    return GXFormat::Format_Unknown;
  }


#if 0
  void PrimitiveDescFromResUsage(IN GXDWORD ResUsage, D3D11_BUFFER_DESC* pDesc)
  {
    if(TEST_FLAG(ResUsage, GXRU_TEST_READ)) {
      pDesc->Usage = D3D11_USAGE_STAGING;
      pDesc->CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
    }
    if(TEST_FLAG(ResUsage, GXRU_TEST_WRITE)) {
      pDesc->Usage = D3D11_USAGE_DYNAMIC;
      pDesc->CPUAccessFlags |= (D3D11_CPU_ACCESS_WRITE);
    }
  }
#endif

  // https://docs.microsoft.com/en-us/windows/desktop/api/d3d11/ne-d3d11-d3d11_usage
  // D3D11_USAGE_DEFAULT	  A resource that requires read and write access by the GPU.This is likely to be the most common usage choice.
  // D3D11_USAGE_IMMUTABLE	A resource that can only be read by the GPU.It cannot be written by the GPU, and cannot be accessed at all by the CPU.This type of resource must be initialized when it is created, since it cannot be changed after creation.
  // D3D11_USAGE_DYNAMIC	  A resource that is accessible by both the GPU (read only) and the CPU (write only).A dynamic resource is a good choice
  //                        for a resource that will be updated by the CPU at least once per frame.To update a dynamic resource, use a Map method.
  //                        For info about how to use dynamic resources, see How to : Use dynamic resources.
  // D3D11_USAGE_STAGING	  A resource that supports data transfer (copy) from the GPU to the CPU.

  void PrimitiveDescFromResUsage(D3D11_BUFFER_DESC* pDesc, GXResUsage eResUsage)
  {
    switch(eResUsage)
    {
    case GXResUsage::Default:
      // 不再修改数据，创建时必须指定
      pDesc->Usage = D3D11_USAGE_IMMUTABLE;
      pDesc->CPUAccessFlags = 0;
      break;

    case GXResUsage::Read:
      // 可读，实现中保存数据副本
      pDesc->Usage = D3D11_USAGE_DEFAULT;
      pDesc->CPUAccessFlags = 0;
      break;

    case GXResUsage::Write:
      pDesc->Usage = D3D11_USAGE_DYNAMIC;
      pDesc->CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      break;

    case GXResUsage::ReadWrite:
      pDesc->Usage = D3D11_USAGE_DYNAMIC;
      pDesc->CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      break;

    case GXResUsage::SystemMem:
      // 始终在CPU内存中，不用于渲染
      pDesc->Usage = D3D11_USAGE_DEFAULT;
      pDesc->CPUAccessFlags = 0;
      break;

    default:
      CLBREAK;
      break;
    }
  }

#if 0
  void TextureDescFromResUsage(IN GXDWORD ResUsage, D3D11_TEXTURE2D_DESC* pDesc)
  {
    if(TEST_FLAG(ResUsage, GXRU_SYSTEMMEM))
    {
      pDesc->BindFlags = 0;
      pDesc->CPUAccessFlags = D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE;
      //pDesc->Usage = D3D11_USAGE_STAGING;
      return;
    }
    
    if(TEST_FLAG(ResUsage, GXRU_TEX_RENDERTARGET))
    {
      pDesc->BindFlags |= D3D11_BIND_RENDER_TARGET;
    }

    if(TEST_FLAG(ResUsage, GXRU_TEST_READ)) {
      ASSERT( ! TEST_FLAG(ResUsage, GXRU_TEST_WRITE));
      pDesc->CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
      //pDesc->Usage = D3D11_USAGE_STAGING;
    }

    if(TEST_FLAG(ResUsage, GXRU_TEST_WRITE)) {
      ASSERT( ! TEST_FLAG(ResUsage, GXRU_TEST_READ));
      pDesc->CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      pDesc->Usage = D3D11_USAGE_DYNAMIC;
    }
  }
#endif

  void TextureDescFromResUsage(D3D11_TEXTURE2D_DESC* pDesc, GXResUsage eResUsage, GXUINT nMipLevels, GXBOOL bHasInitData)
  {
    switch(eResUsage)
    {
    case GXResUsage::Default:
      if(bHasInitData && nMipLevels != 0)
      {
        // 不再修改数据，创建时必须指定
        pDesc->Usage = D3D11_USAGE_IMMUTABLE;
        pDesc->CPUAccessFlags = 0;
      }
      else
      {
        pDesc->Usage = D3D11_USAGE_DEFAULT;
        pDesc->CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      }
      break;

    case GXResUsage::Read:
      // 可读，实现中保存数据副本
      pDesc->Usage = D3D11_USAGE_DEFAULT;
      pDesc->CPUAccessFlags = 0;
      break;

    case GXResUsage::Write:
      pDesc->Usage = D3D11_USAGE_DYNAMIC;
      pDesc->CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      break;

    case GXResUsage::ReadWrite:
      pDesc->Usage = D3D11_USAGE_DYNAMIC;
      pDesc->CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      break;

    case GXResUsage::SystemMem:
      // 始终在CPU内存中，不用于渲染
      pDesc->Usage = D3D11_USAGE_DEFAULT;
      pDesc->CPUAccessFlags = 0;
      break;

    case 10:
      break;

    default:
      CLBREAK;
      break;
    }
    if(nMipLevels == 0 && 
      _CL_NOT_(pDesc->Format >= DXGI_FORMAT_BC1_TYPELESS && pDesc->Format <= DXGI_FORMAT_BC5_SNORM) &&
      _CL_NOT_(pDesc->Format >= DXGI_FORMAT_BC6H_TYPELESS && pDesc->Format <= DXGI_FORMAT_BC7_UNORM_SRGB))
    {
      pDesc->MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
      pDesc->BindFlags |= (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
    }
  }

  D3D11_MAP PrimitiveMapFromResUsage(IN GXDWORD ResUsage)
  {
    if(TEST_FLAG(ResUsage, GXRU_TEST_READ) && TEST_FLAG(ResUsage, GXRU_TEST_WRITE)) {
      return D3D11_MAP_READ_WRITE;
    }
    else if(TEST_FLAG(ResUsage, GXRU_TEST_READ)) {
      return D3D11_MAP_READ;
    }
    else if(TEST_FLAG(ResUsage, GXRU_TEST_WRITE)) {
      return D3D11_MAP_WRITE_DISCARD;
    }
    ASSERT(0);
    return (D3D11_MAP)0;
  }
  D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology(GXPrimitiveType eType, GXUINT nPrimCount, GXUINT* pVertCount)
  {
    switch(eType)
    {
    case GXPT_POINTLIST:
      *pVertCount = nPrimCount;
      return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

    case GXPT_LINELIST:
      *pVertCount = nPrimCount * 2;
      return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

    case GXPT_LINESTRIP:
      *pVertCount = nPrimCount + 1;
      return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

    case GXPT_TRIANGLELIST:
      *pVertCount = nPrimCount * 3;
      return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    case GXPT_TRIANGLESTRIP:
      *pVertCount = nPrimCount + 2;
      return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    //case GXPT_TRIANGLEFAN:    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN
    }
    ASSERT(0);
    return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
  }

  void VertexLayoutFromVertexDecl(LPCGXVERTEXELEMENT pVerticesDecl, GXD3D11InputElementDescArray* pArray)
  {
#define CASE_USAGE(USAGE) case GXDECLUSAGE_##USAGE: Desc.SemanticName = #USAGE; break;
    GXD3D11_INPUT_ELEMENT_DESC Desc;

    int nCount = GetVertexDeclLength<int>(pVerticesDecl);
    pArray->reserve(nCount + 1);

    for(int i = 0; i < nCount; i++)
    {
      Desc.SemanticIndex = pVerticesDecl[i].UsageIndex;
      switch(pVerticesDecl[i].Usage)
      {
      CASE_USAGE(POSITION);
      CASE_USAGE(BLENDWEIGHT);
      CASE_USAGE(BLENDINDICES);
      CASE_USAGE(NORMAL);
      //CASE_USAGE(PSIZE);
      CASE_USAGE(TEXCOORD);
      CASE_USAGE(TANGENT);
      CASE_USAGE(BINORMAL);
      //CASE_USAGE(TESSFACTOR);
      CASE_USAGE(POSITIONT);
      CASE_USAGE(COLOR);
      }
      switch(pVerticesDecl[i].Type)
      {
      case GXDECLTYPE_FLOAT1:   Desc.Format = DXGI_FORMAT_R32_FLOAT;          break;
      case GXDECLTYPE_FLOAT2:   Desc.Format = DXGI_FORMAT_R32G32_FLOAT;       break;
      case GXDECLTYPE_FLOAT3:   Desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;    break;
      case GXDECLTYPE_FLOAT4:   Desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
      case GXDECLTYPE_D3DCOLOR: Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  break;
      }

      Desc.InputSlot            = 0;
      Desc.AlignedByteOffset    = pVerticesDecl[i].Offset;
      Desc.InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
      Desc.InstanceDataStepRate = 0;

      pArray->push_back(Desc);
    }
    Desc.SemanticName         = "";
    Desc.SemanticIndex        = -1;
    Desc.Format               = DXGI_FORMAT_UNKNOWN;
    Desc.InputSlot            = -1;
    Desc.AlignedByteOffset    = -1;
    Desc.InputSlotClass       = (D3D11_INPUT_CLASSIFICATION)-1;
    Desc.InstanceDataStepRate = -1;
    pArray->push_back(Desc);
  }
  
  D3D11_FILTER FilterFrom(GXTextureFilterType eMag, GXTextureFilterType eMin, GXTextureFilterType eMip)
  {
    if(eMip == 0) {
      eMip = GXTEXFILTER_POINT;
    }
    ASSERT((eMag == 1 || eMag == 2) && (eMin == 1 || eMin == 2) && (eMip == 1 || eMip == 2));
    return D3D11_ENCODE_BASIC_FILTER(eMin - 1, eMag - 1, eMip - 1, 0);
  }

} // namespace GrapXToDX11

IGXPlatform_Win32D3D11* AppCreateD3D11Platform(GXApp* pApp, GXAPP_DESC* pDesc, GrapX::Graphics** ppGraphics)
{
  return AppCreatePlatformT<IGXPlatform_Win32D3D11>(pApp, pDesc, ppGraphics);
}
//////////////////////////////////////////////////////////////////////////

STATIC_ASSERT(GXTADDRESS_WRAP       == D3D11_TEXTURE_ADDRESS_WRAP);
STATIC_ASSERT(GXTADDRESS_MIRROR     == D3D11_TEXTURE_ADDRESS_MIRROR);
STATIC_ASSERT(GXTADDRESS_CLAMP      == D3D11_TEXTURE_ADDRESS_CLAMP);
STATIC_ASSERT(GXTADDRESS_BORDER     == D3D11_TEXTURE_ADDRESS_BORDER);
STATIC_ASSERT(GXTADDRESS_MIRRORONCE == D3D11_TEXTURE_ADDRESS_MIRROR_ONCE);

#endif // #ifdef ENABLE_GRAPHICS_API_DX11
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
