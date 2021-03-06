// 全局头文件
#include <GrapX.h>
#include "thread/clMessageThread.h"
#include <User/GrapX.Hxx>

// 标准接口
#include "GrapX/GResource.h"
#include "GrapX/GPrimitive.h"
#include "GrapX/GXGraphics.h"
#include "GrapX/GShader.h"

// 私有头文件
#include "clTree.h"
#include "clTransform.h"
#include "smart/SmartRepository.h"

#include "GrapX/gvNode.h"
#include "GrapX/gvScene.h"
#include "GrapX/gvMesh.h"
#include "GrapX/gvSkeleton.h"
#include "GrapX/gvSkinnedMesh.h"
#include "GrapX/gvScene.h"
#include "GrapX/gxUtility.h"

// 文件储存用的Keys
#define SKINNEDMESH_NAME          "SkMesh@Name"
#define SKINNEDMESH_MTLINST       "SkMesh@MtlInstW"
//#define SKINNEDMESH_VERTEXDECL    "SkMesh@VertexDeclaration"
//#define SKINNEDMESH_ASMVERTICES   "SkMesh@AssembledVertices"
//#define SKINNEDMESH_INDICES       "SkMesh@Indices"
//#define SKINNEDMESH_PRIMCOUNT     "SkMesh@PrimitiveCount"
//#define SKINNEDMESH_STARTINDEX    "SkMesh@StartIndex"
#define SKINNEDMESH_TRANSFORM     "SkMesh@Transform"
#define SKINNEDMESH_WEIGHT        "SkMesh@Weight"
#define SKINNEDMESH_CLUSTERCOUNT  "SkMesh@ClusterCount"

using namespace GrapX;

GXBOOL GVSkinnedMeshSoft::Initialize(
  GrapX::Graphics*    pGraphics,
  GXSIZE_T            nPrimCount, 
  GXLPCVERTEXELEMENT  lpVertDecl, 
  GXLPVOID            lpVertics, 
  GXSIZE_T            nVertCount, 
  GXWORD*             pIndices, 
  GXSIZE_T            nIdxCount, 
  GVSkeleton*         pSkeleton, 
  float*              pWeight, 
  GXSIZE_T            nClusterCount)
{
  if(IntCreatePrimitive(pGraphics, nPrimCount, lpVertDecl, lpVertics, nVertCount, pIndices, nIdxCount, GXResUsage::Default))
  {
    m_pWeight = new float[nClusterCount * nVertCount];
    memcpy(m_pWeight, pWeight, nClusterCount * nVertCount * sizeof(float));

    if(pSkeleton)
    {
      m_pSkeleton = pSkeleton;
      m_pSkeleton->AddRef();
    }

    m_nVertStride = MOGetDeclVertexSize(lpVertDecl);
    m_nPosOffset = MOGetDeclOffset(lpVertDecl, GXDECLUSAGE_POSITION, 0);
    m_nNormalOffset = MOGetDeclOffset(lpVertDecl, GXDECLUSAGE_NORMAL, 0);
    m_pVertices = new GXBYTE[m_nVertStride * nVertCount];
    m_nClusterCount = nClusterCount;
    memcpy(m_pVertices, lpVertics, m_nVertStride * nVertCount);

    return TRUE;
  }
  return FALSE;
}

GXBOOL GVSkinnedMeshSoft::Initialize(GrapX::Graphics* pGraphics, const GVMESHDATA* pMeshData, GVSkeleton* pSkeleton, float* pWeight, clsize nClusterCount)
{
  if(GVMesh::IntCreateMesh(pGraphics, pMeshData))
  {
    const GXSIZE_T nVertCount = pMeshData->nVertexCount;
    m_pWeight = new float[nClusterCount * nVertCount];
    memcpy(m_pWeight, pWeight, nClusterCount * nVertCount * sizeof(float));

    if(pSkeleton)
    {
      m_pSkeleton = pSkeleton;
      m_pSkeleton->AddRef();
    }
    m_nVertStride = m_Renderer.pPrimitive->GetVertexStride();
    m_nPosOffset = m_Renderer.pPrimitive->GetElementOffset(GXDECLUSAGE_POSITION, 0);
    m_nNormalOffset = m_Renderer.pPrimitive->GetElementOffset(GXDECLUSAGE_NORMAL, 0);
    m_pVertices = new GXBYTE[m_nVertStride * nVertCount];
    m_nClusterCount = nClusterCount;

    PrimitiveUtility::MapVertices locker_v(m_Renderer.pPrimitive, GXResMap::Write);
    memcpy(m_pVertices, locker_v.GetPtr(), m_nVertStride * nVertCount);

    return TRUE;
  }
  return FALSE;
}

