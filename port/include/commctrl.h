// <commctrl.h> subset: the ListView messages and structures the network lobby dialog uses.
#pragma once
#include <windows.h>

#define LVM_FIRST 0x1000
#define LVM_GETITEMCOUNT (LVM_FIRST + 4)
#define LVM_INSERTITEMA (LVM_FIRST + 7)
#define LVM_DELETEITEM (LVM_FIRST + 8)
#define LVM_DELETEALLITEMS (LVM_FIRST + 9)
#define LVM_GETNEXTITEM (LVM_FIRST + 12)
#define LVM_FINDITEMA (LVM_FIRST + 13)
#define LVM_INSERTCOLUMNA (LVM_FIRST + 27)
#define LVM_SETCOLUMNWIDTH (LVM_FIRST + 30)
#define LVM_SETITEMTEXTA (LVM_FIRST + 46)
#define LVM_GETITEMTEXTA (LVM_FIRST + 45)
#define LVIF_TEXT 0x0001
#define LVIF_PARAM 0x0004
#define LVIF_STATE 0x0008
#define LVNI_ALL 0x0000
#define LVNI_SELECTED 0x0002
#define LVFI_PARAM 0x0001
#define LVFI_STRING 0x0002
#define LVCF_FMT 0x0001
#define LVCF_WIDTH 0x0002
#define LVCF_TEXT 0x0004
#define LVCF_SUBITEM 0x0008
#define LVSCW_AUTOSIZE -1
#define LVSCW_AUTOSIZE_USEHEADER -2

typedef struct tagLVITEMA
{
    UINT mask;
    int iItem;
    int iSubItem;
    UINT state;
    UINT stateMask;
    LPSTR pszText;
    int cchTextMax;
    int iImage;
    LPARAM lParam;
    int iIndent;
} LVITEMA, *LPLVITEMA, LV_ITEMA;

typedef struct tagLVFINDINFOA
{
    UINT flags;
    LPCSTR psz;
    LPARAM lParam;
    POINT pt;
    UINT vkDirection;
} LVFINDINFOA, *LPFINDINFOA;

typedef struct tagLVCOLUMNA
{
    UINT mask;
    int fmt;
    int cx;
    LPSTR pszText;
    int cchTextMax;
    int iSubItem;
    int iImage;
    int iOrder;
} LVCOLUMNA, *LPLVCOLUMNA, LV_COLUMNA;

#ifdef __cplusplus
extern "C" {
#endif
void InitCommonControls(void);
#ifdef __cplusplus
}
#endif
