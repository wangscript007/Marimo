﻿#ifndef _GRAPVR_SEQUENCE_H_
#define _GRAPVR_SEQUENCE_H_

#define ENABLE_MULTIMAP_RENDERING_SORTING

class GXDLL GVSequence : public GUnknown
{
public:
#ifdef ENABLE_MULTIMAP_RENDERING_SORTING
  typedef clmultimap<GXUINT, GVRENDERDESC2*> RenderDescArray;
#else
  typedef clvector<GVRENDERDESC2*> RenderDescArray; // 虽然可以，但是渲染顺序是错的
#endif

  typedef clmap<GXUINT, RenderDescArray> RenderQueue;
protected:
  const static int c_nNumRQSlot = 4096;

  //RenderDescArray   m_aRenderDesc;
  //RenderDescArray   m_aRenderDesc2; // 这个需要排序
  RenderQueue       m_mapRenderQueue;
  int               m_nRenderCate = 0;

  // 这个是临时的，目前仅分了5级渲染顺序，还没想好怎么设计渲染序列这个问题， 实际上应该是4096级，并且材质排序的
  const static int c_nNeedRefactorRenderDescCount = 6;
  RenderDescArray   m_aRenderDesc[6];
  // </Comment>

//void*             m_aRenderQueueSlot[c_nNumRQSlot]; // void* 是临时的,没想好是啥类型

protected:
  GVSequence();
  virtual ~GVSequence();
public:
  void  Clear (int nRenderCate);
  int   Add   (GVRENDERDESC2* pDesc);

  int   GetArrayCount ();
  int   GetRenderCategory() const;

  const RenderDescArray&
        GetArray      (int nIndex);
public:
#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef  ();
  virtual GXHRESULT Release ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  static  GXHRESULT Create  (GVSequence** ppSequence);
};

#endif // _GRAPVR_SEQUENCE_H_