GXHRESULT GVSkinnedMeshSoft::CreateMesh(
  GrapX::Graphics*    pGraphics,
  int                 nPrimCount, 
  GXLPCVERTEXELEMENT  lpVertDecl, 
  GXLPVOID            lpVertics, 
  int                 nVertCount, 
  GXWORD*             pIndices, 
  int                 nIdxCount, 
  GVSkeleton*         pSkeleton,
  float*              pWeight, 
  int                 nClusterCount,
  GVSkinnedMeshSoft** ppSkinnedMesh)
{
  GVSkinnedMeshSoft* pMesh = new GVSkinnedMeshSoft(pGraphics);
  if(pMesh == NULL) {
    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
    return NULL;
  }
  GXHRESULT hval = GX_OK;
  pMesh->AddRef();
  if( ! pMesh->Initialize(pGraphics, nPrimCount, lpVertDecl, lpVertics, nVertCount, 
    pIndices, nIdxCount, pSkeleton, pWeight, nClusterCount))
  {
    pMesh->Release();
    pMesh = NULL;
    hval = GX_FAIL;
  }
  *ppSkinnedMesh = pMesh;
  return hval;
}

GXHRESULT GVSkinnedMeshSoft::CreateFromMeshData(GrapX::Graphics* pGraphics, const GVMESHDATA* pMeshData, GVSkeleton* pSkeleton, float* pWeight, int nClusterCount, GVSkinnedMeshSoft** ppSkinnedMesh)
{
  GVSkinnedMeshSoft* pMesh = new GVSkinnedMeshSoft(pGraphics);
  if(pMesh == NULL) {
    CLOG_ERROR(MOERROR_FMT_OUTOFMEMORY);
    return NULL;
  }
  GXHRESULT hval = GX_OK;
  pMesh->AddRef();
  if( ! pMesh->Initialize(pGraphics, pMeshData, pSkeleton, pWeight, nClusterCount))
  {
    pMesh->Release();
    pMesh = NULL;
    hval = GX_FAIL;
  }
  *ppSkinnedMesh = pMesh;
  return hval;
}

#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
GXHRESULT GVSkinnedMeshSoft::Release()
{
  return GVMesh::Release();
}
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

GXBOOL GVSkinnedMeshSoft::CheckSkeleton(GVSkeleton* pSkeleton)
{
  return TRUE;
}

GXHRESULT GVSkinnedMeshSoft::SetSkeleton(GVSkeleton* pSkeleton)
{
  if( ! CheckSkeleton(pSkeleton)) {
    return GX_FAIL;
  }
  return InlSetNewObjectT(m_pSkeleton, pSkeleton);
}

int GVSkinnedMeshSoft::SetTrackData(GXLPCSTR szName, GVAnimationTrack* pTrack)
{
  return m_pSkeleton->SetTrackData(szName, pTrack);
}

GXBOOL GVSkinnedMeshSoft::PlayByName(GXLPCSTR szName)
{
  return m_pSkeleton->PlayByName(szName);
}

GXBOOL GVSkinnedMeshSoft::PlayById(int nId)
{
  return m_pSkeleton->PlayById(nId);
}

