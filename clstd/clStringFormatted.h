#ifndef _STRING_FORMATTED_H_
#define _STRING_FORMATTED_H_

namespace clstd
{
  template<typename _TCh, class _Traits, class _TStr>
  class StringFormatted : public _TStr
  {
    const static int MAX_DIGITS = 80;

  public:
    StringFormatted& VarFormat(const _TCh *pFmt, va_list arglist)  // 在原始内容后面，使用变参列表追加格式化字符串
    {
      const _TCh* ptr = pFmt;
      _TCh        buffer[MAX_DIGITS + 1];  // 用来作为数字转换的缓冲区,对于32位整数和浮点数,转换为字符串后长度都不大于80
      int         i;


      while(*ptr != '\0')
      {
        const _TCh* ptr2 = _Traits::StringSearchChar(ptr, '%');
        if(ptr2 == NULL)
        {
          _TStr::Append(ptr);
          break;
        }
        else
        {
          // %[flags][width][.precision][length]specifier
          int nWidth = 0;
          int nPrecision = 0;
          int nLong = 0;
          b32 bLeftAlign = FALSE;  // '-' 左对齐
          b32 bZeroPrefix = FALSE; // '0' 不足位用0填充, 比'-'优先级低
          b32 bForceSign = FALSE;  // '+' 强制显示符号
          b32 bSpace = FALSE;      // ' ' 符号位占位，与'+'同时出现时比'+'优先级低
          b32 bPound = FALSE;      // '#' 显示八进制或者十六进制前缀
          b32 bPrecision = FALSE;  // 遇到'.'之后为True
          _TStr::Append(ptr, ptr2 - ptr);
          ptr = ptr2 + 1;
        SEQUENCE:
          switch(*ptr)
          {
          case '\0':
            goto FUNC_RET;
          case '%':
            _TStr::Append((_TCh)'%');
            break;
          case 'l':
            ptr++;
            if(nLong < 2) {
              nLong++;
              goto SEQUENCE;
            }
            break;
          case 's':
            _TStr::Append((_TCh*)va_arg(arglist, _TCh*), bZeroPrefix && nWidth > 0 ? '0' : ' ', nWidth);
            break;
          case 'c':
            _TStr::Append((_TCh)va_arg(arglist, int/*_TCh*/));
            break;

          case 'd':
          case 'i':
            if(nLong == 2) { // long long(64bits)
              i64 va_value = va_arg(arglist, i64);

              if(va_value >= 0 && bForceSign) {
                buffer[0] = '+';
                _Traits::Integer64ToString(buffer + 1, MAX_DIGITS - 1, va_value, 0);
              }
              else if(va_value >= 0 && bSpace) {
                buffer[0] = 0x20;
                _Traits::Integer64ToString(buffer + 1, MAX_DIGITS - 1, va_value, 0);
              }
              else {
                _Traits::Integer64ToString(buffer, MAX_DIGITS, va_value, 0);
              }

            }
            else { // int(32bits)
              int va_value = va_arg(arglist, int);

              if(va_value >= 0 && bForceSign) {
                buffer[0] = '+';
                _Traits::Integer32ToString(buffer + 1, MAX_DIGITS - 1, va_value, 0);
              }
              else if(va_value >= 0 && bSpace) {
                buffer[0] = 0x20;
                _Traits::Integer32ToString(buffer + 1, MAX_DIGITS - 1, va_value, 0);
              }
              else {
                _Traits::Integer32ToString(buffer, MAX_DIGITS, va_value, 0);
              }
            }

            ASSERT(buffer[0] == '+' || buffer[0] == '-' || buffer[0] == 0x20 ||
              (buffer[0] >= '0' && buffer[0] <= '9'));

            if(bPrecision)
            {
              // "%5.3d", 1  = "  001"
              // "%3.5d", 1  = "00001"
              // "%5.3d", -1 = " -001"
              // "%3.5d", -1 = "-00001"

              if(buffer[0] == '+' || buffer[0] == '-' || buffer[0] == 0x20) {
                this->_AppendFormat(buffer, 1, buffer + 1, nWidth, nPrecision);
              }
              else {
                this->_AppendFormat(buffer, nWidth, nPrecision);
              }
            }
            else
            {
              if(bZeroPrefix && nWidth > 0)
              {
                if(buffer[0] == '+' || buffer[0] == '-' || buffer[0] == 0x20) {
                  _TStr::Append(buffer[0]);
                  _TStr::Append(buffer + 1, '0', nWidth - 1);
                }
                else {
                  _TStr::Append(buffer, '0', nWidth);
                }
              }
              else
              {
                _TStr::Append(buffer, ' ', nWidth);
              }
            }
            break;

          case 'o':
          {
            unsigned long va_value = va_arg(arglist, unsigned long);
            if(bPound && va_value) {
              buffer[0] = '0'; // 进制前缀
              _Traits::OctalToString(buffer + 1, MAX_DIGITS - 1, va_value);
            }
            else {
              _Traits::OctalToString(buffer, MAX_DIGITS, va_value);
            }

            if(bPrecision) {
              _AppendFormat(buffer, nWidth, nPrecision);
            }
            else {
              _TStr::Append(buffer, bZeroPrefix && nWidth > 0 ? '0' : ' ', nWidth);
            }
            break;
          }
          case 'u':
          {
            if(nLong == 2) {
              _Traits::Unsigned64ToString(buffer, MAX_DIGITS, va_arg(arglist, u64), 0);
            }
            else {
              _Traits::Unsigned32ToString(buffer, MAX_DIGITS, va_arg(arglist, unsigned long), 0);
            }

            if(bPrecision) {
              _AppendFormat(buffer, nWidth, nPrecision);
            }
            else {
              _TStr::Append(buffer, bZeroPrefix && nWidth > 0 ? '0' : ' ', nWidth);
            }
            break;
          }

          case 'f':
          case 'F':
          {
            double va_value = va_arg(arglist, double);
            int avail = 0;
            //if(nPrecision)
            {
              if(bForceSign && *(i64*)&va_value >= 0) {
                buffer[0] = '+';
                avail = 1 + _Traits::FloatToString(buffer + 1, MAX_DIGITS - 1, nPrecision, (float)va_value, 'F');
              }
              else if(bSpace && *(i64*)&va_value >= 0) {
                buffer[0] = 0x20;
                avail = 1 + _Traits::FloatToString(buffer + 1, MAX_DIGITS - 1, nPrecision, (float)va_value, 'F');
              }
              else {
                avail = _Traits::FloatToString(buffer, MAX_DIGITS, nPrecision, (float)va_value, 'F');
              }

              //const _TCh* pDot = _Traits::StringSearchChar(buffer, '.');
              //if(pDot != NULL) {
              //  int nn = nPrecision + 1; // 包含'.'的个数
              //  while(nn-- && *++pDot != '\0'); // 没错，就是分号！
              //  *(_TCh*)pDot = '\0';
              //}
            }

            if(!bPound && buffer[avail - 1] == '.') {
              buffer[avail - 1] = '\0';
            }
            //else
            //{
            //  if(bForceSign && va_value >= 0) {
            //    buffer[0] = '+';
            //    _Traits::FloatToString(buffer + 1, MAX_DIGITS - 1, 0, (float)va_value, 'F');
            //  }
            //  else {
            //    _Traits::FloatToString(buffer, MAX_DIGITS, 0, (float)va_value, 'F');
            //  }
            //}
            _TStr::Append(buffer);
            break;
          }

          case 'b':
            _Traits::BinaryToString(buffer, MAX_DIGITS, va_arg(arglist, unsigned long));
            _TStr::Append(buffer, bZeroPrefix && nWidth > 0 ? '0' : ' ', nWidth);
            break;

          case 'X':
          case 'x':
          {
            unsigned long va_value = va_arg(arglist, unsigned long);

            if(va_value)
            {
              buffer[0] = '0'; buffer[1] = *ptr;
              if(*ptr == 'X') {
                _Traits::HexToUpperString(buffer + 2, MAX_DIGITS - 2, va_value);
              }
              else {
                _Traits::HexToLowerString(buffer + 2, MAX_DIGITS - 2, va_value);
              }

              if(bPrecision)
              {
                if(bPound) {
                  _AppendFormat(buffer, 2, buffer + 2, nWidth, nPrecision);
                }
                else {
                  _AppendFormat(buffer + 2, nWidth, nPrecision);
                }
              }
              else if(bPound && bZeroPrefix)
              {
                _TStr::Append(buffer, 2);

                // 因为前面加了前缀，如果设定了宽度，这里要把宽度缩短两个字符
                if(nWidth > 2) {
                  _TStr::Append(buffer + 2, '0', nWidth - 2);
                }
                else if(nWidth < -2) {
                  _TStr::Append(buffer + 2, 0x20, nWidth + 2);
                }
                else {
                  _TStr::Append(buffer + 2);
                }
              }
              else if(bPound)
              {
                _TStr::Append(buffer, 0x20, nWidth);
              }
              else if(bZeroPrefix)
              {
                _TStr::Append(buffer + 2, (nWidth > 0 ? '0' : 0x20), nWidth);
              }
              else {
                _TStr::Append(buffer + 2, 0x20, nWidth);
              }
            }
            else
            {
              if(bPrecision)
              {
                if(nWidth > nPrecision) {
                  _TStr::Append(0x20, nWidth - nPrecision);
                }

                _TStr::Append('0', nPrecision);

                if(-nWidth > nPrecision) {
                  _TStr::Append(0x20, -nWidth - nPrecision);
                }
              }
              else
              {
                // 忽略 bZeroPerfix
                if(nWidth) {
                  buffer[0] = '0'; buffer[1] = '\0';
                  _TStr::Append(buffer, (bZeroPrefix && nWidth > 0 ? '0' : 0x20), nWidth);
                }
                else {
                  _TStr::Append('0', nWidth < 0 ? -nWidth : nWidth); // abs(nWidth)
                }
              }
            }
            break;
          }

          case '*':
            if(bPrecision) {
              nPrecision = (int)va_arg(arglist, int);
            }
            else {
              nWidth = (int)va_arg(arglist, int);
            }
            ptr++;
            goto SEQUENCE;

          case '0':
            bZeroPrefix = TRUE;
            ptr++;
            goto SEQUENCE;

          case '+':
            bForceSign = TRUE;
            ptr++;
            goto SEQUENCE;

          case '-':
            bLeftAlign = TRUE;
            ptr++;
            goto SEQUENCE;

          case '#':
            bPound = TRUE;
            ptr++;
            goto SEQUENCE;

          case 0x20: // space
            bSpace = TRUE;
            ptr++;
            goto SEQUENCE;

          case '.':  // "%.3f"
            bPrecision = TRUE;
            ptr++;
            goto SEQUENCE;

          default:
            if(*ptr >= '0' && *ptr <= '9')  // "%8d"
            {
              i = 0;
              while(1)
              {
                if(*ptr >= '0' && *ptr <= '9') {
                  buffer[i++] = *ptr;
                }
                else if(*ptr == '\0') {
                  goto FUNC_RET;
                }
                else if(i >= sizeof(buffer)) {
                  break;
                }
                else if(*ptr == 'd' || *ptr == 'i' || *ptr == 'u' || *ptr == 'o' || *ptr == 'X' || *ptr == 'x' || *ptr == '.' || *ptr == 'f')
                {
                  buffer[i] = '\0';

                  if(bPrecision) {
                    nPrecision = _Traits::StringToInteger32(buffer);
                  }
                  else {
                    nWidth = _Traits::StringToInteger32(buffer);
                    if(bLeftAlign) {
                      nWidth = -nWidth;
                    }
                  }

                  //if(*ptr != 'f' && nWidth == 0 && nPrecision != 0) {
                  //  bLeftAlign = FALSE;
                  //  bZeroPrefix = TRUE; // 例如 "%.5d" 要求按照5位填0扩充
                  //  nWidth = nPrecision;
                  //}

                  goto SEQUENCE;
                }
                else
                  break;
                ptr++;
              }
            } // if
            break;
          } // switch
        }
        ptr++;
      }

    FUNC_RET:
      va_end(arglist);
      return *this;
    }

