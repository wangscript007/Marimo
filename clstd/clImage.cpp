﻿#include "clstd.h"
#include "clImage.h"
const int delta = 'a' - 'A';

#define CMP_CHANNEL(_IDX)                           (m_format.name[_IDX] == fmt[_IDX] || m_format.name[_IDX] + delta == fmt[_IDX])
#define MIN_PITCH_PARAM(_WIDTH, _CHANNEL, _DEPTH)   ((_WIDTH) * (_CHANNEL) * ((_DEPTH) >> 3))
#define MIN_PITCH                                   MIN_PITCH_PARAM(m_width, m_channel, m_depth)
#define GETPIXELSIZE                                (m_channel * (m_depth >> 3))
namespace clstd
{
  Image::Image()
    : m_ptr(NULL)
    , m_width(0)
    , m_height(0)
    , m_channel(0)
    , m_pitch(0)
    , m_depth(0)
  {
    m_format.code = 0;
  }

  Image::Image(const Image& image)
    : m_ptr(NULL)
    , m_width(image.m_width)
    , m_height(image.m_height)
    , m_channel(image.m_channel)
    , m_pitch(image.m_pitch)
    , m_depth(image.m_depth)
  {
    m_format.code = image.m_format.code;
    if(image.m_ptr)
    {
      m_ptr = new u8[GetDataSize()];
      memcpy(m_ptr, image.m_ptr, GetDataSize());
    }
  }

  Image::~Image()
  {
    SAFE_DELETE_ARRAY(m_ptr);
  }

  int Image::GetWidth() const
  {
    return m_width;
  }

  int Image::GetHeight() const
  {
    return m_height;
  }

  const void* Image::GetPixelPtr(int x, int y) const
  {
    return (CLLPBYTE)m_ptr + m_pitch * y + x;
  }

  void* Image::GetPixelPtr(int x, int y)
  {
    return (CLLPBYTE)m_ptr + m_pitch * y + x;
  }

  size_t Image::GetDataSize() const
  {
    return (size_t)(m_pitch * m_height);
  }

  int Image::GetDepth() const
  {
    return m_depth;
  }

  b32 Image::Set(int nWidth, int nHeight, const char* fmt, const void* pData)
  {
    if (m_width != nWidth || m_height != nHeight || m_depth != 8 ||
      MIN_PITCH_PARAM(nWidth, m_channel, m_depth) != m_pitch || !CompareFormat(fmt))
    {
      if (!IntParseFormat(fmt, &m_format.code, &m_channel)) {
        return FALSE;
      }

      SAFE_DELETE_ARRAY(m_ptr);
      m_width = nWidth;
      m_height = nHeight;
      m_depth = 8;
      m_pitch = MIN_PITCH;
      m_ptr = new u8[GetDataSize()];
    }

    if (pData) {
      memcpy(m_ptr, pData, GetDataSize());
    }
    return true;
  }

  b32 Image::Set(int nWidth, int nHeight, const char* fmt, int nPitch, int nDepth, const void* pData)
  {
    if (m_width != nWidth || m_height != nHeight || m_depth != nDepth ||
      m_pitch != nPitch || !CompareFormat(fmt))
    {
      u32 fmtcode;
      int channel;
      if (!IntParseFormat(fmt, &fmtcode, &channel)) {
        return FALSE;
      }

      if (nPitch == 0) {
        nPitch = MIN_PITCH_PARAM(nWidth, channel, nDepth);
      }
      else if (MIN_PITCH_PARAM(nWidth, channel, nDepth) < nPitch) {
        return FALSE;
      }

      SAFE_DELETE_ARRAY(m_ptr);
      m_width = nWidth;
      m_height = nHeight;
      m_pitch = nPitch;
      m_channel = channel;
      m_depth = nDepth;
      m_ptr = new u8[GetDataSize()];
      m_format.code = fmtcode;
    }

    if (pData) {
      memcpy(m_ptr, pData, GetDataSize());
    }
    return true;

  }

  b32 Image::CompareFormat(const char* fmt) const
  {
    switch (m_channel)
    {
    case 1: return CMP_CHANNEL(0) && fmt[1] == '\0';
    case 2: return CMP_CHANNEL(0) && CMP_CHANNEL(1) && fmt[2] == '\0';
    case 3: return CMP_CHANNEL(0) && CMP_CHANNEL(1) && CMP_CHANNEL(2) && fmt[3] == '\0';
    case 4: return CMP_CHANNEL(0) && CMP_CHANNEL(1) && CMP_CHANNEL(2) && CMP_CHANNEL(3);
    }
    return FALSE;
  }

