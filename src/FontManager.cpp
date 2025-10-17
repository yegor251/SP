#include "FontManager.h"
#include "Resource.h"
#include <tchar.h>
#include <richedit.h>

FontManager::FontManager() : hFont(nullptr), currentFont(FontChoice::DefaultConsolas) {
}

FontManager::~FontManager() {
    DestroyFont();
}

bool FontManager::CreateDefaultFont() {
    hFont = CreateFont(
        16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Consolas"
    );
    return hFont != nullptr;
}

void FontManager::DestroyFont() {
    if (hFont) {
        DeleteObject(hFont);
        hFont = nullptr;
    }
}

void FontManager::SetFontByMenuId(UINT id) {
    LOGFONT lf = {};
    lf.lfHeight = 14;
    lf.lfWeight = FW_NORMAL;
    wcscpy_s(lf.lfFaceName, LF_FACESIZE, L"Consolas");
    if (id == IDM_FONT_RASTER) {
        wcscpy_s(lf.lfFaceName, LF_FACESIZE, L"Courier New");
        lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
    } else if (id == IDM_FONT_VECTOR) {
        wcscpy_s(lf.lfFaceName, LF_FACESIZE, L"Arial");
        lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
    } else {
        wcscpy_s(lf.lfFaceName, LF_FACESIZE, L"Consolas");
        lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
    }

    if (id == IDM_FONT_RASTER) currentFont = FontChoice::RasterCourierNew;
    else if (id == IDM_FONT_VECTOR) currentFont = FontChoice::VectorArial;
    else currentFont = FontChoice::DefaultConsolas;
    
    ApplyCurrentFontToFocusedSelection();
}

void FontManager::ApplyCurrentFontToFocusedSelection() {
    HWND target = GetFocus();
    if (!target) {
        return;
    }
    const wchar_t* face = L"Consolas";
    if (currentFont == FontChoice::RasterCourierNew) face = L"Courier New";
    else if (currentFont == FontChoice::VectorArial) face = L"Arial";
    
    CHARFORMAT2 cf = {};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_FACE | CFM_SIZE | CFM_WEIGHT;
    cf.yHeight = 16 * 20;
    cf.wWeight = FW_NORMAL;
    wcscpy_s(cf.szFaceName, LF_FACESIZE, face);
    SendMessage(target, EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);
}

void FontManager::ApplyFontToAll(const wchar_t* faceName, int heightLogical) {
    if (!faceName) {
        return;
    }
    HFONT newFont = CreateFontByName(faceName, heightLogical);
    if (!newFont) {
        return;
    }
    if (hFont) {
        DeleteObject(hFont);
    }
    hFont = newFont;
}

void FontManager::SetFontPreset(FontPreset preset) {
    switch (preset) {
    case FontPreset::DefaultConsolas:
        ApplyFontToAll(L"Consolas", 16);
        break;
    case FontPreset::RasterCourierNew:
        ApplyFontToAll(L"Courier New", 16);
        break;
    case FontPreset::VectorArial:
        ApplyFontToAll(L"Arial", 16);
        break;
    }
}

HFONT FontManager::CreateFontByName(const wchar_t* faceName, int height) {
    return CreateFont(
        height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        faceName
    );
}