GXBOOL GVSkinnedMeshSoft::Update(const GVSCENEUPDATE& sContext)
{
  if(m_pSkeleton == NULL)
    return TRUE;

  //GXLPVOID pNewVertices = NULL;
  //GXWORD* pIndices = NULL;

  float3* pSrcPos = NULL;
  float3* pDestPos = NULL;
  float3* pSrcNormal = NULL;
  float3* pDestNormal = NULL;
  m_aabbLocal.Clear();

  PrimitiveUtility::MapVertices locker_v(m_Renderer.pPrimitive, GXResMap::ReadWrite);

  //if(m_pPrimitive->Lock(0, 0, 0, 0, &pNewVertices, &pIndices))
  if(locker_v.GetPtr())
  {
    GXLPVOID pNewVertices = locker_v.GetPtr();
    pSrcPos = (float3*)((GXBYTE*)m_pVertices + m_nPosOffset);
    pDestPos = (float3*)((GXBYTE*)pNewVertices + m_nPosOffset);

    pSrcNormal = (float3*)((GXBYTE*)m_pVertices + m_nNormalOffset);
    pDestNormal = (float3*)((GXBYTE*)pNewVertices + m_nNormalOffset);

    for (GXSIZE_T nVertIndex = 0; nVertIndex < m_Renderer.NumVertices; nVertIndex++)
    {
      float3 vPos(0.0f);
      float3 vNormal(0.0f);
      float3 vNormalStore;
      float4x4 mat;
      for(GXSIZE_T nClusterIndex = 0; nClusterIndex < m_nClusterCount; nClusterIndex++)
      {
        const float fWeight = m_pWeight[nVertIndex * m_nClusterCount + nClusterIndex];
        if(fWeight > 0) {
          const Bone& bone = m_pSkeleton->GetBones()[nClusterIndex];
          vPos += ((*pSrcPos) * (bone.BindPose * bone.matAbs)) * fWeight;

          mat = bone.BindPose * bone.matAbs;
          clstd::Vec3TransformNormal(&vNormalStore, pSrcNormal, &mat);
          vNormal += vNormalStore * fWeight;
        }
      }
      vNormal.normalize();

      m_aabbLocal.AppendVertex(vPos);
      //TRACE("[%d] %f,%f,%f\n", nVertIndex, vPos.x,vPos.y,vPos.z)
      *pDestPos = vPos;
      *pDestNormal = vNormal;
      pSrcPos = (float3*)((GXBYTE*)pSrcPos + m_nVertStride);
      pDestPos = (float3*)((GXBYTE*)pDestPos + m_nVertStride);
      pSrcNormal = (float3*)((GXBYTE*)pSrcNormal + m_nVertStride);
      pDestNormal = (float3*)((GXBYTE*)pDestNormal + m_nVertStride);
    }
    //m_pPrimitive->Unlock();
  }
  return TRUE;
}