  b32 Image::IntParseFormat(const char* fmt, u32* pFmtCode, int* pChannel)
  {
    int channel = 0;
    union
    {
      u8  name[4];
      u32 code;
    }format;
    format.code = 0;

    for (int i = 0; i < 4; i++)
    {
      if (fmt[i] == 'R' || fmt[i] == 'G' || fmt[i] == 'B' || fmt[i] == 'A' || fmt[i] == 'X') {
        format.name[i] = fmt[i];
      }
      else if (fmt[i] == 'r' || fmt[i] == 'g' || fmt[i] == 'b' || fmt[i] == 'a' || fmt[i] == 'x') {
        format.name[i] = fmt[i] - ('a' - 'A');
      }
      else if (fmt[i] == '\0') { break; }
      else { return FALSE; }
      channel++;
    }

    // 这样写如果返回false，则不会修改fmtcode和channel的任何内容
    *pChannel = channel;
    *pFmtCode = format.code;
    return TRUE;
  }

  int Image::IntGetChanelIndex(const PIXELFORMAT& fmt, int nNumOfChannels, char chChannelCode)
  {
    for (int i = 0; i < nNumOfChannels; i++) {
      ASSERT(fmt.name[i] == 'R' || fmt.name[i] == 'G' ||
        fmt.name[i] == 'B' || fmt.name[i] == 'A' || fmt.name[i] == 'X');
      if (fmt.name[i] == chChannelCode) {
        return i;
      }
    }
    return -1;
  }

  const void* Image::GetLine(int y) const
  {
    return (CLLPBYTE)m_ptr + m_pitch * y;
  }

  void* Image::GetLine(int y)
  {
    return (CLLPBYTE)m_ptr + m_pitch * y;
  }

  int Image::Inflate(int left, int top, int right, int bottom)
  {
    // 没实现
    CLBREAK;
    return 0;
  }

  int Image::GetChannelOffset(char chChannel) const
  {
    const int index = IntGetChanelIndex(m_format, m_channel, chChannel);
    return index >= 0 ? (index * (m_depth >> 3)) : -1;
  }

  b32 Image::GetChannelPlane(Image* pDestImage, char chChannel)
  {
    int nOffset = GetChannelOffset(chChannel);
    if (nOffset < 0) {
      return FALSE;
    }

    char fmt[8] = { 0 };
    fmt[0] = chChannel;

    if (!pDestImage->Set(m_width, m_height, fmt, 0, m_depth, NULL)) {
      return FALSE;
    }
    const int nPixelSize = GETPIXELSIZE;

    switch (m_depth)
    {
    case 8:
      IntCopyChannel<u8>(pDestImage, nOffset, nPixelSize);
      break;

    case 16:
      IntCopyChannel<u16>(pDestImage, nOffset, nPixelSize);
      break;

    case 32:
      IntCopyChannel<u32>(pDestImage, nOffset, nPixelSize);
      break;

    default:
      CLBREAK;
      return FALSE;
    }
    return TRUE;
  }

