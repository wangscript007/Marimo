#if defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)

#ifndef _SHADER_CLASS_D3D9_IMPLEMENT_HEADER_DEFINE_FILE_
#define _SHADER_CLASS_D3D9_IMPLEMENT_HEADER_DEFINE_FILE_

#define GET_VSREGISTER_IDX(_HANDLE) (GXUINT)GXLOWORD(_HANDLE)
#define GET_PSREGISTER_IDX(_HANDLE)  (GXUINT)GXHIWORD(_HANDLE)

struct STANDARDMTLUNIFORMTABLE;

namespace D3D9
{
  class GXGraphicsImpl;
  //class IHLSLInclude;

  struct GXD3DXCONSTDESC : D3DXCONSTANT_DESC
  {
    GXDWORD       dwNameID;
    GXDWORD       dwHandle;  // ��ʵ������clvector�д�1��ʼ������,Pixel������������16λ
    GXINT_PTR     nCanvasUniform;
  };

  typedef GXD3DXCONSTDESC*        GXLPD3DXCONSTDESC;
  typedef const GXD3DXCONSTDESC*  GXLPCD3DXCONSTDESC;

  class GXCanvasImpl;
  class GShaderImpl : public GShader
  {
    friend class GXGraphicsImpl;
  public:
    typedef clvector<GXD3DXCONSTDESC>  ConstantDescArray;
    typedef const ConstantDescArray    ConstDescArray;

    enum CompiledType // ���Ҫ��DX9,DX10,DX11��Ķ���һ��
    {
      CompiledVertexShder,
      CompiledPixelShder,
      CompiledComponentVertexShder,
      CompiledComponentPixelShder,
    };

  protected:
    GXGraphicsImpl*  m_pGraphicsImpl;
    GXDWORD          m_dwFlag;
    GXINT            m_cbPixelTopIndex;
    GXINT            m_cbCacheSize;
    GXHRESULT        CleanUp              ();
    GXINT            UpdateConstTabDesc   (LPD3DXCONSTANTTABLE pct, LPD3DXCONSTANTTABLE_DESC pctd, GXUINT uHandleShift);

    GShaderImpl(GXGraphics* pGraphics);
    virtual  ~GShaderImpl();
  public:
    LPDIRECT3DVERTEXSHADER9       m_pVertexShader;
    LPDIRECT3DPIXELSHADER9        m_pPixelShader;
    LPD3DXCONSTANTTABLE           m_pvct;
    LPD3DXCONSTANTTABLE           m_ppct;
    clStringW                     m_strProfileDesc; // Shader �����ļ���+����

    ConstantDescArray             m_aConstDesc;
    D3DXCONSTANTTABLE_DESC        m_VertexShaderConstTabDesc;
    D3DXCONSTANTTABLE_DESC        m_PixelShaderConstTabDesc;

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT   AddRef            ();
    virtual GXHRESULT   Release           ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
    virtual GXHRESULT   Invoke            (GRESCRIPTDESC* pDesc);
    virtual GXHRESULT   LoadFromFile      (MOSHADER_ELEMENT_SOURCE* pSdrElementSrc);
    virtual GXHRESULT   LoadFromMemory    (const clBufferBase* pVertexBuf, const clBufferBase* pPixelBuf);
    virtual GXGraphics* GetGraphicsUnsafe () GXCONST;
    virtual GXLPCWSTR   GetProfileDesc    () GXCONST;
    static  GXHRESULT   CompileShader     (clBuffer* pBuffer, LPD3DXINCLUDE pInclude, GXDEFINITION* pMacros, CompiledType eCompiled); // �����buffer���������ƴ����滻

  public:
    GXHRESULT       Activate            ();
    ConstDescArray& GetConstantDescTable() GXCONST;
    GXINT           GetCacheSize        () GXCONST;
    inline GXINT    GetPixelIndexOffset () GXCONST;
    GXUINT          GetHandle           (GXLPCSTR pName) GXCONST;
    GXUniformType   GetHandleType       (GXUINT handle) GXCONST;
    GXUINT          GetStageByHandle    (GXUINT handle) GXCONST;
#ifdef REFACTOR_SHADER
    GXBOOL          CommitToDevice      (GXLPVOID lpUniform, GXUINT cbSize);
#endif // #ifdef REFACTOR_SHADER
  };
  //////////////////////////////////////////////////////////////////////////
  inline GXINT GShaderImpl::GetCacheSize() GXCONST
  {
    return m_cbCacheSize;
  }
  inline GXINT GShaderImpl::GetPixelIndexOffset() GXCONST
  {
    return (m_cbPixelTopIndex >> 2) >> 2;
  }
  //////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////
} // namespace D3D9

#endif // _SHADER_CLASS_D3D9_IMPLEMENT_HEADER_DEFINE_FILE_
#endif // defined(_WIN32_XXX) || defined(_WIN32) || defined(_WINDOWS)