#ifdef ENABLE_GRAPHICS_API_DX11
#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#ifndef _GRAPH_X_CANVAS_D3D11_IMPLEMENT_H_
#define _GRAPH_X_CANVAS_D3D11_IMPLEMENT_H_

#define CANVAS_SHARED_SHADER_REGCOUNT 64

class CKinematicGrid;

//extern GXRENDERSTATE s_OpaqueFinalBlend[];
//extern GXRENDERSTATE s_AlphaFinalBlend[];
//extern GXRENDERSTATE s_OpaquePreBlend[];
//extern GXRENDERSTATE s_AlphaPreBlend[];

extern "C" GXBOOL GXDLLAPI  gxSetRectEmpty  (GXLPRECT lprc);
namespace GrapX
{
  class EffectImpl;
  namespace D3D11
  {
    class GraphicsImpl;
    class TextureImpl;
    //class GRenderState;

#include "Canvas/GXCanvasCoreImpl.h"

    struct DEFAULT_EFFECT
    {
      Effect*       pEffect;
      MOVarMatrix4  transform;
      MOVarFloat4   color;
      MOVarFloat4   color_add;
    };

    struct RENDERSTATE
    {
      Effect* pEffect;
    };

    class CanvasImpl : public CanvasCoreImpl
    {
      friend class GraphicsImpl;
    public:
      CanvasImpl(GraphicsImpl* pGraphics, GXBOOL bStatic);
      virtual ~CanvasImpl();
      GXBOOL  Initialize(RenderTarget* pTarget, const REGN* pRegn);

      GXINT   UpdateStencil    (GRegion* pClipRegion);

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
      virtual GXHRESULT Release();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

      GXULONG  GetRef();

      struct PRIMITIVE : public CANVAS_PRMI_VERT
      {
        inline void SetTexcoord(const GXFLOAT _u, const GXFLOAT _v) { u = _u; v = _v; }
      };


    public:
      Graphics*   GetGraphicsUnsafe   () const override;
      GXBOOL      SetTransform        (const float4x4* matTransform) override;
      GXBOOL      GetTransform        (float4x4* matTransform) const override;
      GXBOOL      SetViewportOrg      (GXINT x, GXINT y, GXLPPOINT lpPoint) override;
      GXBOOL      GetViewportOrg      (GXLPPOINT lpPoint) const override;
      GXBOOL      Flush               () override;
      GXBOOL      SetSamplerState     (GXUINT Sampler, GXSAMPLERDESC* pDesc) override;
      GXBOOL      SetEffect           (Effect* pEffect) override;
      GXDWORD     SetParametersInfo   (CanvasParamInfo eAction, GXUINT uParam, GXLPVOID pParam) override;
      PenStyle    SetPenStyle         (PenStyle eStyle) override;

      GXBOOL      Clear               (GXCOLORREF crClear) override;

      GXBOOL      SetPixel            (GXINT xPos, GXINT yPos, GXCOLORREF crPixel) override;
      GXBOOL      DrawLine            (GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF crLine) override;

      inline GXBOOL       InlDrawRectangle    (GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF crRect);
      inline GXBOOL       InlFillRectangle    (GXINT left, GXINT top, GXINT right, GXINT bottom, GXCOLORREF crFill);

      GXBOOL      DrawRectangle       (GXINT x, GXINT y, GXINT w, GXINT h, GXCOLORREF crRect) override;
      GXBOOL      DrawRectangle       (GXLPCRECT lprc, GXCOLORREF crRect) override;
      GXBOOL      DrawRectangle       (GXLPCREGN lprg, GXCOLORREF crRect) override;

      GXBOOL      FillRectangle       (GXINT x, GXINT y, GXINT w, GXINT h, GXCOLORREF crFill) override;
      GXBOOL      FillRectangle       (GXLPCRECT lprc, GXCOLORREF crFill) override;
      GXBOOL      FillRectangle       (GXLPCREGN lprg, GXCOLORREF crFill) override;
      GXBOOL      InvertRect          (GXINT x, GXINT y, GXINT w, GXINT h) override;

      GXBOOL      ColorFillRegion     (GRegion* pRegion, GXCOLORREF crFill) override;

      GXBOOL      DrawUserPrimitive   (Texture*pTexture, GXLPVOID lpVertices, GXUINT uVertCount, GXWORD* pIndices, GXUINT uIdxCount) override;
      GXBOOL      DrawTexture         (Texture*pTexture, const GXREGN *rcDest) override;
      GXBOOL      DrawTexture         (Texture*pTexture, GXINT xPos, GXINT yPos, const GXREGN *rcSrc) override;
      GXBOOL      DrawTexture         (Texture*pTexture, const GXREGN *rcDest, const GXREGN *rcSrc) override;
      GXBOOL      DrawTexture         (Texture*pTexture, const GXREGN *rcDest, const GXREGN *rcSrc, RotateType eRotation) override;