    //////////////////////////////////////////////////////////////////////////

    void _AppendSpace(int len, int nPrefixLen, int nWidth, int nPrecision)
    {
      if(nWidth > nPrecision)
      {
        if(nPrecision > len) {
          if(nWidth - nPrecision > nPrefixLen) {
            _TStr::Append(0x20, nWidth - nPrecision - nPrefixLen);
          }
        }
        else if(nWidth > len) {
          if(nWidth - len > nPrefixLen) {
            _TStr::Append(0x20, nWidth - len - nPrefixLen);
          }
        }
      }
    }

    //************************************
    // Method:    _AppendFormat
    // Qualifier:
    // Parameter: szPrefix      前缀符号，比如'+'，'-'，'0x'
    // Parameter: nPrefixLen    前缀长度
    // Parameter: szNumeric     数字字符
    // Parameter: nWidth        正值数字右对齐，负值左对齐
    // Parameter: nPrecision
    //************************************
    void _AppendFormat(const _TCh* szPrefix, int nPrefixLen, const _TCh* szNumeric, int nWidth, int nPrecision)
    {
      int len = (int)_Traits::StringLength(szNumeric);
      _AppendSpace(len, nPrefixLen, nWidth, nPrecision);
      _TStr::Append(szPrefix, nPrefixLen);
      _TStr::Append(szNumeric, '0', nPrecision);
      _AppendSpace(len, nPrefixLen, -nWidth, nPrecision);
    }

    //////////////////////////////////////////////////////////////////////////

    void _AppendSpace(int len, int nWidth, int nPrecision)
    {
      if(nWidth > nPrecision)
      {
        if(nPrecision > len) {
          _TStr::Append(0x20, nWidth - nPrecision);
        }
        else if(nWidth > len) {
          _TStr::Append(0x20, nWidth - len);
        }
      }
    }

    void _AppendFormat(const _TCh* szNumeric, int nWidth, int nPrecision)
    {
      int len = (int)_Traits::StringLength(szNumeric);

      _AppendSpace(len, nWidth, nPrecision);
      _TStr::Append(szNumeric, '0', nPrecision);
      _AppendSpace(len, -nWidth, nPrecision);
    }
  };

} // namespace clstd

#endif // _STRING_FORMATTED_H_