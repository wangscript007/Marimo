#ifndef _IMPLEMENT_GRAPH_X_SPRITE_HEADER_FILE_
#define _IMPLEMENT_GRAPH_X_SPRITE_HEADER_FILE_

//class GXImage;
namespace GrapX
{
  class Canvas;
  class Graphics;
}

class GXSpriteDescImpl : public GXSpriteDesc
{
  //friend GXHRESULT GXDLLAPI GXLoadSpriteDescW(GXLPCWSTR szSpriteFile, GXSpriteDescObj** ppDesc);
  typedef clvector<GXSprite::MODULE>        ModuleArray;
  typedef clvector<GXSprite::FRAME>         FrameArray;
  typedef clvector<GXSprite::FRAME_MODULE>  FrameModuleArray;
  typedef clvector<GXSprite::ANIMATION>     AnimationArray;
  typedef clvector<GXUINT>                  AnimFrameArray;
  typedef clstd::StringSetA                 clStringSetA;

protected:
  clStringW         m_strImageFile;
  clStringSetA      m_NameSet;      // 用来储存字符串的集合
  ModuleArray       m_aModules;
  FrameArray        m_aFrames;
  FrameModuleArray  m_aFrameModules;
  AnimationArray    m_aAnimations;
  AnimFrameArray    m_aAnimFrames;

public:
  virtual ~GXSpriteDescImpl();
  GXSPRITE_DESCW ToDesc();

public:
  friend GXHRESULT IntLoadSpriteDesc(clstd::StockA& ss, GXLPCWSTR szSpriteFile, GXSpriteDesc** ppDesc);
  friend GXHRESULT IntLoadModules   (clstd::StockA& ss, GXSpriteDescImpl* pDescObj);
  friend GXHRESULT IntLoadFrames    (clstd::StockA& ss, GXSpriteDescImpl* pDescObj);
  friend GXHRESULT IntLoadAnimations(clstd::StockA& ss, GXSpriteDescImpl* pDescObj) ;
};

class GXSpriteImpl : public GXSprite
{
public:
  //struct MODULE
  //{
  //  clStringA name;
  //  Regn      regn;
  //};

  //typedef clvector<MODULE>      ModuleArray;
  struct IDATTR{
    GXSprite::Type type;
    union {
      GXUINT               index;
      GXSprite::MODULE*    pModel;
      GXSprite::FRAME*     pFrame;
      GXSprite::ANIMATION* pAnimation;
    };
  };

  typedef clmap<clStringA, IDATTR>          NameDict;
  typedef clmap<ID, IDATTR>                 IDDict;

  typedef clvector<GXSprite::MODULE>        ModuleArray;
  typedef clvector<GXSprite::FRAME>         FrameArray;
  typedef clvector<GXSprite::FRAME_MODULE>  FrameModuleArray;
  typedef clvector<GXSprite::ANIMATION>     AnimationArray;
  typedef clvector<GXUINT>                  AnimFrameArray;
  typedef clstd::StringSetA                 clStringSetA;
private:
  clStringW         m_strImageFile;
  //GXImage*          m_pImage;
  GrapX::Texture*   m_pTexture;

  clStringSetA      m_NameSet; // TODO: 这个将来可以取消，使用压实的一大块内存储存字符串序列

  ModuleArray       m_aModules;
  
  FrameArray        m_aFrames;
  FrameModuleArray  m_aFrameModules;

  AnimationArray    m_aAnimations;
  AnimFrameArray    m_aAnimFrames;

  NameDict          m_NameDict;
  IDDict            m_IDDict;
  int IntGetSpriteCount() const;
protected:
  virtual ~GXSpriteImpl();
public:
  GXSpriteImpl();
  GXBOOL Initialize(GrapX::Graphics* pGraphics, const GXSPRITE_DESCW* pDesc);
  GXBOOL Initialize(GrapX::Graphics* pGraphics, GXLPCWSTR szTextureFile, GXREGN *arrayRegion, GXSIZE_T nCount);


#ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE
  virtual GXHRESULT AddRef          ();
  virtual GXHRESULT Release         ();
#endif // #ifdef ENABLE_VIRTUALIZE_ADDREF_RELEASE

  //virtual GXHRESULT SaveW             (GXLPCWSTR szFilename) const;

  GXSTDIMPLEMENT(GXVOID    PaintModule          (GrapX::Canvas *pCanvas, GXINT nIndex, GXINT x, GXINT y) const);
  GXSTDIMPLEMENT(GXVOID    PaintModule          (GrapX::Canvas *pCanvas, GXINT nIndex, GXLPCREGN lpRegn) const);
  GXSTDIMPLEMENT(GXVOID    PaintModule          (GrapX::Canvas *pCanvas, GXINT nIndex, GXINT x, GXINT y, GXINT right, GXINT height) const);
  
