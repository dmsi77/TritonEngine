// font_manager.hpp

#pragma once

#include <unordered_map>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "../../thirdparty/glm/glm/glm.hpp"
#include "object.hpp"
#include "types.hpp"

namespace realware
{
    class iRenderContext;
    class cApplication;
    struct sTexture;

    struct sGlyph
    {
        types::u8 _character = 0;
        types::s32 _width = 0;
        types::s32 _height = 0;
        types::s32 _left = 0;
        types::s32 _top = 0;
        types::f32 _advanceX = 0.0f;
        types::f32 _advanceY = 0.0f;
        types::s32 _atlasXOffset = 0;
        types::s32 _atlasYOffset = 0;
        void* _bitmapData = nullptr;
    };

    struct sFont
    {
        FT_Face _font = {};
        types::usize _glyphCount = 0;
        types::usize _glyphSize = 0;
        types::usize _offsetNewline = 0;
        types::usize _offsetSpace = 0;
        types::usize _offsetTab = 0;
        std::unordered_map<types::u8, sGlyph> _alphabet = {};
        sTexture* _atlas = nullptr;
    };

    struct sText
    {
        sFont* _font = nullptr;
        std::string _text = "";
    };

    class mFont : public iObject
    {
    public:
        mFont(cContext* context, iRenderContext* renderContext);
        ~mFont();

        inline virtual cType GetType() const override final { return cType("Font"); }

        sFont* CreateFontTTF(const std::string& filename, types::usize glyphSize);
        sText* CreateText(const sFont* font, const std::string& text);
        void DestroyFontTTF(sFont* font);
        void DestroyText(sText* text);
            
        types::f32 GetTextWidth(sFont* font, const std::string& text) const;
        types::f32 GetTextHeight(sFont* font, const std::string& text) const;
        types::usize GetCharacterCount(const std::string& text) const;
        types::usize GetNewlineCount(const std::string& text) const;

        static constexpr types::usize K_MAX_ATLAS_WIDTH = 2048;

    private:
        types::boolean _initialized = types::K_FALSE;
        iRenderContext* _renderContext = nullptr;
        FT_Library _lib = {};
    };
}