      GXINT       DrawText           (GXFont* pFTFont, GXLPCSTR lpString, GXINT nCount, GXLPRECT lpRect, GXUINT uFormat, GXCOLORREF crText) override;
      GXINT       DrawText           (GXFont* pFTFont, GXLPCWSTR lpString, GXINT nCount, GXLPRECT lpRect, GXUINT uFormat, GXCOLORREF crText) override;
      GXBOOL      TextOut            (GXFont* pFTFont, GXINT nXStart, GXINT nYStart, GXLPCSTR lpString, GXINT cbString, GXCOLORREF crText) override;
      GXBOOL      TextOut            (GXFont* pFTFont, GXINT nXStart, GXINT nYStart, GXLPCWSTR lpString, GXINT cbString, GXCOLORREF crText) override;
      GXLONG      TabbedTextOut      (GXFont* pFTFont, GXINT x, GXINT y, GXLPCSTR lpString, GXINT nCount, GXINT nTabPositions, GXINT* lpTabStopPositions, GXCOLORREF crText) override;
      GXLONG      TabbedTextOut      (GXFont* pFTFont, GXINT x, GXINT y, GXLPCWSTR lpString, GXINT nCount, GXINT nTabPositions, GXINT* lpTabStopPositions, GXCOLORREF crText) override;


      GXINT       SetCompositingMode  (CompositingMode eMode) override;
      GXBOOL      SetRegion           (GRegion* pRegion, GXBOOL bAbsOrigin) override;
      GXBOOL      SetClipBox          (const GXLPRECT lpRect) override;
      GXINT       GetClipBox          (GXLPRECT lpRect) override;
      GXDWORD     GetStencilLevel     () override;

      GXBOOL      Scroll              (int dx, int dy, LPGXCRECT lprcScroll, LPGXCRECT lprcClip, GRegion** lpprgnUpdate, LPGXRECT lprcUpdate) override;

    private:
      enum CanvasFunc
      {
        CF_NoOperation,  // 空的指令

        CF_DrawFirst,  // 绘图相关的
        CF_Points,
        CF_LineList,
        CF_Triangle,
        CF_Textured,
        CF_Clear,
        CF_ClearStencil,
        //CF_Scroll,
        CF_DrawLast,

        CF_StateFirst,  // 状态相关的
        //CF_RenderState,
        CF_SetViewportOrg,
        CF_SetRegion,
        CF_SetClipBox,
        CF_ResetClipBox,
        CF_SetTextClip,
        CF_ResetTextClip,
        CF_CompositingMode,
        CF_Effect,
        CF_ColorAdditive,
        //CF_SetUniform1f,
        //CF_SetUniform2f,
        //CF_SetUniform3f,
        //CF_SetUniform4f,
        //CF_SetUniform4x4f,
        //CF_SetPixelSizeInv,
        CF_SetExtTexture,
        //CF_SetEffectConst,
        CF_SetSamplerState,
        CF_SetTransform,
        CF_StateLast,
      };

      struct BATCH
      {
        CanvasFunc  eFunc;
        GXUINT      Handle;
        union {
          struct // Path, Pixel, Triangle
          {
            GXUINT  uVertexCount;
            GXUINT  uIndexCount;
          };
          struct  // Canvas' origin
          {
            GXINT   x;
            GXINT   y;
            GXINT   z;
            GXINT   w;
          }PosI;
          struct
          {
            float x;
            float y;
            float z;
            float w;
          }PosF;
          struct  // Render state
          {
            GXUINT   nRenderStateCode;
            GXDWORD  dwStateValue;
          };
          struct // qita (这是拼音!)
          {
            GXDWORD   dwFlag;
            GXWPARAM  wParam;
            GXLPARAM  lParam;
          }comm;
        };
        inline void Set(CanvasFunc _eFunc, GXUINT _uVertexCount, GXUINT _uIndexCount, GXLPARAM _lParam);
        inline void Set2(CanvasFunc _eFunc, GXINT x, GXINT y);
        inline void SetFloat4(CanvasFunc _eFunc, float x, float y, float z, float w);
        inline void SetRenderState(GXUINT nCode, GXDWORD dwValue);
      };

      // CALLSTATE 是用来检测是否重复设置的, 这里面的值是在接口调用后立即改变的
      // 而对应类中的值是在Flush之后改变的。
      struct CALLSTATE
      {
        GXINT           xOrigin;
        GXINT           yOrigin;
        GXRECT          rcClip;       // m_rcClip
        CompositingMode eCompMode;
        GXDWORD         dwColorAdditive;
        RENDERSTATE     RenderState;  // 不增加引用
        //EffectImpl*     pEffectImpl;  // 不增加引用
        float4x4        matTransform;
        CALLSTATE()
          : xOrigin         (0)
          , yOrigin         (0)
          , eCompMode       (CM_SourceCopy)
          , dwColorAdditive (NULL)
          //, pEffectImpl     (NULL)
        {
          gxSetRectEmpty(&rcClip);
          RenderState.pEffect = NULL;
        }
      };
    private:
      GXBOOL    CommitState        ();