GXHRESULT GVSkinnedMeshSoft::SaveFile(clSmartRepository* pStorage)
{
  pStorage->WriteStringA(NULL, SKINNEDMESH_NAME, GetName());

  if (m_Renderer.materials.empty() == FALSE && m_Renderer.materials[0] != NULL)
  {
    clStringW strMtlFile;
    m_Renderer.materials[0]->GetFilename(&strMtlFile);
    if(strMtlFile.IsNotEmpty()) {
      m_Renderer.pPrimitive->GetGraphicsUnsafe()->ConvertToRelativePathW(strMtlFile);
      pStorage->WriteStringW(NULL, SKINNEDMESH_MTLINST, strMtlFile);
    }
  }

  if(m_Renderer.pPrimitive != NULL)
  {
    //GVertexDeclaration* pVertexDecl = NULL;
    //if(GXSUCCEEDED(m_pPrimitive->GetVertexDeclaration(&pVertexDecl))) {
    //  GXLPCVERTEXELEMENT  lpVertexElement = pVertexDecl->GetVertexElement();
    //  GXUINT              nElementCount   = MOGetDeclCount(lpVertexElement);
    //  pStorage->WriteStructArrayT(NULL, SKINNEDMESH_VERTEXDECL, 
    //    *lpVertexElement, (nElementCount + 1));
    //  SAFE_RELEASE(pVertexDecl);
    //}

    float4x4 matLocal = m_Transformation.ToRelativeMatrix();
    pStorage->WriteStructT(NULL, SKINNEDMESH_TRANSFORM, matLocal);

    ASSERT(m_Renderer.pPrimitive->GetVertexStride() == m_nVertStride);
    //pStorage->Write(NULL, SKINNEDMESH_ASMVERTICES, m_pVertices, 
    //  m_nVertStride * m_pPrimitive->GetVerticesCount());

    //pStorage->Write(NULL, SKINNEDMESH_INDICES, m_pPrimitive->GetIndicesBuffer(), 
    //  m_pPrimitive->GetIndexCount() * sizeof(VIndex));

    //pStorage->Write64(NULL, SKINNEDMESH_PRIMCOUNT, m_nPrimiCount, 0);
    //pStorage->Write64(NULL, SKINNEDMESH_STARTINDEX, m_nStartIndex, 0);
    RepoUtility::SavePrimitive(pStorage, "SkMesh", m_Renderer.pPrimitive, m_Renderer.StartIndex, m_Renderer.PrimitiveCount);
    
    pStorage->Write(NULL, SKINNEDMESH_WEIGHT, m_pWeight, (u32)(m_Renderer.NumVertices * m_nClusterCount * sizeof(float)));
    pStorage->Write64(NULL, SKINNEDMESH_CLUSTERCOUNT, (u32)m_nClusterCount, 0);
  }

  ASSERT(GetFirstChild() == NULL);
  return GX_OK;
}

GXHRESULT GVSkinnedMeshSoft::LoadFile(GrapX::Graphics* pGraphics, clSmartRepository* pStorage)
{
  Clear();
  clStringA strName;
  clStringW strMtlName;
  s32 nReadSize;
  float4x4 matLocal = m_Transformation.ToRelativeMatrix();
  GXVERTEXELEMENT VertexElement[64];
  if( ! pStorage->ReadStringA(NULL, SKINNEDMESH_NAME, strName)) {
    CLOG_ERROR("%s : Can not find mesh name.\n", __FUNCTION__);
  }
  InlSetZeroT(VertexElement);
  pStorage->ReadStringW(NULL, SKINNEDMESH_MTLINST, strMtlName);    
  //nReadSize = pStorage->ReadStructArrayT(NULL, SKINNEDMESH_VERTEXDECL, VertexElement, 64);
  //if(nReadSize < 0) {
  //  ASSERT(0);
  //}

  if( ! pStorage->ReadStructT(NULL, SKINNEDMESH_TRANSFORM, matLocal)) {
    matLocal.identity();
  }

  //s32 nIndexCount = pStorage->GetLength(NULL, SKINNEDMESH_INDICES) / sizeof(VIndex);
  s32 cbWeightSize = pStorage->GetLength(NULL, SKINNEDMESH_WEIGHT);

  //GXBYTE* pVertices = NULL;
  //VIndex* pIndices = NULL;
  clBuffer Vertices;
  clBuffer Indices;
  GXBYTE* pWeight = NULL;

  try {
    //pVertices = new GXBYTE[cbVertSize];
    //pIndices = new VIndex[nIndexCount];
    pWeight = new GXBYTE[cbWeightSize];


    //nReadSize = pStorage->Read(NULL, SKINNEDMESH_ASMVERTICES, pVertices, cbVertSize);
    //if(nReadSize != cbVertSize) {
    //  ASSERT(0);
    //}
    //nReadSize = pStorage->Read(NULL, SKINNEDMESH_INDICES, pIndices, nIndexCount * sizeof(VIndex));
    //if(nReadSize != nIndexCount * sizeof(VIndex))
    //{
    //  ASSERT(0);
    //}

    clsize nPrimiCount;
    clsize nStartIndex;
    clsize nClusterCount;

    RepoUtility::LoadPrimitive(pStorage, "SkMesh", VertexElement, &Vertices, &Indices, nStartIndex, nPrimiCount);

    s32 nStride = MOGetDeclVertexSize(VertexElement);
    size_t nVertCount = Vertices.GetSize() / nStride;

    //pStorage->Read64(NULL, SKINNEDMESH_PRIMCOUNT, (u32*)&nPrimiCount, 0);
    //pStorage->Read64(NULL, SKINNEDMESH_STARTINDEX, (u32*)&nStartIndex, 0);
    pStorage->Read64(NULL, SKINNEDMESH_CLUSTERCOUNT, (u32*)&nClusterCount, 0);

    nReadSize = pStorage->Read(NULL, SKINNEDMESH_WEIGHT, pWeight, (u32)(nVertCount * nClusterCount * sizeof(float)));


    GXBOOL bval = Initialize(pGraphics, nPrimiCount, VertexElement, Vertices.GetPtr(), nVertCount, (VIndex*)Indices.GetPtr(), 
      Indices.GetSize() / sizeof(VIndex), NULL, (float*)pWeight, nClusterCount);
    //GXBOOL bval = IntCreatePrimitive(pGraphics, nPrimiCount, VertexElement, pVertices, 
    //  nPrimiCount * 3, pIndices, nIndexCount);

    if(bval) {
      SetMaterialFromFile(pGraphics, strMtlName, NODEMTL_CLONEINST);
        //FALSE, MLT_CLONE);
      SetName(strName);
      SetTransform(matLocal);
    }
    //SAFE_DELETE_ARRAY(pVertices);
    //SAFE_DELETE_ARRAY(pIndices);
    SAFE_DELETE_ARRAY(pWeight);
    return bval;
  }
  catch(...)
  {
    //SAFE_DELETE_ARRAY(pVertices);
    //SAFE_DELETE_ARRAY(pIndices);
    SAFE_DELETE_ARRAY(pWeight);
    return FALSE;
  }
}

