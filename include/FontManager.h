#pragma once
#include <windows.h>
#include <string>

class FontManager {
private:
    HFONT hFont;
    enum class FontChoice { DefaultConsolas, RasterCourierNew, VectorArial } currentFont;


public:
    FontManager();
    ~FontManager();
    
    bool CreateDefaultFont();
    void DestroyFont();
    
    void SetFontByMenuId(UINT id);
    void ApplyCurrentFontToFocusedSelection();
    void ApplyFontToAll(const wchar_t* faceName, int heightLogical);
    enum class FontPreset { DefaultConsolas, RasterCourierNew, VectorArial };
    void SetFontPreset(FontPreset preset);
    HFONT GetFont() const { return hFont; }

    FontChoice GetCurrentFont() const { return currentFont; }
    
private:
    HFONT CreateFontByName(const wchar_t* faceName, int height);
};