  GXSTDIMPLEMENT(GXVOID    PaintModule3H        (GrapX::Canvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const);
  GXSTDIMPLEMENT(GXVOID    PaintModule3V        (GrapX::Canvas *pCanvas, GXINT nStartIdx, GXINT x, GXINT y, GXINT nWidth, GXINT nHeight) const);
  GXSTDIMPLEMENT(GXVOID    PaintModule3x3       (GrapX::Canvas *pCanvas, GXINT nStartIdx, GXBOOL bDrawEdge, GXLPCRECT rect) const);
  
  GXSTDIMPLEMENT(GXVOID    PaintFrame           (GrapX::Canvas *pCanvas, GXUINT nIndex, GXINT x, GXINT y) const);
  GXSTDIMPLEMENT(GXVOID    PaintFrame           (GrapX::Canvas *pCanvas, GXUINT nIndex, GXLPCREGN lpRegn) const);
  GXSTDIMPLEMENT(GXVOID    PaintFrame           (GrapX::Canvas *pCanvas, GXUINT nIndex, GXLPCRECT lpRect) const);
  
  GXSTDIMPLEMENT(GXVOID    PaintAnimationFrame  (GrapX::Canvas *pCanvas, GXUINT nAnimIndex, GXUINT nFrameIndex, GXINT x, GXINT y) const);
  GXSTDIMPLEMENT(GXVOID    PaintAnimationFrame  (GrapX::Canvas *pCanvas, GXUINT nAnimIndex, GXUINT nFrameIndex, GXLPCREGN lpRegn) const);
  GXSTDIMPLEMENT(GXVOID    PaintAnimationFrame  (GrapX::Canvas *pCanvas, GXUINT nAnimIndex, GXUINT nFrameIndex, GXLPCRECT lpRect) const);
  GXSTDIMPLEMENT(GXVOID    PaintAnimationByTime (GrapX::Canvas *pCanvas, GXUINT nAnimIndex, TIME_T time, GXINT x, GXINT y));
  GXSTDIMPLEMENT(GXVOID    PaintAnimationByTime (GrapX::Canvas *pCanvas, GXUINT nAnimIndex, TIME_T time, GXLPCREGN lpRegn));
  GXSTDIMPLEMENT(GXVOID    PaintAnimationByTime (GrapX::Canvas *pCanvas, GXUINT nAnimIndex, TIME_T time, GXLPCRECT lpRect));

  GXSTDIMPLEMENT(GXVOID    Paint                (GrapX::Canvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y) const);
  GXSTDIMPLEMENT(GXVOID    Paint                (GrapX::Canvas *pCanvas, ID id, TIME_T time, GXLPCREGN lpRegn) const);
  GXSTDIMPLEMENT(GXVOID    Paint                (GrapX::Canvas *pCanvas, ID id, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const);
  GXSTDIMPLEMENT(GXVOID    Paint                (GrapX::Canvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y) const);
  GXSTDIMPLEMENT(GXVOID    Paint                (GrapX::Canvas *pCanvas, GXLPCSTR name, TIME_T time, GXLPCREGN lpRegn) const);
  GXSTDIMPLEMENT(GXVOID    Paint                (GrapX::Canvas *pCanvas, GXLPCSTR name, TIME_T time, GXINT x, GXINT y, GXINT right, GXINT bottom) const);

  GXSTDIMPLEMENT(GXINT     Find                 (ID id, GXOUT Type* pType) const); // pType 可以设置为NULL, 不返回类型
  GXSTDIMPLEMENT(GXINT     Find                 (GXLPCSTR szName, GXOUT Type* pType) const);
  GXSTDIMPLEMENT(GXINT     Find                 (GXLPCWSTR szName, GXOUT Type* pType) const);
  GXSTDIMPLEMENT(GXLPCSTR  FindName             (ID id) const);           // 用 ID 查找 Name
  GXSTDIMPLEMENT(ID        FindID               (GXLPCSTR szName) const); // 用 Name 查找 ID
  GXSTDIMPLEMENT(ID        FindID               (GXLPCWSTR szName) const); // 用 Name 查找 ID

  GXSTDIMPLEMENT(GXINT     PackIndex            (Type type, GXUINT index) const);   // 将不同类型的索引打包为统一类型的索引
  GXSTDIMPLEMENT(GXINT     UnpackIndex          (GXUINT nUniqueIndex, Type* pType) const); // 将统一索引拆解为类型和类型索引

  GXSTDIMPLEMENT(GXSIZE_T  GetModuleCount       () const);
  GXSTDIMPLEMENT(GXSIZE_T  GetFrameCount        () const);
  GXSTDIMPLEMENT(GXSIZE_T  GetFrameModuleCount  (GXUINT nFrameIndex) const);
  GXSTDIMPLEMENT(GXSIZE_T  GetAnimationCount    () const);
  GXSTDIMPLEMENT(GXSIZE_T  GetAnimFrameCount    (GXUINT nAnimIndex) const);
  //virtual GXBOOL    GetNameA          (IndexType eType, GXUINT nIndex, clStringA* pstrName) const;

  GXSTDIMPLEMENT(GXBOOL    GetModule            (GXUINT nIndex, MODULE* pModule) const);
  GXSTDIMPLEMENT(GXBOOL    GetFrame             (GXUINT nIndex, FRAME* pFrame) const);
  GXSTDIMPLEMENT(GXUINT    GetFrameModule       (GXUINT nIndex, FRAME_MODULE* pFrameModule, GXSIZE_T nCount) const);
  GXSTDIMPLEMENT(GXBOOL    GetAnimation         (GXUINT nIndex, ANIMATION* pAnimation) const);
  GXSTDIMPLEMENT(GXUINT    GetAnimFrame         (GXUINT nIndex, ANIM_FRAME* pAnimFrame, GXSIZE_T nCount) const);

  
  virtual GXBOOL    GetModuleRect     (GXUINT nIndex, GXRECT *rcSprite) const;
  virtual GXBOOL    GetModuleRegion   (GXUINT nIndex, REGN *rgSprite) const;
  virtual GXBOOL    GetFrameBounding  (GXUINT nIndex, GXRECT* lprc) const;
  virtual GXBOOL    GetAnimBounding   (GXUINT nIndex, GXRECT* lprc) const;

  template<typename _TID>
  Type GetBoundingT(_TID id, GXLPRECT lprc) const;
  template<typename _TID>
  Type GetBoundingT(_TID id, GXLPREGN lprg) const;

  GXSTDIMPLEMENT(Type      GetBounding          (ID id, GXLPRECT lprc) const); // 对于Module，返回值的left和top都应该是0
  GXSTDIMPLEMENT(Type      GetBounding          (ID id, GXLPREGN lprg) const);
  GXSTDIMPLEMENT(Type      GetBounding          (GXLPCSTR szName, GXLPRECT lprc) const); // 对于Module，返回值的left和top都应该是0
  GXSTDIMPLEMENT(Type      GetBounding          (GXLPCSTR szName, GXLPREGN lprg) const);
  GXSTDIMPLEMENT(Type      GetBounding          (GXLPCWSTR szName, GXLPRECT lprc) const); // 对于Module，返回值的left和top都应该是0
  GXSTDIMPLEMENT(Type      GetBounding          (GXLPCWSTR szName, GXLPREGN lprg) const);

  GXHRESULT GetTexture        (GrapX::Texture** ppTexture) override;
  clStringW GetImageFileW     () const override;
  clStringA GetImageFileA     () const override;

  //virtual int FindByNameA             (GXLPCSTR szName) const;
  //virtual int FindByNameW             (GXLPCWSTR szName) const;

  template<class _TArray, class _TDesc>
  void Add(_TArray& aArray, Type type, _TDesc& desc);

  const IDATTR* IntFind(ID id) const;
  const IDATTR* IntFind(GXLPCSTR szName) const;
  GXINT AttrToIndex(const IDATTR* pAttr) const;
  void  IntGetBounding(const IDATTR* pAttr, GXREGN* lprg) const;

  friend GXHRESULT GXDLLAPI GXCreateSprite          (GrapX::Graphics* pGraphics, GXLPCWSTR szTextureFile, GXREGN*  aRegion, GXINT nCount, GXSprite** ppSprite);
  friend GXHRESULT GXDLLAPI GXCreateSpriteEx        (GrapX::Graphics* pGraphics, const GXSPRITE_DESCW* pDesc, GXSprite** ppSprite);
  friend GXHRESULT GXDLLAPI GXCreateSpriteArray     (GrapX::Graphics* pGraphics, GXLPCWSTR szTextureFile, int xStart, int yStart, int nTileWidth, int nTileHeight, int xGap, int yGap, GXSprite** ppSprite);
  friend GXHRESULT GXDLLAPI GXCreateSpriteFromFileW (GrapX::Graphics* pGraphics, GXLPCWSTR szSpriteFile, GXSprite** ppSprite);
  friend GXHRESULT GXDLLAPI GXCreateSpriteFromFileA (GrapX::Graphics* pGraphics, GXLPCSTR szSpriteFile, GXSprite** ppSprite);
  //friend GXHRESULT GXDLLAPI GXLoadSpriteDescW       (GXLPCWSTR szSpriteFile, GXSpriteDescObj** ppDesc);
  //friend GXHRESULT GXDLLAPI GXLoadSpriteDescA       (GXLPCSTR szSpriteFile, GXSpriteDescObj** ppDesc);
};


#endif // _IMPLEMENT_GRAPH_X_SPRITE_HEADER_FILE_