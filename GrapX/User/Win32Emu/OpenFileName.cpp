﻿#include <GrapX.h>
//#include "GrapX/GUnknown.h"
#include "clPathFile.h"
#include "smart/smartstream.h"
#include <clTokens.h>
#include "clStock.h"
#include "GrapX/MOLogger.h"
#include "GrapX/GXUser.h"
#include "GrapX/DataPool.h"
#include "GrapX/DataPoolVariable.h"
#include "GrapX/DataInfrastructure.h"


void GetPathFromItem(const GXHWND hTree, const GXHTREEITEM hItem, clStringW& strDir)
{
  GXWCHAR buffer[MAX_PATH + 1] = _CLTEXT("\\");
  GXTVITEM tvi;
  GXHTREEITEM hCurItem = hItem, hParent;
  memset(&tvi, 0, sizeof(tvi));
  strDir.Clear();
  GXWCHAR szDriverName[] = _CLTEXT("*:");

  while(1)
  {
    hParent = (GXHTREEITEM)gxSendMessage(hTree, GXTVM_GETNEXTITEM, GXTVGN_PARENT, (LPARAM)hCurItem);
    if(hParent != NULL)
    {
      tvi.hItem = hCurItem;
      tvi.mask = GXTVIF_TEXT;
      tvi.pszText = &buffer[1];
      tvi.cchTextMax = MAX_PATH;
      gxSendMessage(hTree, GXTVM_GETITEM, 0, (LPARAM)&tvi);
      //lstrcat(buffer, _T("\\"));
      strDir.Insert(0, buffer);
      hCurItem = hParent;
      continue;
    }
    else
    {
      tvi.hItem = hCurItem;
      tvi.mask = GXTVIF_PARAM;
      gxSendMessage(hTree, GXTVM_GETITEM, 0, (LPARAM)&tvi);
      szDriverName[0] = _CLTEXT('A') + (int)tvi.lParam;
      strDir.Insert(0, szDriverName);
      break;
    }
  }
}

