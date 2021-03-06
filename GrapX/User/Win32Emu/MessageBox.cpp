﻿#ifndef _DEV_DISABLE_UI_CODE
// 全局头文件
#include <GrapX.h>
#include <User/GrapX.Hxx>

#include <Smart/smartstream.h>
#include <clTokens.h>
#include <clStock.h>

// 平台相关
//#include "GrapX/GUnknown.h"
#include "GrapX/GResource.h"
#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"
#include "GrapX/DataInfrastructure.h"
#include "clPathFile.h"

// 私有头文件
#include "GrapX/GXUser.h"
#include "GrapX/GXGDI.h"
#include "GrapX/GXKernel.h"
#include "User/UILayoutMgr.h"
#include "GrapX/gxDevice.h"

extern GXWNDCLASSEX WndClassEx_Menu;
extern GXWNDCLASSEX WndClassEx_MyButton;
extern GXWNDCLASSEX WndClassEx_MyEdit;
extern GXWNDCLASSEX WndClassEx_MyEdit_1_3_30;
extern GXWNDCLASSEX WndClassEx_MyListbox;
extern GXWNDCLASSEX WndClassEx_MyStatic;
extern GXWNDCLASSEX WndClassEx_GXUIEdit_1_3_30;
extern GXBOOL LoadMenuTemplateFromStockW(clStockW* pSmart, GXLPCWSTR szName, clBuffer* pBuffer);
//struct DLGLOG
//{
//  typedef clmap<clStringW, GXHWND>  NameToWndDict;
//
//  size_t        cbSize;
//  NameToWndDict CtrlItemDict;
//
//  GXBOOL AddItem(GXLPCWSTR szName, GXHWND hWnd)
//  {
//    if(szName != NULL && szName[0] != '\0')
//    {
//      NameToWndDict::iterator it = CtrlItemDict.find(szName);
//      if(it != CtrlItemDict.end())
//      {
//        CtrlItemDict[szName] = hWnd;
//        return TRUE;
//      }
//    }
//    return FALSE;
//  }
//  GXHWND GetItem(GXLPCWSTR szName) const
//  {
//    if(szName != NULL && szName[0] != '\0')
//    {
//      NameToWndDict::const_iterator it = CtrlItemDict.find(szName);
//      if(it != CtrlItemDict.end())
//      {
//        return it->second;
//      }
//    }
//    return NULL;
//  }
//};

//////////////////////////////////////////////////////////////////////////
struct MESSAGE_TEXT
{
  GXLPCWSTR szText;
  GXLPCWSTR szCaption;
  GXLPCWSTR szBtnText[3];
  GXUINT szBtnID[3];
};

