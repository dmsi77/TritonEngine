// font_manager.cpp

#include <iostream>
#include "font_manager.hpp"
#include "render_context.hpp"
#include "application.hpp"
#include "memory_pool.hpp"
#include "log.hpp"

using namespace types;

namespace realware
{
    mFont::mFont(cContext* context, iRenderContext* renderContext) : iObject(context), _renderContext(renderContext)
    {
        if (FT_Init_FreeType(&_lib))
        {
            Print("Failed to initialize FreeType library!");
            return;
        }

        _initialized = K_TRUE;
    }

    mFont::~mFont()
    {
        if (_initialized)
            FT_Done_FreeType(_lib);
    }

    usize CalculateNewlineOffset(sFont* font)
    {
        return font->_font->size->metrics.height >> 6;
    }

    usize CalculateSpaceOffset(sFont* font)
    {
        const FT_Face& ftFont = font->_font;
        const FT_UInt spaceIndex = FT_Get_Char_Index(ftFont, ' ');
        if (FT_Load_Glyph(ftFont, spaceIndex, FT_LOAD_DEFAULT) == 0)
            return ftFont->glyph->advance.x >> 6;
        else
            return 0;
    }

    void FillAlphabetAndFindAtlasSize(cMemoryPool* memoryPool, sFont* font, usize& xOffset, usize& atlasWidth, usize& atlasHeight)
    {
        const FT_Face& ftFont = font->_font;
        usize maxGlyphHeight = 0;

        for (usize c = 0; c < 256; c++)
        {
            if (c == '\n' || c == ' ' || c == '\t')
                continue;

            const FT_Int ci = FT_Get_Char_Index(ftFont, c);
            if (FT_Load_Glyph(ftFont, (FT_UInt)ci, FT_LOAD_DEFAULT) == 0)
            {
                font->_glyphCount += 1;

                FT_Render_Glyph(ftFont->glyph, FT_RENDER_MODE_NORMAL);

                sGlyph glyph = {};
                glyph._character = (u8)c;
                glyph._width = ftFont->glyph->bitmap.width;
                glyph._height = ftFont->glyph->bitmap.rows;
                glyph._left = ftFont->glyph->bitmap_left;
                glyph._top = ftFont->glyph->bitmap_top;
                glyph._advanceX = ftFont->glyph->advance.x >> 6;
                glyph._advanceY = ftFont->glyph->advance.y >> 6;
                glyph._bitmapData = memoryPool->Allocate(glyph._width * glyph._height);

                if (ftFont->glyph->bitmap.buffer)
                    memcpy(glyph._bitmapData, ftFont->glyph->bitmap.buffer, glyph._width * glyph._height);

                font->_alphabet.insert({(u8)c, glyph});

                xOffset += glyph._width + 1;

                if (atlasWidth < mFont::K_MAX_ATLAS_WIDTH - (glyph._width + 1))
                    atlasWidth += glyph._width + 1;

                if (glyph._height > maxGlyphHeight)
                    maxGlyphHeight = glyph._height;

                if (xOffset >= mFont::K_MAX_ATLAS_WIDTH)
                {
                    atlasHeight += maxGlyphHeight + 1;
                    xOffset = 0;
                    maxGlyphHeight = 0;
                }
            }
        }

        if (atlasHeight < maxGlyphHeight + 1)
            atlasHeight += maxGlyphHeight + 1;
    }

    usize NextPowerOfTwo(usize n)
    {
        if (n <= 0)
            return 1;

        usize power = 1;
        while (power < n)
        {
            if (power >= 0x80000000)
                return 1;

            power <<= 1;
        }

        return power;
    }

    void MakeAtlasSizePowerOf2(usize& atlasWidth, usize& atlasHeight)
    {
        atlasWidth = NextPowerOfTwo(atlasWidth);
        atlasHeight = NextPowerOfTwo(atlasHeight);
    }

    void FillAtlasWithGlyphs(cMemoryPool* memoryPool, sFont* font, usize& atlasWidth, usize& atlasHeight, iRenderContext* context)
    {
        usize maxGlyphHeight = 0;

        void* atlasPixels = memoryPool->Allocate(atlasWidth * atlasHeight);
        memset(atlasPixels, 0, atlasWidth * atlasHeight);

        usize xOffset = 0;
        usize yOffset = 0;
        u8* pixelsU8 = (u8*)atlasPixels;

        for (auto& glyph : font->_alphabet)
        {
            glyph.second._atlasXOffset = xOffset;
            glyph.second._atlasYOffset = yOffset;

            for (usize y = 0; y < glyph.second._height; y++)
            {
                for (usize x = 0; x < glyph.second._width; x++)
                {
                    const usize glyphPixelIndex = x + (y * glyph.second._width);
                    const usize pixelIndex = (xOffset + x) + ((yOffset + y) * atlasWidth);
                        
                    if (glyphPixelIndex < glyph.second._width * glyph.second._height &&
                        pixelIndex < atlasWidth * atlasHeight)
                        pixelsU8[pixelIndex] = ((u8*)glyph.second._bitmapData)[glyphPixelIndex];
                }
            }

            xOffset += glyph.second._width + 1;
            if (glyph.second._height > maxGlyphHeight)
                maxGlyphHeight = glyph.second._height;

            if (xOffset >= mFont::K_MAX_ATLAS_WIDTH)
            {
                yOffset += maxGlyphHeight + 1;
                xOffset = 0;
                maxGlyphHeight = 0;
            }
        }

        font->_atlas = context->CreateTexture(
            atlasWidth,
            atlasHeight,
            0,
            sTexture::eType::TEXTURE_2D,
            sTexture::eFormat::R8,
            atlasPixels
        );

        memoryPool->Free(atlasPixels);
    }