//////////////////////////////////////////////////////////////////////////
GXHRESULT GVSkinnedMeshSoft::CreateFromFileA(GrapX::Graphics* pGraphics, GXLPCSTR szFilename, GVSkinnedMeshSoft** ppMesh)
{
  clStringW strFilename = szFilename;
  return CreateFromFileW(pGraphics, strFilename, ppMesh);
}

GXHRESULT GVSkinnedMeshSoft::CreateFromFileW(GrapX::Graphics* pGraphics, GXLPCWSTR szFilename, GVSkinnedMeshSoft** ppMesh)
{
  GVSkinnedMeshSoft* pMesh = new GVSkinnedMeshSoft(NULL);
  if( ! InlCheckNewAndIncReference(pMesh)) {
    return GX_FAIL;
  }

  GXHRESULT hval = pMesh->LoadFileW(pGraphics, szFilename);
  if(GXSUCCEEDED(hval)) {
    *ppMesh = pMesh;
    return hval;
  }
  SAFE_RELEASE(pMesh);
  return hval;
}

GXHRESULT GVSkinnedMeshSoft::CreateFromRepository(GrapX::Graphics* pGraphics, clSmartRepository* pStorage, GVSkinnedMeshSoft** ppMesh)
{
  GVSkinnedMeshSoft* pMesh = new GVSkinnedMeshSoft(NULL);
  if( ! InlCheckNewAndIncReference(pMesh)) {
    return GX_FAIL;
  }

  if(pMesh->LoadFile(pGraphics, pStorage)) {
    *ppMesh = pMesh;
    return GX_OK;
  }
  SAFE_RELEASE(pMesh);
  return GX_FAIL;
}

GVSkinnedMeshSoft::GVSkinnedMeshSoft( GrapX::Graphics* pGraphics ) : GVMesh(pGraphics)
  , m_pWeight(NULL)
  , m_pSkeleton(NULL)
  , m_pVertices(NULL)
  , m_nVertStride(0)
  , m_nPosOffset(0)
  , m_nNormalOffset(0)
  , m_nClusterCount(NULL)
{

}

GVSkinnedMeshSoft::~GVSkinnedMeshSoft()
{
  SAFE_RELEASE(m_pSkeleton);
  SAFE_DELETE(m_pWeight);
  SAFE_DELETE(m_pVertices);
}
