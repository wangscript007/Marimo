﻿GPrimImpl::GPrimImpl(Graphics* pGraphics)
  : m_pGraphicsImpl   ((GraphicsImpl*)pGraphics)
  , m_uVerticesBuffer   (NULL)
  , m_uElementSize    (0)
  , m_uElementCount   (0)
  , m_pLockedVertex   (NULL)
  , m_dwResUsage      (NULL)
  , m_pVertexDeclImpl (NULL)
  //, m_pVertices       (NULL)
{
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GPrimitiveVertexOnlyImpl::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

GPrimitiveVertexOnlyImpl::GPrimitiveVertexOnlyImpl(Graphics* pGraphics)
  : GPrimitiveV         ()
  , GPrimImpl         (pGraphics)
  //, m_uElementSize      (0)
  //, m_uElementCount     (0)
  //, m_pLockedVertex     (NULL)
  //, m_dwResUsage        (GXRU_DEFAULT)
  //, m_uVerticesBuffer   (0)
  //, m_pVertexDeclImpl   (NULL)
{
}

GPrimitiveVertexOnlyImpl::~GPrimitiveVertexOnlyImpl()
{
  m_pGraphicsImpl->UnregisterResource(this);
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXLRESULT GPrimitiveVertexOnlyImpl::Release()
{
  GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
  if(nRefCount == 0)
  {
    delete this;
    return GX_OK;
  }
  return nRefCount;
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

//GXLRESULT GPrimitiveVImpl::OnDeviceEvent(DeviceEvent eEvent)
//{
//  switch(eEvent)
//  {
//  case DE_LostDevice:
//    {
//      GLVERIFY(glDeleteBuffers(1, &m_uVerticesBuffer));
//      m_uVerticesBuffer = 0;
//      SAFE_RELEASE(m_pVertexDeclImpl);
//      return GX_OK;
//    }
//    break;
//  case DE_ResetDevice:
//    {
//
//      GLVERIFY(glGenBuffers(1, &m_uVerticesBuffer));
//      GLVERIFY(glBindBuffer(GL_ARRAY_BUFFER, m_uVerticesBuffer));
//      GLVERIFY(glBufferData(GL_ARRAY_BUFFER, m_uElementCount * m_uElementSize,
//        NULL, GL_STATIC_DRAW));
//
//      return GX_OK;
//    }
//    break;
//  case DE_ResizeDevice:
//    break;
//  }
//  return GX_FAIL;
//}

GXBOOL GPrimitiveVertexOnlyImpl::InitPrimitive(GXLPCVOID pVertInitData, GXUINT uElementCount, GXUINT uElementSize, LPCGXVERTEXELEMENT pVertexDecl, GXDWORD ResUsage)
{
  ASSERT(pVertInitData == NULL); // TODO: 稍后支持初始化数据
  //ASSERT(pVertexDecl == NULL);
  m_uElementCount = uElementCount;
  m_uElementSize  = uElementSize;
  m_dwResUsage    = ResUsage;

  // 创建顶点声明
  if(pVertexDecl != NULL)
  {
    GVertexDeclaration* pInterface;
    GXHRESULT hval = m_pGraphicsImpl->CreateVertexDeclaration(&pInterface, pVertexDecl);
    if(GXSUCCEEDED(hval))
    {
      m_pVertexDeclImpl = static_cast<VertexDeclImpl*>(pInterface);
      //SET_FLAG(m_dwFlag, GXSHADERCAP_VERTDECL);
    }
  }
  // </创建顶点声明>

  if(OnDeviceEvent(DE_ResetDevice) == GX_OK)
    return TRUE;
  return FALSE;
}

GXBOOL GPrimitiveVertexOnlyImpl::EnableDiscard(GXBOOL bDiscard)
{
  return FALSE;
}

GXBOOL GPrimitiveVertexOnlyImpl::IsDiscardable()
{
  return TRUE;
  //return (m_hVertex == NULL);
}

GXLPVOID GPrimitiveVertexOnlyImpl::Lock(GXUINT uElementOffsetToLock, GXUINT uElementCountToLock, GXDWORD dwFlags/* = (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE)*/)
{
  if(m_pLockedVertex != NULL)
    return m_pLockedVertex;

  m_uLockedOffset = (GLintptr)(uElementOffsetToLock * m_uElementSize);
  m_uLockedSize = (GLsizeiptr)(uElementOffsetToLock * m_uElementSize);

  if(IsDiscardable() != FALSE)
  {
    return m_pLockedVertex;
  }
  return NULL;
}

GXBOOL GPrimitiveVertexOnlyImpl::Unlock()
{
  if(m_uLockedSize == 0)
  {
    GLVERIFY(glBindBuffer(GL_ARRAY_BUFFER, m_uVerticesBuffer));
    GLVERIFY(glBufferData(
      GL_ARRAY_BUFFER, 
      m_uElementCount * m_uElementSize, 
      m_pLockedVertex, 
      GL_STATIC_DRAW));
  }
  else
  {
    ASSERT(0);  // TODO: 验证后去除
    GLVERIFY(glBindBuffer(GL_ARRAY_BUFFER, m_uVerticesBuffer));
    GLVERIFY(glBufferSubData(
      GL_ARRAY_BUFFER,
      m_uLockedOffset,
      m_uLockedSize, 
      (GXBYTE*)m_pLockedVertex + m_uLockedOffset));  // TODO: 可能是这样
  }

  if(IsDiscardable() != FALSE)
  {
    m_pLockedVertex = NULL;
  }
  return GX_OK;
}

Primitive::Type GPrimitiveVertexOnlyImpl::GetType()
{
  return VertexOnly;
}

GXLPVOID GPrimitiveVertexOnlyImpl::GetVerticesBuffer()
{
  ASSERT(0);
  return NULL;
}

GXUINT GPrimitiveVertexOnlyImpl::GetVerticesCount()
{
  return m_uElementCount;
}

GXUINT GPrimitiveVertexOnlyImpl::GetVertexStride()
{
  return m_uElementSize;
}

  GXHRESULT GPrimitiveVertexOnlyImpl::GetVertexDeclaration(GVertexDeclaration** ppDeclaration)
  {
    return InlGetSafeObjectT<GVertexDeclaration>(ppDeclaration, m_pVertexDecl);
  }

GXINT GPrimitiveVertexOnlyImpl::GetElementOffset(GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc)
{
  return m_pVertexDecl->GetElementOffset(Usage, UsageIndex, lpDesc);
}

//////////////////////////////////////////////////////////////////////////
GPrimitiveVertexIndexImpl::GPrimitiveVertexIndexImpl(Graphics* pGraphics)
  : GPrimitiveVI            (pOwner)
  , m_pGraphicsImpl         ((GraphicsImpl*)pGraphics)
  , m_uElementSize          (0)
  , m_uVertexCount          (0)
  , m_uIndexCount           (0)
  , m_pLockedVertex         (NULL)
  , m_pLockedIndex          (NULL)
  , m_dwResUsage            (GXRU_DEFAULT)
  , m_uVerticesBuffer       (0)
  , m_uIndicesBuffer        (0)
  , m_uLockedVerticesOffset (0)
  , m_uLockedVerticesSize   (0)
  , m_uLockedIndicesOffset  (0)
  , m_uLockedIndicesSize    (0)
  , m_pVertexDeclImpl       (NULL)
{
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXLRESULT GPrimitiveVertexIndexImpl::AddRef()
{
  return gxInterlockedIncrement(&m_nRefCount);
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

GXLRESULT GPrimitiveVertexIndexImpl::Release()
{
  GXLONG nRefCount = gxInterlockedDecrement(&m_nRefCount);
  ASSERT((m_uRefCount & 0x80000000) == 0);
  if(nRefCount == 0)
  {
    delete this;
    return GX_OK;
  }
  else if(m_uRefCount == 1)
  {
    return m_pGraphicsImpl->OldUnregisterResource(this);
  }
  return nRefCount;
}

GXLRESULT GPrimitiveVertexIndexImpl::OnDeviceEvent(DeviceEvent eEvent)
{
  switch(eEvent)
  {
  case DE_LostDevice:
    {
      GLVERIFY(glDeleteBuffers(1, &m_uVerticesBuffer));
      GLVERIFY(glDeleteBuffers(1, &m_uIndicesBuffer));
      m_uVerticesBuffer = 0;
      m_uIndicesBuffer = 0;
      SAFE_RELEASE(m_pVertexDeclImpl);
      return GX_OK;
    }
    break;
  case DE_ResetDevice:
    {
      GLVERIFY(glGenBuffers(1, &m_uVerticesBuffer));
      GLVERIFY(glBindBuffer(GL_ARRAY_BUFFER, m_uVerticesBuffer));
      GLVERIFY(glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * m_uElementSize,
        NULL, GL_STATIC_DRAW));

      GLVERIFY(glGenBuffers(1, &m_uIndicesBuffer));
      GLVERIFY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uIndicesBuffer));
      GLVERIFY(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_uIndexCount * sizeof(GXWORD),
        NULL, GL_STATIC_DRAW));

      return GX_OK;
    }
    break;
  case DE_ResizeDevice:
    break;
  }
  return GX_FAIL;
}

GXBOOL GPrimitiveVertexIndexImpl::InitPrimitive(GXLPCVOID pVertInitData, GXUINT uVertexCount, GXUINT uVertexSize, GXLPVOID pIndexInitData, GXUINT uIndexCount, LPCGXVERTEXELEMENT pVertexDecl, GXDWORD ResUsage)
{
  ASSERT(pIndexInitData == NULL && pVertInitData == NULL); // TODO: 稍后支持初始化数据
  //ASSERT(pVertexDecl == NULL);
  if(uVertexCount == 0 || uVertexSize == 0 || uIndexCount == 0)
    return FALSE;

  m_uVertexCount = uVertexCount;
  m_uElementSize  = uVertexSize;
  m_uIndexCount   = uIndexCount;
  m_dwResUsage    = ResUsage;
  //m_ePool      = ePool;

  // 创建顶点声明
  if(pVertexDecl != NULL)
  {
    GVertexDeclaration* pInterface;
    GXHRESULT hval = m_pGraphicsImpl->CreateVertexDeclaration(&pInterface, pVertexDecl);
    if(GXSUCCEEDED(hval))
    {
      m_pVertexDeclImpl = static_cast<VertexDeclImpl*>(pInterface);
      //SET_FLAG(m_dwFlag, GXSHADERCAP_VERTDECL);
    }
  }
  // </创建顶点声明>


  if(OnDeviceEvent(DE_ResetDevice) == GX_OK)
    return TRUE;
  return FALSE;
}

GXBOOL GPrimitiveVertexIndexImpl::EnableDiscard(GXBOOL bDiscard)
{
  return FALSE;
}

GXBOOL GPrimitiveVertexIndexImpl::IsDiscardable()
{
  return TRUE;
  //return (m_hVertex == NULL);
}

GXBOOL GPrimitiveVertexIndexImpl::Lock(GXUINT uElementOffsetToLock, GXUINT uElementCountToLock, GXUINT uIndexOffsetToLock, GXUINT uIndexLengthToLock,
  GXLPVOID* ppVertexData, GXWORD** ppIndexData, GXDWORD dwFlags/* = (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE)*/)
{
  if(ppVertexData == NULL || ppIndexData == NULL)
    return FALSE;
  if(m_pLockedVertex != NULL || m_pLockedIndex != NULL)
  {
    *ppVertexData = m_pLockedVertex;
    *ppIndexData  = m_pLockedIndex;

    return ((*ppIndexData) != NULL && (*ppVertexData) != NULL);
  }

  m_uLockedVerticesOffset = (GLintptr)(uElementOffsetToLock * m_uElementSize);
  m_uLockedVerticesSize   = (GLsizeiptr)(uElementCountToLock * m_uElementSize);
  m_uLockedIndicesOffset  = (GLintptr)(uIndexOffsetToLock * sizeof(GXWORD));
  m_uLockedIndicesSize    = (GLsizeiptr)(uIndexLengthToLock * sizeof(GXWORD));

  if(m_uLockedVerticesSize != 0)
    m_pLockedVertex = new GXBYTE[m_uLockedVerticesSize];
  else
    m_pLockedVertex = new GXBYTE[m_uVertexCount * m_uElementSize];

  if(m_uLockedIndicesSize != 0)
    m_pLockedIndex = new GXWORD[m_uLockedIndicesSize];
  else
    m_pLockedIndex = new GXWORD[m_uIndexCount * sizeof(GXWORD)];

  if(IsDiscardable() != FALSE)
  {
    *ppVertexData = m_pLockedVertex;
    *ppIndexData  = m_pLockedIndex;

    return ((*ppIndexData) != NULL && (*ppVertexData) != NULL);
  }
  return FALSE;
}


GXBOOL GPrimitiveVertexIndexImpl::Unlock()
{
  if(m_uLockedVerticesSize == 0)
  {
    GLVERIFY(glBindBuffer(GL_ARRAY_BUFFER, m_uVerticesBuffer));

    GLVERIFY(glBufferData(
      GL_ARRAY_BUFFER, 
      m_uVertexCount * m_uElementSize, 
      m_pLockedVertex, 
      GL_STATIC_DRAW));
  }
  else
  {
    ASSERT(0);  // TODO: 验证后去除
    GLVERIFY(glBindBuffer(GL_ARRAY_BUFFER, m_uVerticesBuffer));

    GLVERIFY(glBufferSubData(
      GL_ARRAY_BUFFER,
      m_uLockedVerticesOffset,
      m_uLockedVerticesSize, 
      m_pLockedVertex));  // TODO: 可能是这样
  }

  if(m_uLockedIndicesSize == 0)
  {
    GLVERIFY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uIndicesBuffer));

    GLVERIFY(glBufferData(
      GL_ELEMENT_ARRAY_BUFFER, 
      m_uIndexCount * sizeof(GXWORD), 
      m_pLockedIndex, 
      GL_STATIC_DRAW));
  }
  else
  {
    ASSERT(0);  // TODO: 验证后去除
    GLVERIFY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uIndicesBuffer));

    GLVERIFY(glBufferSubData(
      GL_ELEMENT_ARRAY_BUFFER,
      m_uLockedIndicesOffset,
      m_uLockedIndicesSize, 
      m_pLockedIndex));  // TODO: 可能是这样
  }

  if(IsDiscardable() != FALSE)
  {
    if(m_pLockedVertex != NULL)
    {
      delete m_pLockedVertex;
      m_pLockedVertex = NULL;
    }
    if(m_pLockedIndex != NULL)
    {    
      delete m_pLockedIndex;
      m_pLockedIndex  = NULL;
    }
  }
  return GX_OK;
}

Primitive::Type GPrimitiveVertexIndexImpl::GetType()
{
  return Indexed;
}

GXLPVOID GPrimitiveVertexIndexImpl::GetVerticesBuffer()
{
  ASSERT(0);
  return NULL;
}

GXLPVOID GPrimitiveVertexIndexImpl::GetIndicesBuffer()
{
  ASSERT(0);
  return NULL;
}

  GXHRESULT GPrimitiveVertexIndexImpl::GetVertexDeclaration(GVertexDeclaration** ppDeclaration)
  {
    return InlGetSafeObjectT<GVertexDeclaration>(ppDeclaration, m_pVertexDecl);
  }


GXUINT GPrimitiveVertexIndexImpl::GetVerticesCount()
{
  return m_uVertexCount;
}

GXUINT GPrimitiveVertexIndexImpl::GetVertexStride()
{
  return m_uElementSize;
}

GXINT GPrimitiveVertexIndexImpl::GetElementOffset(GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc)
{
  return m_pVertexDecl->GetElementOffset(Usage, UsageIndex, lpDesc);
}