void PreFillItem(GXHWND hTree, GXHTREEITEM hParent)
{
  WIN32_FIND_DATAW ffd;
  clStringW strPath;
  GXTVINSERTSTRUCT tis;
  memset(&tis, 0, sizeof(tis));

  GetPathFromItem(hTree, hParent, strPath);
  strPath += _CLTEXT("\\*.*");

  HANDLE hFind = FindFirstFileW(reinterpret_cast<LPCWSTR>(strPath.CStr()), &ffd);
  if(hFind == INVALID_HANDLE_VALUE) {
    return;
  }

  do
  {
    if((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 && ffd.cFileName[0] != _CLTEXT('.'))
    {
      //LRESULT hr = SendMessage(hTree, TVM_GETISEARCHSTRING, 0, (LPARAM)&ffd.cFileName);
      //if(hr == 0)
      {
        tis.hParent      = hParent;
        tis.hInsertAfter = GXTVI_SORT;
        tis.u.item.mask    = GXTVIF_TEXT;
        tis.u.item.pszText = reinterpret_cast<GXLPWSTR>(ffd.cFileName);
        gxSendMessage(hTree, GXTVM_INSERTITEM, 0, (LPARAM)&tis);
      }
    }
  } while (FindNextFileW(hFind, &ffd));
  FindClose(hFind);
}

//TODO: 如果子项目展开,则也填充它的子项目
void FillItem(GXHWND hTree, GXHTREEITEM hParent)
{
  GXHTREEITEM hItem = (GXHTREEITEM)gxSendMessage(hTree, GXTVM_GETNEXTITEM, GXTVGN_CHILD, (LPARAM)hParent);
  while(hItem != 0)
  {
    PreFillItem(hTree, hItem);
    hItem = (GXHTREEITEM)gxSendMessage(hTree, GXTVM_GETNEXTITEM, GXTVGN_NEXT, (LPARAM)hItem);
  }
}

void DeleteItem(GXHWND hTree, GXHTREEITEM hParent)
{
  GXTVITEM tvi;
  //GXWCHAR buffer[1024];
  memset(&tvi, 0, sizeof(tvi));
  tvi.mask = GXTVIF_STATE;
  tvi.stateMask = TVIS_EXPANDED;
  GXHTREEITEM hItem = (GXHTREEITEM)gxSendMessage(hTree, GXTVM_GETNEXTITEM, GXTVGN_CHILD, (LPARAM)hParent);

  while(hItem != 0)
  {
    GXHTREEITEM hChildItem = (GXHTREEITEM)gxSendMessage(hTree, GXTVM_GETNEXTITEM, GXTVGN_CHILD, (LPARAM)hItem);

    while(hChildItem != 0)
    {
      GXHTREEITEM hNextChildItem = (GXHTREEITEM)gxSendMessage(hTree, GXTVM_GETNEXTITEM, GXTVGN_NEXT, (LPARAM)hChildItem);
      gxSendMessage(hTree, GXTVM_DELETEITEM, 0, (LPARAM)hChildItem);
      hChildItem = hNextChildItem;
    }
    tvi.hItem = hItem;
    tvi.state &= (~TVIS_EXPANDED);
    gxSendMessage(hTree, GXTVM_SETITEM, 0, (LPARAM)&tvi);

    hItem = (GXHTREEITEM)gxSendMessage(hTree, GXTVM_GETNEXTITEM, GXTVGN_NEXT, (LPARAM)hItem);
  }
}

void InitTree(GXHWND hWnd)
{
  GXHWND hTree = GXGetDlgItemByName(hWnd, _CLTEXT("Directory"));
  DWORD dwDriverMask = GetLogicalDrives();
  GXWCHAR szVolume[64];
  GXWCHAR szBuffer[512];

  GXWCHAR szPathName[] = _CLTEXT("A:\\");
  GXTVINSERTSTRUCT tis;
  memset(&tis, 0, sizeof(tis));

  for(int i = 0; i < 26; i++)
  {
    if(((1 << i) & dwDriverMask) != 0)
    {
      szPathName[0] = i + _CLTEXT('A');
      UINT nType = GetDriveTypeW(reinterpret_cast<LPCWSTR>(szPathName));

      szVolume[0] = _CLTEXT('\0');
      DWORD dwFileSystemFlag;
      GXWCHAR szFilesystem[32];
      szFilesystem[0] = '\0';
      GetVolumeInformationW(reinterpret_cast<LPCWSTR>(szPathName), reinterpret_cast<LPWSTR>(szVolume), 64, NULL, NULL, &dwFileSystemFlag, reinterpret_cast<LPWSTR>(szFilesystem), sizeof(szFilesystem) / sizeof(szFilesystem[0]));

      static GXWCHAR *aType[] = {
        _CLTEXT("Unknown"), _CLTEXT("NoRootDir"), _CLTEXT("Removable"), _CLTEXT("Fixed"), _CLTEXT("Remote"), _CLTEXT("CD-ROM"), _CLTEXT("RamDisk")};

        if(szVolume[0] != _CLTEXT('\0'))
        {
          wsprintfW(reinterpret_cast<LPWSTR>(szBuffer), _T("%s(%c:)"), szVolume, _T('A') + i);
        }
        else
        {
          //FILE_NAMED_STREAMS
          wsprintfW(reinterpret_cast<LPWSTR>(szBuffer), _T("%s(%c:)"), aType[nType], _T('A') + i);
        }

        tis.hParent       = NULL;
        tis.hInsertAfter  = GXTVI_ROOT;
        tis.u.item.mask     = GXTVIF_TEXT | GXTVIF_PARAM;
        tis.u.item.pszText  = szBuffer;
        tis.u.item.lParam   = i;

        GXHTREEITEM hItem = (GXHTREEITEM)gxSendMessage(hTree, GXTVM_INSERTITEM , 0, (GXLPARAM)&tis);

        PreFillItem(hTree, hItem);
    }
  }
}
void InitListView(GXHWND hWnd)
{
  GXHWND hListView = GXGetDlgItemByName(hWnd, _CLTEXT("FileList"));
  GXLVCOLUMN lvc;
  memset(&lvc, 0, sizeof(lvc));
  lvc.mask = LVCF_TEXT;
  lvc.fmt = LVCFMT_LEFT;
  lvc.pszText = _CLTEXT("文件名");

  gxSendMessage(hListView, GXLVM_INSERTCOLUMN, 0, (GXLPARAM)&lvc);
}

void FillFile(GXHWND hWnd, GXHTREEITEM hTreeItem)
{
  clStringW strPath;
  GXHWND hListView = GXGetDlgItemByName(hWnd, _CLTEXT("FileList"));
  GXHWND hTree = GXGetDlgItemByName(hWnd, _CLTEXT("Directory"));
  GetPathFromItem(hTree, hTreeItem, strPath);
  strPath += _CLTEXT("\\*.*");

  gxSendMessage(hListView, GXLVM_DELETEALLITEMS, 0, 0);

  WIN32_FIND_DATAW ffd;
  HANDLE hFind = FindFirstFileW(reinterpret_cast<LPCWSTR>(strPath.CStr()), &ffd);
  if(hFind != INVALID_HANDLE_VALUE)
  {
    do{
      if((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
      {
        GXLVITEM lvi;
        memset(&lvi, 0, sizeof(lvi));
        lvi.mask = GXLVIF_TEXT;
        lvi.pszText = reinterpret_cast<GXLPWSTR>(ffd.cFileName);
        gxSendMessage(hListView, GXLVM_INSERTITEM, 0, (LPARAM)&lvi);
      }
    }while(FindNextFileW(hFind, &ffd));
    FindClose(hFind);
  }
}

GXINT_PTR GXCALLBACK IntBrowseFile(GXHWND hDlg, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
{
  UNREFERENCED_PARAMETER(lParam);
  switch (message)
  {
  case WM_INITDIALOG:
    //SendDlgItemMessage(hDlg, IDC_COMBOBOXEX1, CB_DIR, DDL_ARCHIVE, (LPARAM)_T("*.*"));
    //SendDlgItemMessage(hDlg, IDC_COMBO1, CB_DIR, DDL_ARCHIVE, (LPARAM)_T("*.*"));
    SetErrorMode(SEM_FAILCRITICALERRORS); // FindFirstFile 访问无磁盘的读卡器时会弹出MessageBox， 这个用来修改错误模式屏蔽掉
    InitTree(hDlg);
    InitListView(hDlg);
    return (INT_PTR)TRUE;

  case WM_NOTIFY:
    //CLBREAK; // 测试下面的wParam值
    //if(wParam == IDT_DIR) // FIXME: 修复这个判断
    {
      NMHDR* pnmh = (NMHDR*)lParam;
      if(pnmh->code == GXTVN_ITEMEXPANDING)
      {
        GXNMTREEVIEW* pnmtv = (GXNMTREEVIEW*)pnmh;
        //GXHWND hListView = GXGetDlgItemByName(hWnd, L"FileList");
        GXHWND hTree = GXGetDlgItemByName(hDlg, _CLTEXT("Directory"));

        if(pnmtv->action == TVE_EXPAND) {
          FillItem(hTree, pnmtv->itemNew.hItem);
        }
        else if(pnmtv->action == TVE_COLLAPSE) {
          DeleteItem(hTree, pnmtv->itemNew.hItem);
        }

      }
      else if(pnmh->code == GXTVN_SELCHANGED)
      {
        GXNMTREEVIEW* pnmtv = (GXNMTREEVIEW*)pnmh;
        FillFile(hDlg, pnmtv->itemNew.hItem);
      }
    }
    break;
  case WM_COMMAND:
    if (GXLOWORD(wParam) == IDOK || GXLOWORD(wParam) == IDCANCEL)
    {
      gxEndDialog(hDlg, LOWORD(wParam));
      return (GXINT_PTR)TRUE;
    }
    break;
  }
  return (GXINT_PTR)FALSE;
}

//////////////////////////////////////////////////////////////////////////
class CSimpleBrowseFile
{
  clStringW m_strPath;
  GXOPENFILENAMEW* m_lpofn;
public:
  const static GXLPARAM DriverSign = -1;

  CSimpleBrowseFile(GXOPENFILENAMEW* lpofn)
    : m_lpofn(lpofn)
  {
    ASSERT(lpofn->lpstrInitialDir);
    if(clpathfile::IsPathExist(lpofn->lpstrInitialDir))
    {
      m_strPath = lpofn->lpstrInitialDir;
    }
  }

  void FillDirvers(GXHWND hListView)
  {
    DWORD dwDriverMask = GetLogicalDrives();
    GXWCHAR szVolume[64];
    GXWCHAR szBuffer[512];

    GXWCHAR szPathName[] = _CLTEXT("A:\\");
    GXTVINSERTSTRUCT tis;
    memset(&tis, 0, sizeof(tis));

    for(int i = 0; i < 26; i++)
    {
      if(((1 << i) & dwDriverMask) != 0)
      {
        szPathName[0] = i + _CLTEXT('A');
        UINT nType = GetDriveTypeW(reinterpret_cast<LPCWSTR>(szPathName));

        szVolume[0] = _CLTEXT('\0');
        DWORD dwFileSystemFlag;
        GXWCHAR szFilesystem[32];
        szFilesystem[0] = '\0';
        GetVolumeInformationW(
          reinterpret_cast<LPCWSTR>(szPathName),
          reinterpret_cast<LPWSTR>(szVolume),
          64, NULL, NULL, &dwFileSystemFlag,
          reinterpret_cast<LPWSTR>(szFilesystem),
          sizeof(szFilesystem) / sizeof(szFilesystem[0]));

        static GXWCHAR *aType[] = {
          _CLTEXT("Unknown"), _CLTEXT("NoRootDir"), _CLTEXT("Removable"), _CLTEXT("Fixed"), _CLTEXT("Remote"), _CLTEXT("CD-ROM"), _CLTEXT("RamDisk")};

          if(szVolume[0] != _CLTEXT('\0')) {
            wsprintfW(reinterpret_cast<LPWSTR>(szBuffer), _T("%s (%c:)"), szVolume, _T('A') + i);
          }
          else {
            wsprintfW(reinterpret_cast<LPWSTR>(szBuffer), _T("%s (%c:)"), aType[nType], _T('A') + i);
          }

          int index = (int)gxSendMessage(hListView, GXLB_ADDSTRINGW, 0, (GXLPARAM)szBuffer);
          gxSendMessage(hListView, GXLB_SETITEMDATA, index, DriverSign);
      }
    }
  }

  void FillList(GXHWND hDlg)
  {
    GXHWND hListView = GXGetDlgItemByName(hDlg, _CLTEXT("FileList"));
    gxSendMessage(hListView, GXLB_RESETCONTENT, 0, 0);

    GXHWND hPath = GXGetDlgItemByName(hDlg, _CLTEXT("Path"));
    gxSetWindowTextW(hPath, m_strPath);

    if(m_strPath.EndsWith(_CLTEXT(":\\"))) {
      FillDirvers(hListView);
    }
    else if(m_strPath.IsEmpty()) {
      FillDirvers(hListView);
      return;
    }

    clstd::FindFile find(m_strPath + _CLTEXT("\\*"));
    clstd::FINDFILEDATAW ffd;
    clStringArrayW aFilter;

    GXLPCWSTR szFilter = m_lpofn->lpstrFilter;
    GXSIZE_T len = GXSTRLEN(szFilter);
    szFilter += (len + 1); // 有点临时

    clstd::ResolveString(clStringW(szFilter), ';', aFilter);
    std::for_each(aFilter.begin(), aFilter.end(), [](clStringW& str){
      str.MakeUpper();
    });

    while(find.GetFile(&ffd))
    {
      if(ffd.cFileName[0] == '.') {
        if(ffd.cFileName[1] == '\0') {
          continue;
        }
      }

      int index = -1;
      if(TEST_FLAG(ffd.dwAttributes, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) {
        continue;
      }

      if(TEST_FLAG(ffd.dwAttributes, FILE_ATTRIBUTE_DIRECTORY)) {
        index = (int)gxSendMessage(hListView, GXLB_ADDSTRINGW, NULL, (GXLPARAM)ffd.cFileName);
      }
      else {
        for(auto it = aFilter.begin(); it != aFilter.end(); ++it) {
          clStringW str((wch)0x20, 2);
          str.Append(ffd.cFileName);
          str.MakeUpper();
          if(clpathfile::MatchSpec(str, *it)) {
            index = (int)gxSendMessage(hListView, GXLB_ADDSTRINGW, NULL, (GXLPARAM)ffd.cFileName);
            break;
          }
        }
      }
      gxSendMessage(hListView, GXLB_SETITEMDATA, index, (GXLPARAM)ffd.dwAttributes);
    }
  }

  void TrueClose()
  {
    m_lpofn->lpstrInitialDir = m_strPath;
  }

  static GXINT_PTR GXCALLBACK IntSimpleBrowseFile(GXHWND hDlg, GXUINT message, GXWPARAM wParam, GXLPARAM lParam)
  {
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
      {
        //SendDlgItemMessage(hDlg, IDC_COMBOBOXEX1, CB_DIR, DDL_ARCHIVE, (LPARAM)_T("*.*"));
        //SendDlgItemMessage(hDlg, IDC_COMBO1, CB_DIR, DDL_ARCHIVE, (LPARAM)_T("*.*"));
        //GXOPENFILENAMEW* lpofn = (GXOPENFILENAMEW*)lParam;
        CSimpleBrowseFile* psbf = (CSimpleBrowseFile*)lParam;
        gxSetWindowLong(hDlg, GXGWL_USERDATA, lParam);
        gxSetWindowTextW(hDlg, psbf->m_lpofn->lpstrTitle);

        // FindFirstFile 访问无磁盘的读卡器时会弹出MessageBox， 这个用来修改错误模式屏蔽掉
        SetErrorMode(SEM_FAILCRITICALERRORS);
        psbf->FillList(hDlg);
      }
      return (INT_PTR)TRUE;

    case WM_NOTIFY:
      {
        GXNMHDR* pnmh = (GXNMHDR*)lParam;
      }
      break;
    case WM_COMMAND:
      {
        if(GXHIWORD(wParam) == GXLBN_DBLCLK)
        {
          GXHWND hListView = GXGetDlgItemByName(hDlg, _CLTEXT("FileList"));
          int nSel = (int)gxSendMessage(hListView, GXLB_GETCURSEL, 0, 0);

          if(nSel >= 0)
          {
            GXWCHAR szFile[MAX_PATH];
            gxSendMessage(hListView, GXLB_GETTEXT, nSel, (GXLPARAM)szFile);
            GXDWORD dwAttr = (GXDWORD)gxSendMessage(hListView, GXLB_GETITEMDATA, nSel, 0);

            CSimpleBrowseFile* psbf = (CSimpleBrowseFile*)gxGetWindowLong(hDlg, GXGWL_USERDATA);

            if(dwAttr == CSimpleBrowseFile::DriverSign)
            {
              size_t len = GXSTRLEN(szFile);
              psbf->m_strPath.Clear();
              psbf->m_strPath.Format(_CLTEXT("%c:\\"), szFile[len - 3]);
              psbf->FillList(hDlg);
            }
            else if(TEST_FLAG(dwAttr, FILE_ATTRIBUTE_DIRECTORY))
            {
              clpathfile::CombinePath(psbf->m_strPath, psbf->m_strPath, szFile);
              psbf->FillList(hDlg);
            }
            else
            {
              clStringW strFilePath;
              clpathfile::CombinePath(strFilePath, psbf->m_strPath, szFile);
              GXSTRCPYN(psbf->m_lpofn->lpstrFile, (GXLPCWSTR)strFilePath, psbf->m_lpofn->nMaxFile);
              psbf->TrueClose();
              gxEndDialog(hDlg, TRUE);
            }
          } // if(nSel >= 0)
        }
        else if(GXHIWORD(wParam) == GXBN_CLICKED)
        {
          GXLONG_PTR id = gxGetWindowLong((GXHWND)lParam, GXGWL_ID);
          if( ! IS_IDENTIFY(id))
          {
            GXLPCWSTR szName = (GXLPCWSTR)id;

            if(GXSTRCMP(szName, _CLTEXT("Open")) == 0) {
              CSimpleBrowseFile* psbf = (CSimpleBrowseFile*)gxGetWindowLong(hDlg, GXGWL_USERDATA);
              psbf->TrueClose();
              gxEndDialog(hDlg, TRUE);
            }
            else if(GXSTRCMP(szName, _CLTEXT("Cancel")) == 0) {
              gxEndDialog(hDlg, FALSE);
            }
          }
        }
      }
      break;
    }
    return (GXINT_PTR)FALSE;
  }
};


//

//////////////////////////////////////////////////////////////////////////
extern "C"
{
  GXBOOL GXDLLAPI gxGetSaveFileNameW(GXLPOPENFILENAMEW lpOFN)
  {
    return FALSE;
  }

  GXBOOL GXDLLAPI gxGetOpenFileNameW(GXLPOPENFILENAMEW lpOFN)
  {
    int result = 0;
    GXWCHAR szConfig[MAX_PATH];
    //GXWCHAR szInitDir[MAX_PATH];
    clStringW strInitDir;
    clstd::StockW stock;
    GXBOOL bConfig = FALSE;
    GXUIGetStationDesc(GXSD_CONFIGPATH, MAX_PATH, (GXLPARAM)szConfig);
    bConfig = stock.LoadFromFile(szConfig);

    // 初始化起始路径
    if( ! lpOFN->lpstrInitialDir) {
      if(bConfig) {
        clstd::StockW::Section sect = stock.OpenSection(NULL);
        strInitDir = sect.GetKeyAsString(_CLTEXT("GetOpenFileName"), _CLTEXT(""));
      }
      
      // 缺省
      if(strInitDir.IsEmpty()) {
        GXUIGetStationDesc(GXSD_ROOTDIR, MAX_PATH, (GXLPARAM)strInitDir.GetBuffer(MAX_PATH));
        strInitDir.ReleaseBuffer();
      }

      lpOFN->lpstrInitialDir = strInitDir;
    }

    if(TEST_FLAG(lpOFN->Flags, GXOFN_SIMPLEBROWSER)) {
      CSimpleBrowseFile sbf(lpOFN);
      result = gxDialogBoxParamW(NULL, _CLTEXT("@UI\\simplebrowser.dlg.txt"), lpOFN->hwndOwner,
        CSimpleBrowseFile::IntSimpleBrowseFile, (GXLPARAM)&sbf);
      strInitDir = lpOFN->lpstrInitialDir;
    }
    else {
      result = gxDialogBoxParamW(NULL, _CLTEXT("@UI\\browserfiles.dlg.txt"), lpOFN->hwndOwner, IntBrowseFile, (GXLPARAM)lpOFN);
    }

    // 保存起始路径
    if(result) {
      clstd::StockW::Section sect = stock.CreateSection(NULL);
      sect.SetKey(_CLTEXT("GetOpenFileName"), strInitDir);
      stock.SaveToFile(szConfig);
    }

    return result;
  }
}