  b32 Image::ScaleNearest(Image* pDestImage, int nWidth, int nHeight)
  {
    if (!pDestImage->Set(nWidth, nHeight, (const char*)m_format.name, 0, m_depth, NULL)) {
      return FALSE;
    }

    // TODO: Copy ...
    switch (GETPIXELSIZE)
    {
    case 1: // 8 depth 1 channel
      IntStretchCopy<u8>(pDestImage, nWidth, nHeight);
      break;

    case 2: // 16 depth 1 channel
            //  8 depth 2 channel
      IntStretchCopy<u16>(pDestImage, nWidth, nHeight);
      break;

    case 3: // 8 depth 3 channel
      IntStretchCopyMulti<u8>(pDestImage, nWidth, nHeight, 3);
      break;

    case 4: // 32 depth 1 channel
            // 16 depth 2 channel
            //  8 depth 4 channel
      IntStretchCopy<u32>(pDestImage, nWidth, nHeight);
      break;

    case 6: // 16 depth 3 channel
      IntStretchCopyMulti<u16>(pDestImage, nWidth, nHeight, 3);
      break;

    case 8: // 16 depth 4 channel
            // 32 depth 2 channel
      IntStretchCopy<u64>(pDestImage, nWidth, nHeight);
      break;

    case 12: // 32 depth 3 channel
      IntStretchCopyMulti<u32>(pDestImage, nWidth, nHeight, 3);
      break;

    case 16: // 32 depth 4 channel
      IntStretchCopyMulti<u32>(pDestImage, nWidth, nHeight, 4);
      break;

    default:
      CLBREAK;
      CLOG_ERROR("%s: Unsupported pixel size.\r\n", __FUNCTION__);
      break;
    }

    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////
  void Image::BlockTransfer(IMAGEDESC* pDest, int x, int y, int nCopyWidth, int nCopyHeight, const IMAGEDESC* pSrc)
  {
    CLBREAK;
  }

  template<typename _Ty>
  void Image::IntCopyChannel(Image* pDestImage, int nOffset, const int nPixelSize)
  {
    _Ty* pDestData;
    _Ty* pSrcData;
    for (int y = 0; y < m_height; y++)
    {
      pDestData = (_Ty*)((CLLPBYTE)pDestImage->GetLine(y));
      pSrcData = (_Ty*)((CLLPBYTE)GetLine(y) + nOffset);
      for (int x = 0; x < m_width; x++)
      {
        *pDestData++ = *pSrcData;
        pSrcData = (_Ty*)((CLLPBYTE)pSrcData + nPixelSize);
      }
    }
  }

  template<typename _Ty>
  void Image::IntStretchCopy(Image* pDestImage, int nWidth, int nHeight)
  {
    int* aLine = new int[nWidth];
    const float fStrideH = (float)m_width / (float)nWidth;
    const float fStrideV = (float)m_height / (float)nHeight;
    float fx = 0;
    float fy = 0;

    for (int x = 0; x < nWidth; x++) {
      aLine[x] = (int)floor(fx + 0.5f);
      fx += fStrideH;
    }

    for (int y = 0; y < nHeight; y++)
    {
      const _Ty* pSrcData = (const _Ty*)GetLine((int)floor(fy + 0.5f));
      _Ty* pDestData = (_Ty*)pDestImage->GetLine(y);
      for (int x = 0; x < nWidth; x++)
      {
        *pDestData = pSrcData[aLine[x]];
        pDestData++;
      }
      fy += fStrideV;
    }
    SAFE_DELETE_ARRAY(aLine);
  }

  template<typename _Ty>
  void Image::IntStretchCopyMulti(Image* pDestImage, int nWidth, int nHeight, int nCount)
  {
    int* aLine = new int[nWidth];
    const float fStrideH = (float)m_width / (float)nWidth;
    //const float fStrideV = (float)m_height / (float)nHeight;
    float fx = 0;
    float fy = 0;

    for (int x = 0; x < nWidth; x++) {
      aLine[x] = (int)floor(fx + 0.5f);
      fx += fStrideH;
    }

    for (int y = 0; y < nHeight; y++)
    {
      const _Ty* pSrcData = (const _Ty*)GetLine((int)floor(fy + 0.5f));
      _Ty* pDestData = (_Ty*)pDestImage->GetLine(y);
      for (int x = 0; x < nWidth; x++)
      {
        for (int i = 0; i < nCount; i++)
        {
          *pDestData = pSrcData[aLine[x] + i];
          pDestData++;
        }
      }
    }
    SAFE_DELETE_ARRAY(aLine);
  }

  const char* Image::GetFormat() const
  {
    return reinterpret_cast<const char*>(m_format.name);
  }

  template<typename _TChannel>
  void Image::ChangeFormat(int* aMapTab, int nNewChannel, CLBYTE* pDestData, int nNewPitch)
  {
    switch (nNewChannel)
    {
    case 1:
    {
      struct PIXEL { _TChannel channel[1]; };
      ChangePixel<PIXEL>(aMapTab, nNewChannel, pDestData, nNewPitch);
      break;
    }
    case 2:
    {
      struct PIXEL { _TChannel channel[2]; };
      ChangePixel<PIXEL>(aMapTab, nNewChannel, pDestData, nNewPitch);
      break;
    }
    case 3:
    {
      struct PIXEL { _TChannel channel[3]; };
      ChangePixel<PIXEL>(aMapTab, nNewChannel, pDestData, nNewPitch);
      break;
    }
    case 4:
    {
      struct PIXEL { _TChannel channel[4]; };
      ChangePixel<PIXEL>(aMapTab, nNewChannel, pDestData, nNewPitch);
      break;
    }
    default:
      break;
    }
  }

  template<typename _TPixel>
  void Image::ChangePixel(int* aMapTab, int nNewChannel, CLBYTE* pDestData, int nNewPitch)
  {
    _TPixel* pDstPixel;
    _TPixel* pSrcPixel;
    _TPixel  tmp_pixel;
    for (int y = 0; y < m_height; y++)
    {
      pDstPixel = (_TPixel*)GetLine(y);
      pSrcPixel = (_TPixel*)(pDestData + y * nNewPitch);
      for (int x = 0; x < m_width; x++)
      {
        tmp_pixel = *pSrcPixel++;
        for (int c = 0; c < nNewChannel; c++)
        {
          (*pDstPixel).channel[c] = aMapTab[c] >= 0 ? tmp_pixel.channel[aMapTab[c]] : 0;
        }
        pDstPixel++;
      }
    }
  }

  b32 Image::SetFormat(const char* fmt)
  {
    //u32 NewFmt = 0;
    PIXELFORMAT NewFormat;
    int nNewChannel = 0;
    if( ! IntParseFormat(fmt, &NewFormat.code, &nNewChannel)) {
      // ERROR: bad format
      return FALSE;
    }

    if (NewFormat.code == m_format.code) {
      // same format
      return TRUE;
    }

    int nChannelMapTab[4] = { -1, -1, -1, -1 }; // 新通道在旧通道上的索引

    for (int i = 0; i < nNewChannel; i++)
    {
      nChannelMapTab[i] = IntGetChanelIndex(m_format, m_channel, NewFormat.name[i]);
    }

    if (nChannelMapTab[0] < 0 && nChannelMapTab[1] < 0 && nChannelMapTab[2] < 0 && nChannelMapTab[3] < 0) {
      return FALSE;
    }

    int nNewPitch = MIN_PITCH_PARAM(m_width, nNewChannel, m_depth);
    if(nNewChannel <= m_channel)
    {
      switch (m_depth)
      {
      case 8:
        ChangeFormat<u8>(nChannelMapTab, nNewChannel, m_ptr, nNewPitch);
        break;
      case 16:
        ChangeFormat<u16>(nChannelMapTab, nNewChannel, m_ptr, nNewPitch);
        break;
      case 32:
        ChangeFormat<u32>(nChannelMapTab, nNewChannel, m_ptr, nNewPitch);
        break;
      default:
        break;
      }
    }
    else {
      CLBREAK; // 暂时不支持
    }

    m_pitch = nNewPitch;
    m_channel = nNewChannel;
    m_format.code = NewFormat.code;

    return TRUE;
  }

  //////////////////////////////////////////////////////////////////////////

  b32 ImageFilterF::Set( int nWidth, int nHeight, const char* fmt, const void* pData )
  {
    return Image::Set(nWidth, nHeight, fmt, 0, 32, pData);
  }

  b32 ImageFilterF::Set( ImageFilterI8* pSrcImage, float fLevel /*= (1.0f / 255.0f)*/ )
  {
    if( ! Image::Set(pSrcImage->GetWidth(), pSrcImage->GetHeight(), pSrcImage->GetFormat(), 0, 32, NULL)) {
      return FALSE;
    }

    float* pDestData;
    u8* pSrcData;

    if(m_channel == 1)
    {
      for(int y = 0; y < m_height; y++)
      {
        pDestData = (float*)GetLine(y);
        pSrcData = (u8*)(pSrcImage->GetLine(y));
        for(int x = 0; x < m_width; x++)
        {
          *pDestData++ = (float)*pSrcData * fLevel;
          pSrcData++;
        }
      }
    }
    else {
      for(int y = 0; y < m_height; y++)
      {
        pDestData = (float*)GetLine(y);
        pSrcData = (u8*)(pSrcImage->GetLine(y));
        for(int x = 0; x < m_width; x++)
        {
          for(int n = 0; n < m_channel; n++) {
            *pDestData++ = (float)*pSrcData * fLevel;
            pSrcData++;
          }
        }
      }
    }
    return TRUE;
  }

  b32 ImageFilterF::Clear( float value, char chChannel )
  {
    int nOffset = GetChannelOffset(chChannel);
    if(nOffset < 0) {
      return FALSE;
    }

    const int nPixelSize = GETPIXELSIZE;
    for(int y = 0; y < m_height; y++)
    {
      float* pData = (float*)((CLLPBYTE)GetLine(y) + nOffset);
      for(int x = 0; x < m_width; x++)
      {
        *pData = value;
        pData = (float*)((CLLPBYTE)pData + nPixelSize);
      }
    }
    return TRUE;
  }

} // namespace clstd