      inline GXBOOL _CanFillBatch       (GXUINT uVertCount, GXUINT uIndexCount);
      inline void _SetPrimitivePos      (GXUINT nIndex, const GXINT _x, const GXINT _y);
      GXUINT    PrepareBatch            (CanvasFunc eFunc, GXUINT uVertCount, GXUINT uIndexCount, GXLPARAM lParam);

      GXINT    TextOutDirect            (const INTMEASURESTRING* p, GXLPPOINT pptPosition);
      GXINT    LocalizeTabPos           (const INTMEASURESTRING* pMeasureStr, int nCurPos, int nDefaultTabWidth, int* pLastIndex);
      GXINT    MeasureStringWidth_SL    (const INTMEASURESTRING* pMeasureStr); // Single line
      GXINT    MeasureStringWidth_RN    (const INTMEASURESTRING* pMeasureStr, GXINT* pEnd); // 忘了，靠！
      GXINT    MeasureStringWidth_WB    (const INTMEASURESTRING* pMeasureStr, GXINT nWidthLimit, GXINT* pEnd); // Work Break
      GXINT    DrawText_SingleLine      (const INTMEASURESTRING* pMeasureStr, GXLPRECT lpRect);
      GXINT    DrawText_Normal          (const INTMEASURESTRING* pMeasureStr, GXLPRECT lpRect);
      GXINT    DrawText_WordBreak       (const INTMEASURESTRING* pMeasureStr, GXLPRECT lpRect);

      void    SetStencil                (GXDWORD dwStencil);
      void    IntUpdateClip             (const GXRECT& rcClip);
    public:
      //inline const GXCANVASCOMMCONST&   GetCommonConst() const;
      //inline clBuffer&                  GetUniformBuffer();
      //inline void                       GetConstBuffer(clBuffer** ppVertexBuffer, clBuffer** ppPixelBuffer);

    private:
      GXDWORD       m_bStatic : 1;    // 标志是否是 GXGraphics 中的静态成员, 是的话表示在 GXGraphics 创建时已经创建好

      GXINT         m_xAbsOrigin;     // 绝对原点位置，不受API影响
      GXINT         m_yAbsOrigin;
      GXRECT        m_rcAbsClip;      // Canvas 初始化的/最大的/系统的 裁剪区域, 其实这个Rect的left和top等于m_xAbsOrigin, m_yAbsOrigin

      GXINT         m_xOrigin;        // 原点位置，可以通过函数设置  // [貌似这些值可以省略]
      GXINT         m_yOrigin;        // [貌似这些值可以省略]
      GXRECT        m_rcClip;         // 对应 m_LastState.rcClip, 纹理的坐标空间, 在flush阶段，m_rcClip只能写入/写入后读取，不能只读取，因为m_rcClip不是上一条命令的结果

      GXDWORD         m_dwStencil;
      RenderTarget*   m_pTargetImage;
      GXDWORD         m_dwTexVertColor; // 输出纹理图元时的顶点颜色
      GXDWORD         m_dwColorAdditive;
      PenStyle        m_eStyle;

      GRegion*        m_pClipRegion;

      const GXUINT    s_uDefVertIndexSize;
      const GXUINT    s_uDefBatchSize;

      RasterizerStateImpl*   m_pRasterizerState;
      BlendStateImpl*        m_pBlendingState[2];// Alpha合成方式的状态, 0: 最终合成, 1: 预先合成到纹理
      BlendStateImpl*        m_pOpaqueState[2];  // 不透明方式的状态
      DepthStencilStateImpl* m_pCanvasStencil[2];  // Canvas 用的Stencil开关,[0]关闭模板测试, [1]开启模板测试

      Primitive*    m_pPrimitive;
      PRIMITIVE*    m_lpLockedVertex;
      GXUINT        m_uVertCount;
      GXUINT        m_uIndexCount;
      GXUINT        m_uVertIndexSize;
      GXWORD*       m_lpLockedIndex;
      Texture*      m_pWhiteTex;


      BATCH*      m_aBatch;
      GXUINT      m_uBatchCount;  // 计数 这个必须小于m_uBatchSize(不能等于)
      GXUINT      m_uBatchSize;   // 尺寸

      CALLSTATE    m_CallState;   // User Call State

      GXDWORD      m_dwTexSlot;  // 用来判断是否设置了扩展纹理的标志, 减少循环之用
      TextureImpl*  m_aTextureStage[GX_MAX_TEXTURE_STAGE];    // 第一个应该总为0

      DEFAULT_EFFECT m_CommonEffect;
      //GXCANVASCOMMCONST  m_CanvasCommConst;
      //clBuffer      m_UniformBuffer;  // 如果这个用了就不用下面两个
      //clBuffer      m_VertexConstBuffer;
      //clBuffer      m_PixelConstBuffer;
    };

  } // namespace D3D11
} // namespace GrapX

//////////////////////////////////////////////////////////////////////////
#endif // _GRAPH_X_CANVAS_D3D11_IMPLEMENT_H_
#endif // #if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)
#endif // #ifdef ENABLE_GRAPHICS_API_DX11