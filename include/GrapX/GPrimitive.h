﻿#ifndef _GRAPHX_PRIMITIVE_H_
#define _GRAPHX_PRIMITIVE_H_

class GVertexDeclaration;

class GPrimitive : public GResource
{
public:
  GPrimitive(GXUINT nPriority, GXDWORD dwType) : GResource(nPriority, dwType){}
  GXSTDINTERFACE(GXHRESULT  AddRef  ());

  GXSTDINTERFACE(GXGraphics*GetGraphicsUnsafe   ());

  GXSTDINTERFACE(GXUINT     GetVertexCount      ());
  GXSTDINTERFACE(GXUINT     GetVertexStride     ());
  GXSTDINTERFACE(GXLPVOID   MapVertexBuffer     (GXResMap eMap));
  GXSTDINTERFACE(GXBOOL     UnmapVertexBuffer   (GXLPVOID lpMappedBuffer));

  // 如果不包含Index buffer, 则下列索引相关函数返回0或者Null
  GXSTDINTERFACE(GXUINT     GetIndexCount       ());
  GXSTDINTERFACE(GXUINT     GetIndexStride      ());
  GXSTDINTERFACE(GXLPVOID   MapIndexBuffer      (GXResMap eMap));
  GXSTDINTERFACE(GXBOOL     UnmapIndexBuffer    (GXLPVOID lpMappedBuffer));

  //GXSTDINTERFACE(GXBOOL     UpdateResouce     (ResEnum eRes));  // 将内存数据更新到设备上
  GXSTDINTERFACE(GXHRESULT  GetVertexDeclaration(GVertexDeclaration** ppDeclaration));
  GXSTDINTERFACE(GXINT      GetElementOffset    (GXDeclUsage Usage, GXUINT UsageIndex, LPGXVERTEXELEMENT lpDesc = NULL));
};

namespace GrapX
{
  namespace PrimitiveUtility
  {
    // 锁定顶点的辅助对象，超出作用域自动解锁
    class GXDLL MapVertices
    {
    protected:
      GPrimitive* m_pPrimitive;
      GXLPVOID    m_ptr;

    public:
      MapVertices(GPrimitive* pPrimitive, GXResMap eMap);
      ~MapVertices();
      GXLPVOID GetPtr() const;
    };


    // 锁定顶点索引，超出作用域自动解锁
    class GXDLL MapIndices
    {
    protected:
      GPrimitive* m_pPrimitive;
      GXLPVOID    m_ptr;

    public:
      MapIndices(GPrimitive* pPrimitive, GXResMap eMap);
      ~MapIndices();
      GXLPVOID GetPtr() const;
    };

  } // namespace PrimitiveUtility
} // namespace GrapX

#endif // _GRAPHX_PRIMITIVE_H_