    sFont* mFont::CreateFontTTF(const std::string& filename, usize glyphSize)
    {
        cApplication* app = GetApplication();
        cMemoryPool* memoryPool = app->GetMemoryPool();
        sFont* pFont = (sFont*)memoryPool->Allocate(sizeof(sFont));
        sFont* font = new (pFont) sFont;

        FT_Face& ftFont = font->_font;

        if (FT_New_Face(_lib, filename.c_str(), 0, &ftFont) == 0)
        {
            FT_Select_Charmap(ftFont, FT_ENCODING_UNICODE);

            if (FT_Set_Pixel_Sizes(ftFont, glyphSize, glyphSize) == 0)
            {
                font->_glyphCount = 0;
                font->_glyphSize = glyphSize;
                font->_offsetNewline = CalculateNewlineOffset(font);
                font->_offsetSpace = CalculateSpaceOffset(font);
                font->_offsetTab = font->_offsetSpace * 4;

                usize atlasWidth = 0;
                usize atlasHeight = 0;
                usize xOffset = 0;

                FillAlphabetAndFindAtlasSize(memoryPool, font, xOffset, atlasWidth, atlasHeight);
                MakeAtlasSizePowerOf2(atlasWidth, atlasHeight);
                FillAtlasWithGlyphs(memoryPool, font, atlasWidth, atlasHeight, _renderContext);
            }
            else
            {
                font->~sFont();
                app->GetMemoryPool()->Free(font);
                    
                return nullptr;
            }
        }
        else
        {
            Print("Error creating FreeType font face!");

            font->~sFont();
            app->GetMemoryPool()->Free(font);
                
            return nullptr;
        }

        return font;
    }

    sText* mFont::CreateText(const sFont* font, const std::string& text)
    {
        sText* pTextObject = (sText*)GetApplication()->GetMemoryPool()->Allocate(sizeof(sText));
        sText* textObject = new (pTextObject) sText;

        textObject->_font = (sFont*)font;
        textObject->_text = text;

        return textObject;
    }

    void mFont::DestroyFontTTF(sFont* font)
    {
        auto& alphabet = font->_alphabet;
        sTexture* atlas = font->_atlas;

        cApplication* app = GetApplication();

        for (const auto& glyph : alphabet)
            app->GetMemoryPool()->Free(glyph.second._bitmapData);

        alphabet.clear();

        _renderContext->DestroyTexture(atlas);

        FT_Done_Face(font->_font);

        font->~sFont();
        app->GetMemoryPool()->Free(font);
    }

    void mFont::DestroyText(sText* text)
    {
        text->~sText();
        GetApplication()->GetMemoryPool()->Free(text);
    }

    f32 mFont::GetTextWidth(sFont* font, const std::string& text) const
    {
        f32 textWidth = 0.0f;
        f32 maxTextWidth = 0.0f;
        const usize textByteSize = strlen(text.c_str());
        const glm::vec2 windowSize = GetApplication()->GetWindowSize();

        for (usize i = 0; i < textByteSize; i++)
        {
            const sGlyph& glyph = font->_alphabet.find(text[i])->second;

            if (text[i] == '\t')
            {
                textWidth += font->_offsetTab;
            }
            else if (text[i] == ' ')
            {
                textWidth += font->_offsetSpace;
            }
            else if (text[i] == '\n')
            {
                if (maxTextWidth < textWidth)
                    maxTextWidth = textWidth;
                textWidth = 0.0f;
            }
            else
            {
                textWidth += ((f32)glyph._width / windowSize.x);
            }
        }

        if (maxTextWidth < textWidth)
        {
            maxTextWidth = textWidth;
            textWidth = 0.0f;
        }

        return maxTextWidth;
    }

    f32 mFont::GetTextHeight(sFont* font, const std::string& text) const
    {
        f32 textHeight = 0.0f;
        f32 maxHeight = 0.0f;
        const usize textByteSize = strlen(text.c_str());
        const glm::vec2 windowSize = GetApplication()->GetWindowSize();

        for (usize i = 0; i < textByteSize; i++)
        {
            const sGlyph& glyph = font->_alphabet.find(text[i])->second;

            if (text[i] == '\n')
            {
                textHeight += font->_offsetNewline;
            }
            else
            {
                f32 glyphHeight = ((f32)glyph._height / windowSize.y);
                if (glyphHeight > maxHeight) {
                    maxHeight = glyphHeight;
                }
            }

            if (i == textByteSize - 1)
            {
                textHeight += maxHeight;
                maxHeight = 0.0f;
            }
        }

        return textHeight;
    }

    usize mFont::GetCharacterCount(const std::string& text) const
    {
        return strlen(text.c_str());
    }

    usize mFont::GetNewlineCount(const std::string& text) const
    {
        usize newlineCount = 0;
        const usize charCount = strlen(text.c_str());
        for (usize i = 0; i < charCount; i++)
        {
            if (text[i] == '\n')
                newlineCount++;
        }

        return newlineCount;
    }
}