GXINT_PTR GXCALLBACK IntMessageBoxDlgProc(GXHWND hWnd, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
{
  switch(message)
  {
  case GXWM_COMMAND:
    {
      GXUINT_PTR result = gxGetWindowLong((GXHWND)lParam, GXGWL_USERDATA);
      gxEndDialog(hWnd, (int)result);
      //GXLPCWSTR szName = (GXLPCWSTR)gxSendMessageW((GXHWND)lParam, GXWM_GETIDNAMEW, NULL, NULL);
      //if(GXSTRCMP(szName, L"Ok") == 0) {
      //  gxEndDialog(hWnd, IDOK);
      //}
      //else if(GXSTRCMP(szName, L"Cancel") == 0) {
      //  gxEndDialog(hWnd, IDCANCEL);
      //}
      //else if(GXSTRCMP(szName, L"Yes") == 0) {
      //  gxEndDialog(hWnd, IDYES);
      //}
      //else if(GXSTRCMP(szName, L"No") == 0) {
      //  gxEndDialog(hWnd, IDNO);
      //}
    }
    break;

  case GXWM_INITDIALOG:
    {
      MESSAGE_TEXT* lpText = (MESSAGE_TEXT*)lParam;
      gxSetWindowTextW(hWnd, lpText->szCaption);
      GXHWND hMsgWnd = GXGetDlgItemByName(hWnd, _CLTEXT("Message"));
      gxSetWindowTextW(hMsgWnd, lpText->szText);

      GXHWND hBtnWnd;
      GXWCHAR s_BtnName[] = _CLTEXT("Btn0");
      for(int i = 0; i < 3; i++)
      {
        if(lpText->szBtnText[i]) {
          s_BtnName[3] = '0' + i;
          hBtnWnd = GXGetDlgItemByName(hWnd, s_BtnName);
          gxSetWindowTextW(hBtnWnd, lpText->szBtnText[i]);
          gxSetWindowLong(hBtnWnd, GXGWL_USERDATA, lpText->szBtnID[i]);
        }
        else break;
      }
    }
    break;
  }
  return FALSE;
}

int GXDLLAPI gxMessageBoxW(
  GXHWND    hWnd,       // handle of owner window
  GXLPCWSTR lpText,     // address of text in message box
  GXLPCWSTR lpCaption,  // address of title of message box  
  GXUINT    uType       // style of message box
  )
{
  MESSAGE_TEXT sText = {lpText, lpCaption, 0};
  GXUINT uBtnType = uType & 0xf;

  if(uBtnType == GXMB_OK)
  {
    sText.szBtnText[0] = _CLTEXT("OK");
    sText.szBtnID[0] = IDOK;
    return gxDialogBoxParamW(NULL, _CLTEXT("file://elements/MessageBox_Btn_1.txt"), hWnd, IntMessageBoxDlgProc, (GXLPARAM)&sText);
  }
  else if(uBtnType == GXMB_OKCANCEL || uBtnType == GXMB_YESNO || uBtnType == GXMB_RETRYCANCEL)
  {
    if(uBtnType == GXMB_OKCANCEL) {
      sText.szBtnText[0] = _CLTEXT("OK");
      sText.szBtnID[0] = IDOK;
      sText.szBtnText[1] = _CLTEXT("Cancel");
      sText.szBtnID[1] = IDCANCEL;
    }
    else if(uBtnType == GXMB_YESNO) {
      sText.szBtnText[0] = _CLTEXT("Yes");
      sText.szBtnID[0] = IDYES;
      sText.szBtnText[1] = _CLTEXT("No");
      sText.szBtnID[1] = IDNO;
    }
    else if(uBtnType == GXMB_RETRYCANCEL) {
      sText.szBtnText[0] = _CLTEXT("Retry");
      sText.szBtnID[0] = IDRETRY;
      sText.szBtnText[1] = _CLTEXT("Cancel");
      sText.szBtnID[1] = IDCANCEL;
    }
    return gxDialogBoxParamW(NULL, _CLTEXT("file://elements/MessageBox_Btn_2.txt"), hWnd, IntMessageBoxDlgProc, (GXLPARAM)&sText);
  }
  else if(uBtnType == GXMB_ABORTRETRYIGNORE || uBtnType == GXMB_YESNOCANCEL)
  {
    if(uBtnType == GXMB_ABORTRETRYIGNORE) {
      sText.szBtnText[0] = _CLTEXT("Abort");
      sText.szBtnID[0] = IDABORT;
      sText.szBtnText[1] = _CLTEXT("Retry");
      sText.szBtnID[1] = IDRETRY;
      sText.szBtnText[2] = _CLTEXT("Ignore");
      sText.szBtnID[2] = IDIGNORE;
    }
    else if(uBtnType == GXMB_YESNOCANCEL) {
      sText.szBtnText[0] = _CLTEXT("Yes");
      sText.szBtnID[0] = IDYES;
      sText.szBtnText[1] = _CLTEXT("No");
      sText.szBtnID[1] = IDNO;
      sText.szBtnText[2] = _CLTEXT("Cancel");
      sText.szBtnID[2] = IDCANCEL;
    }
    return gxDialogBoxParamW(NULL, _CLTEXT("file://elements/MessageBox_Btn_3.txt"), hWnd, IntMessageBoxDlgProc, (GXLPARAM)&sText);
  }
  return -1;
  //gxIntCreateDialogFromFileW(NULL, , _T("Dialog"), hWnd, , );
  //return MB_OK;
}

//////////////////////////////////////////////////////////////////////////

#endif // _DEV_DISABLE_UI_CODE