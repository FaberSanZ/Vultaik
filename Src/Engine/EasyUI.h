// EasyUI.h
#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <string>
#include <iostream>
#include <cmath>
#include <array>
#include <unordered_map>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"


#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

struct UIVertex
{
    float position[4];
    float color[4];
    float uv[2];

};

struct UIDrawCommand
{
    int layer = 0;
    std::vector<UIVertex> vertices;
    std::vector<uint32_t> indices;
};

enum class UIEventType
{
    MouseDown,
    MouseUp,
    MouseMove
};

struct UIEvent
{
    UIEventType type;
    float x, y;
    int button = 0;
    bool handled = false;
    bool bubbles = true;
};

inline std::array<float, 4> toFloat4(uint32_t c)
{
    return {
        ((c >> 16) & 0xFF) / 255.f,
        ((c >> 8) & 0xFF) / 255.f,
        ((c) & 0xFF) / 255.f,
        ((c >> 24) & 0xFF) / 255.f
    };
}

// Layout structures
enum class HorizontalAlignment
{
    Left,
    Center,
    Right,
    Stretch
};

enum class VerticalAlignment
{
    Top,
    Center,
    Bottom,
    Stretch
};

struct Thickness
{
    float left = 0, top = 0, right = 0, bottom = 0;
    Thickness() = default;
    Thickness(float uniform) : left(uniform), top(uniform), right(uniform), bottom(uniform) {}
    Thickness(float l, float t, float r, float b) : left(l), top(t), right(r), bottom(b) {}
};

struct GlyphInfo
{
    float u0, v0, u1, v1;   // coordenadas de textura
    float xAdvance;          // avance horizontal (cuánto mover después del glifo)
    float xOffset, yOffset;  // offset desde el punto de origen
    int width, height;       // tamaño del glifo en píxeles
};


class UIElement
{
public:
    UIElement(UIElement* parent = nullptr)
        : m_parent(parent), m_desiredWidth(100), m_desiredHeight(100),
        m_color(0xFFFFFFFF), m_visible(true), m_absX(0), m_absY(0),
        m_canBeHitByUser(true), m_id(++s_nextID),
        m_horizontalAlignment(HorizontalAlignment::Left),
        m_verticalAlignment(VerticalAlignment::Top),
        m_useLayoutRounding(false)
    {
    }

    virtual ~UIElement() {}

    // Layout properties
    void setHorizontalAlignment(HorizontalAlignment align) { m_horizontalAlignment = align; }
    void setVerticalAlignment(VerticalAlignment align) { m_verticalAlignment = align; }
    void setMargin(const Thickness& margin) { mMargin = margin; }
    void setPadding(const Thickness& padding) { mPadding = padding; }
    void setUseLayoutRounding(bool enable) { m_useLayoutRounding = enable; }

    HorizontalAlignment getHorizontalAlignment() const { return m_horizontalAlignment; }
    VerticalAlignment getVerticalAlignment() const { return m_verticalAlignment; }
    Thickness getMargin() const { return mMargin; }
    Thickness getPadding() const { return mPadding; }
    bool getUseLayoutRounding() const { return m_useLayoutRounding; }

    // Legacy position/size (converted to margin + desired size)
    void setPosition(float x, float y)
    {
        mMargin.left = x;
        mMargin.top = y;
        m_horizontalAlignment = HorizontalAlignment::Left;
        m_verticalAlignment = VerticalAlignment::Top;
    }
    void setSize(float w, float h) { m_desiredWidth = w; m_desiredHeight = h; }
    void setColor(uint32_t c) { m_color = c; }
    void setVisible(bool v) { m_visible = v; }
    void setCanBeHitByUser(bool can) { m_canBeHitByUser = can; }

    float getAbsoluteX() const { return m_absX; }
    float getAbsoluteY() const { return m_absY; }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
    UIElement* getParent() const { return m_parent; }
    uint32_t getID() const { return m_id; }
    std::vector<UIElement*> getChildren() const { return m_children; }

    void addChild(UIElement* child)
    {
        if (!child) return;
        child->m_parent = this;
        m_children.push_back(child);
        if (m_manager) child->setManager(m_manager);
    }

    void setManager(class UIManager* mgr) { m_manager = mgr; }

    // New layout method: parent provides its content area (origin + size)
    virtual void updateLayout(float parentContentX, float parentContentY,
        float parentContentWidth, float parentContentHeight)
    {
        // Compute final rectangle based on alignment, margin, desired size, and parent content area
        float availableWidth = parentContentWidth - mMargin.left - mMargin.right;
        float availableHeight = parentContentHeight - mMargin.top - mMargin.bottom;

        // Determine width
        if (m_horizontalAlignment == HorizontalAlignment::Stretch)
            m_width = std::max(0.0f, availableWidth);
        else
            m_width = std::max(0.0f, m_desiredWidth);

        // Determine height
        if (m_verticalAlignment == VerticalAlignment::Stretch)
            m_height = std::max(0.0f, availableHeight);
        else
            m_height = std::max(0.0f, m_desiredHeight);

        // Compute X position
        switch (m_horizontalAlignment)
        {
        case HorizontalAlignment::Left:
            m_absX = parentContentX + mMargin.left;
            break;
        case HorizontalAlignment::Center:
            m_absX = parentContentX + mMargin.left + (availableWidth - m_width) * 0.5f;
            break;
        case HorizontalAlignment::Right:
            m_absX = parentContentX + parentContentWidth - mMargin.right - m_width;
            break;
        case HorizontalAlignment::Stretch:
            m_absX = parentContentX + mMargin.left;
            break;
        }

        // Compute Y position
        switch (m_verticalAlignment)
        {
        case VerticalAlignment::Top:
            m_absY = parentContentY + mMargin.top;
            break;
        case VerticalAlignment::Center:
            m_absY = parentContentY + mMargin.top + (availableHeight - m_height) * 0.5f;
            break;
        case VerticalAlignment::Bottom:
            m_absY = parentContentY + parentContentHeight - mMargin.bottom - m_height;
            break;
        case VerticalAlignment::Stretch:
            m_absY = parentContentY + mMargin.top;
            break;
        }

        // Apply layout rounding if enabled
        if (m_useLayoutRounding)
        {
            m_absX = std::round(m_absX);
            m_absY = std::round(m_absY);
        }

        // Compute content area for children (inside our padding)
        float childContentX = m_absX + mPadding.left;
        float childContentY = m_absY + mPadding.top;
        float childContentW = m_width - mPadding.left - mPadding.right;
        float childContentH = m_height - mPadding.top - mPadding.bottom;

        // Recursively layout children
        for (auto c : m_children)
            c->updateLayout(childContentX, childContentY, childContentW, childContentH);
    }

    virtual UIElement* hitTest(float x, float y)
    {
        if (!m_visible || !m_canBeHitByUser) return nullptr;
        if (x >= m_absX && x < m_absX + m_width &&
            y >= m_absY && y < m_absY + m_height)
        {
            for (auto it = m_children.rbegin(); it != m_children.rend(); ++it)
            {
                if (auto hit = (*it)->hitTest(x, y))
                    return hit;
            }
            return this;
        }
        return nullptr;
    }

    virtual void onEvent(UIEvent& e) {}

    virtual void generateVertices(std::vector<UIVertex>& v, std::vector<uint32_t>& i)
    {
        if (!m_visible) return;
        float x0 = m_absX, y0 = m_absY, x1 = x0 + m_width, y1 = y0 + m_height;
        uint32_t base = (uint32_t)v.size();
        auto col = toFloat4(m_color);
        v.push_back({ {x0,y0,0,1}, {col[0],col[1],col[2],col[3]}, {0,0} });
        v.push_back({ {x1,y0,0,1}, {col[0],col[1],col[2],col[3]} , {0,0} });
        v.push_back({ {x1,y1,0,1}, {col[0],col[1],col[2],col[3]} , {0,0} });
        v.push_back({ {x0,y1,0,1}, {col[0],col[1],col[2],col[3]} , {0,0} });
        i.push_back(base + 0); i.push_back(base + 1); i.push_back(base + 2);
        i.push_back(base + 0); i.push_back(base + 2); i.push_back(base + 3);
        for (auto c : m_children) c->generateVertices(v, i);
    }

    virtual void generateCommands(std::vector<UIDrawCommand>& commands, int layer = 0)
    {
        if (!m_visible) return;
        if (commands.size() <= (size_t)layer) commands.resize(layer + 1);
        auto& cmd = commands[layer];
        uint32_t base = (uint32_t)cmd.vertices.size();
        auto col = toFloat4(m_color);
        float x0 = m_absX, y0 = m_absY, x1 = x0 + m_width, y1 = y0 + m_height;
        cmd.vertices.push_back({ {x0,y0,0,1}, {col[0],col[1],col[2],col[3]} });
        cmd.vertices.push_back({ {x1,y0,0,1}, {col[0],col[1],col[2],col[3]} });
        cmd.vertices.push_back({ {x1,y1,0,1}, {col[0],col[1],col[2],col[3]} });
        cmd.vertices.push_back({ {x0,y1,0,1}, {col[0],col[1],col[2],col[3]} });
        cmd.indices.push_back(base + 0); cmd.indices.push_back(base + 1); cmd.indices.push_back(base + 2);
        cmd.indices.push_back(base + 0); cmd.indices.push_back(base + 2); cmd.indices.push_back(base + 3);
        for (auto c : m_children) c->generateCommands(commands, layer);
    }

protected:
    UIElement* m_parent = nullptr;
    std::vector<UIElement*> m_children;
    float m_desiredWidth, m_desiredHeight;   // original size set by user
    float m_width, m_height;                 // final size after layout
    uint32_t m_color;
    bool m_visible;
    float m_absX, m_absY;                    // final absolute position
    bool m_canBeHitByUser;
    uint32_t m_id;
    class UIManager* m_manager = nullptr;

    // New layout members
    HorizontalAlignment m_horizontalAlignment;
    VerticalAlignment m_verticalAlignment;
    Thickness mMargin;
    Thickness mPadding;
    bool m_useLayoutRounding;

    static uint32_t s_nextID;
};

uint32_t UIElement::s_nextID = 1;

// ------------------------------------------------------------------
// UIRect (con bordes)
// ------------------------------------------------------------------
class UIRect : public UIElement
{
public:
    UIRect(UIElement* p = nullptr) : UIElement(p), m_border(false), m_borderSize(0), m_borderColor(0) {}

    void setBorder(float t, uint32_t c) { m_border = true; m_borderSize = t; m_borderColor = c; }

    void generateVertices(std::vector<UIVertex>& v, std::vector<uint32_t>& i) override
    {
        UIElement::generateVertices(v, i);
        if (!m_border) return;
        float x0 = m_absX, y0 = m_absY, x1 = x0 + m_width, y1 = y0 + m_height, t = m_borderSize;
        auto add = [&](float X0, float Y0, float X1, float Y1) {
            uint32_t base = (uint32_t)v.size();
            auto col = toFloat4(m_borderColor);
            v.push_back({ {X0,Y0,0,1}, {col[0],col[1],col[2],col[3]} });
            v.push_back({ {X1,Y0,0,1}, {col[0],col[1],col[2],col[3]} });
            v.push_back({ {X1,Y1,0,1}, {col[0],col[1],col[2],col[3]} });
            v.push_back({ {X0,Y1,0,1}, {col[0],col[1],col[2],col[3]} });
            i.push_back(base + 0); i.push_back(base + 1); i.push_back(base + 2);
            i.push_back(base + 0); i.push_back(base + 2); i.push_back(base + 3);
            };
        add(x0, y0, x1, y0 + t);
        add(x0, y1 - t, x1, y1);
        add(x0, y0 + t, x0 + t, y1 - t);
        add(x1 - t, y0 + t, x1, y1 - t);
    }

    void generateCommands(std::vector<UIDrawCommand>& commands, int layer) override
    {
        UIElement::generateCommands(commands, layer);
        if (!m_border) return;
        int borderLayer = layer + 1;
        if (commands.size() <= (size_t)borderLayer) commands.resize(borderLayer + 1);
        auto& cmd = commands[borderLayer];
        float x0 = m_absX, y0 = m_absY, x1 = x0 + m_width, y1 = y0 + m_height, t = m_borderSize;
        auto add = [&](float X0, float Y0, float X1, float Y1) {
            uint32_t base = (uint32_t)cmd.vertices.size();
            auto col = toFloat4(m_borderColor);
            cmd.vertices.push_back({ {X0,Y0,0,1}, {col[0],col[1],col[2],col[3]} });
            cmd.vertices.push_back({ {X1,Y0,0,1}, {col[0],col[1],col[2],col[3]} });
            cmd.vertices.push_back({ {X1,Y1,0,1}, {col[0],col[1],col[2],col[3]} });
            cmd.vertices.push_back({ {X0,Y1,0,1}, {col[0],col[1],col[2],col[3]} });
            cmd.indices.push_back(base + 0); cmd.indices.push_back(base + 1); cmd.indices.push_back(base + 2);
            cmd.indices.push_back(base + 0); cmd.indices.push_back(base + 2); cmd.indices.push_back(base + 3);
            };
        add(x0, y0, x1, y0 + t);
        add(x0, y1 - t, x1, y1);
        add(x0, y0 + t, x0 + t, y1 - t);
        add(x1 - t, y0 + t, x1, y1 - t);
    }

private:
    bool m_border;
    float m_borderSize;
    uint32_t m_borderColor;
};

class FontAtlas
{
public:
    bool LoadFromFile(const char* filename, float fontSize, int atlasWidth = 512, int atlasHeight = 512);
    const GlyphInfo* GetGlyph(int codepoint) const;
    ID3D12Resource* GetTextureResource() const { return m_texture.Get(); } // D3D12 específico
    float GetFontSize() const { return m_fontSize; }
    float GetLineHeight() const { return m_lineHeight; }

private:
    std::unordered_map<int, GlyphInfo> m_glyphs;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;
    float m_fontSize = 0.0f;
    float m_lineHeight = 0.0f;
    int m_atlasWidth = 0, m_atlasHeight = 0;
};



bool FontAtlas::LoadFromFile(const char* filename, float fontSize, int atlasWidth, int atlasHeight)
{
    FILE* f = nullptr;
    if (fopen_s(&f, filename, "rb") != 0 || !f)
        return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::vector<unsigned char> ttfBuffer(size);
    fread(ttfBuffer.data(), 1, size, f);
    fclose(f);

    // Inicializar stbtt_fontinfo
    stbtt_fontinfo fontInfo;
    if (!stbtt_InitFont(&fontInfo, ttfBuffer.data(), 0))
        return false;

    float scale = stbtt_ScaleForPixelHeight(&fontInfo, fontSize);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
    m_lineHeight = (ascent - descent + lineGap) * scale;

    // Crear un atlas temporal en CPU
    std::vector<unsigned char> atlasData(atlasWidth * atlasHeight, 0);
    int penX = 1, penY = 1;
    int maxRowHeight = 0;

    // Generar glifos para ASCII 32-126 (caracteres imprimibles básicos)
    for (int c = 32; c <= 126; ++c)
    {
        int glyphIndex = stbtt_FindGlyphIndex(&fontInfo, c);
        int advanceWidth, leftSideBearing;
        stbtt_GetGlyphHMetrics(&fontInfo, glyphIndex, &advanceWidth, &leftSideBearing);

        int x0, y0, x1, y1;
        stbtt_GetGlyphBitmapBox(&fontInfo, glyphIndex, scale, scale, &x0, &y0, &x1, &y1);
        int glyphW = x1 - x0;
        int glyphH = y1 - y0;

        // Verificar espacio en el atlas
        if (penX + glyphW + 1 >= atlasWidth)
        {
            penX = 1;
            penY += maxRowHeight + 1;
            maxRowHeight = 0;
        }
        if (penY + glyphH + 1 >= atlasHeight)
        {
            // Atlas demasiado pequeño -> podrías redimensionarlo o expandirlo dinámicamente
            return false;
        }

        // Renderizar el glifo en el atlas
        unsigned char* dest = atlasData.data() + penY * atlasWidth + penX;
        stbtt_MakeGlyphBitmap(&fontInfo, dest, glyphW, glyphH, atlasWidth, scale, scale, glyphIndex);

        // Guardar información UV y métricas
        GlyphInfo info;
        info.u0 = (float)penX / atlasWidth;
        info.v0 = (float)penY / atlasHeight;
        info.u1 = (float)(penX + glyphW) / atlasWidth;
        info.v1 = (float)(penY + glyphH) / atlasHeight;
        info.xAdvance = advanceWidth * scale;
        info.xOffset = (float)x0;
        info.yOffset = (float)y0;
        info.width = glyphW;
        info.height = glyphH;
        m_glyphs[c] = info;

        // Avanzar el cursor
        penX += glyphW + 1;
        if (glyphH > maxRowHeight) maxRowHeight = glyphH;
    }

    // Crear textura D3D12 (simplificado, necesitarás subir los datos)
    // D3D12_RESOURCE_DESC texDesc = ...;
    // m_Device->CreateCommittedResource(...);
    // Copiar atlasData a la textura.
    // (Te muestro solo la lógica básica; depende de tu Render)
    m_atlasWidth = atlasWidth;
    m_atlasHeight = atlasHeight;
    return true;
}

const GlyphInfo* FontAtlas::GetGlyph(int codepoint) const
{
    auto it = m_glyphs.find(codepoint);
    return (it != m_glyphs.end()) ? &it->second : nullptr;
}



// ------------------------------------------------------------------
// UIText (requiere un FontAtlas global o asignado)
// ------------------------------------------------------------------
class UIText : public UIElement
{
public:
    UIText(UIElement* parent = nullptr) : UIElement(parent)
    {
        setColor(0xFFFFFFFF);
        m_fontAtlas = nullptr;
        m_fontSize = 16.0f;
    }

    void setText(const std::string& text) { m_text = text; }
    void setFontAtlas(FontAtlas* atlas) { m_fontAtlas = atlas; }
    void setFontSize(float size) { m_fontSize = size; }
    void setColor(uint32_t color) { m_color = color; } // heredado, pero lo usamos para el texto

    // Opcional: alineación del texto dentro de su área
    void setTextAlignment(HorizontalAlignment hAlign, VerticalAlignment vAlign)
    {
        m_textHAlign = hAlign;
        m_textVAlign = vAlign;
    }

    virtual void updateLayout(float parentContentX, float parentContentY,
        float parentContentWidth, float parentContentHeight) override
    {
        // Calcular el tamaño deseado del texto (si es necesario)
        if (m_fontAtlas)
        {
            // Medir el texto para determinar ancho y alto
            float textWidth = 0.0f, textHeight = m_fontAtlas->GetLineHeight() * m_fontSize / m_fontAtlas->GetFontSize();
            for (char c : m_text)
            {
                auto glyph = m_fontAtlas->GetGlyph((int)c);
                if (glyph) textWidth += glyph->xAdvance * (m_fontSize / m_fontAtlas->GetFontSize());
            }
            m_desiredWidth = textWidth;
            m_desiredHeight = textHeight;
        }
        UIElement::updateLayout(parentContentX, parentContentY, parentContentWidth, parentContentHeight);
    }

    virtual void generateCommands(std::vector<UIDrawCommand>& commands, int layer) override
    {
        if (!m_visible || !m_fontAtlas || m_text.empty()) return;

        int textLayer = layer + 1; // dibujar encima del fondo si lo hubiera
        if (commands.size() <= (size_t)textLayer) commands.resize(textLayer + 1);
        auto& cmd = commands[textLayer];

        float scale = m_fontSize / m_fontAtlas->GetFontSize();
        float penX = m_absX;
        float penY = m_absY;

        // Ajustar alineación vertical
        float textHeight = m_fontAtlas->GetLineHeight() * scale;
        if (m_textVAlign == VerticalAlignment::Center)
            penY += (m_height - textHeight) * 0.5f;
        else if (m_textVAlign == VerticalAlignment::Bottom)
            penY += m_height - textHeight;

        // Ancho total para alineación horizontal
        float totalWidth = 0.0f;
        if (m_textHAlign != HorizontalAlignment::Left)
        {
            for (char c : m_text)
            {
                auto glyph = m_fontAtlas->GetGlyph((int)c);
                if (glyph) totalWidth += glyph->xAdvance * scale;
            }
            if (m_textHAlign == HorizontalAlignment::Center)
                penX += (m_width - totalWidth) * 0.5f;
            else if (m_textHAlign == HorizontalAlignment::Right)
                penX += m_width - totalWidth;
        }

        auto textColor = toFloat4(m_color);

        for (char c : m_text)
        {
            auto glyph = m_fontAtlas->GetGlyph((int)c);
            if (!glyph) continue;

            float x0 = penX + glyph->xOffset * scale;
            float y0 = penY + glyph->yOffset * scale;
            float x1 = x0 + glyph->width * scale;
            float y1 = y0 + glyph->height * scale;

            uint32_t base = (uint32_t)cmd.vertices.size();
            cmd.vertices.push_back({ {x0, y0, 0, 1}, {textColor[0], textColor[1], textColor[2], textColor[3]} });
            cmd.vertices.push_back({ {x1, y0, 0, 1}, {textColor[0], textColor[1], textColor[2], textColor[3]} });
            cmd.vertices.push_back({ {x1, y1, 0, 1}, {textColor[0], textColor[1], textColor[2], textColor[3]} });
            cmd.vertices.push_back({ {x0, y1, 0, 1}, {textColor[0], textColor[1], textColor[2], textColor[3]} });
            cmd.indices.push_back(base + 0); cmd.indices.push_back(base + 1); cmd.indices.push_back(base + 2);
            cmd.indices.push_back(base + 0); cmd.indices.push_back(base + 2); cmd.indices.push_back(base + 3);

            // Avanzar cursor
            penX += glyph->xAdvance * scale;
        }
    }

private:
    std::string m_text;
    FontAtlas* m_fontAtlas;
    float m_fontSize;
    HorizontalAlignment m_textHAlign = HorizontalAlignment::Left;
    VerticalAlignment m_textVAlign = VerticalAlignment::Top;
};




// ------------------------------------------------------------------
// UIButton
// ------------------------------------------------------------------
class UIButton : public UIRect
{
public:
    UIButton(UIElement* p = nullptr) : UIRect(p) {}

    void setOnClick(std::function<void()> cb) { m_onClick = cb; }

    void onEvent(UIEvent& e) override
    {
        if (e.type == UIEventType::MouseDown)
        {
            m_pressed = true;
            e.handled = true;
        }
        else if (e.type == UIEventType::MouseUp)
        {
            if (m_pressed)
            {
                m_pressed = false;
                if (m_onClick) m_onClick();
                e.handled = true;
            }
        }
    }

private:
    bool m_pressed = false;
    std::function<void()> m_onClick;
};

// ------------------------------------------------------------------
// UISlider
// ------------------------------------------------------------------
// ------------------------------------------------------------------
// UISlider (mejorado: handle cuadrado con tamaño personalizable)
// ------------------------------------------------------------------
class UISlider : public UIRect
{
public:
    UISlider(UIElement* p = nullptr)
        : UIRect(p), m_value(0.5f), m_drag(false),
        m_handleSize(20.0f), m_handleColor(0xFFFFFFFF),
        m_handleBorder(false), m_handleBorderSize(1.0f), m_handleBorderColor(0xFF000000) {
    }

    void setValue(float v)
    {
        m_value = std::clamp(v, 0.f, 1.f);
        if (m_cb) m_cb(m_value);
    }

    void setOnChange(std::function<void(float)> cb) { m_cb = cb; }

    // Configuración del handle
    void setHandleSize(float size) { m_handleSize = std::max(4.0f, size); }
    void setHandleColor(uint32_t color) { m_handleColor = color; }
    void setHandleBorder(float thickness, uint32_t color)
    {
        m_handleBorder = true;
        m_handleBorderSize = thickness;
        m_handleBorderColor = color;
    }
    void disableHandleBorder() { m_handleBorder = false; }

    void onEvent(UIEvent& e) override
    {
        if (e.type == UIEventType::MouseDown)
        {
            m_drag = true;
            setValue((e.x - m_absX) / m_width);
            e.handled = true;
        }
        else if (e.type == UIEventType::MouseMove)
        {
            if (m_drag)
            {
                setValue((e.x - m_absX) / m_width);
                e.handled = true;
            }
        }
        else if (e.type == UIEventType::MouseUp)
        {
            m_drag = false;
            e.handled = true;
        }
    }

    void generateVertices(std::vector<UIVertex>& v, std::vector<uint32_t>& i) override
    {
        // Primero dibujar el fondo (heredado)
        UIRect::generateVertices(v, i);

        // Calcular posición del handle (centrado en el valor, verticalmente centrado)
        float cx = m_absX + m_value * m_width;
        float halfSize = m_handleSize * 0.5f;
        float x0 = cx - halfSize;
        float x1 = cx + halfSize;
        float y0 = m_absY + (m_height - m_handleSize) * 0.5f;
        float y1 = y0 + m_handleSize;

        uint32_t base = (uint32_t)v.size();
        auto col = toFloat4(m_handleColor);
        v.push_back({ {x0, y0, 0, 1}, {col[0], col[1], col[2], col[3]} });
        v.push_back({ {x1, y0, 0, 1}, {col[0], col[1], col[2], col[3]} });
        v.push_back({ {x1, y1, 0, 1}, {col[0], col[1], col[2], col[3]} });
        v.push_back({ {x0, y1, 0, 1}, {col[0], col[1], col[2], col[3]} });
        i.push_back(base + 0); i.push_back(base + 1); i.push_back(base + 2);
        i.push_back(base + 0); i.push_back(base + 2); i.push_back(base + 3);

        // Borde del handle (opcional)
        if (m_handleBorder)
        {
            auto addBorderRect = [&](float X0, float Y0, float X1, float Y1, float t)
                {
                    uint32_t bbase = (uint32_t)v.size();
                    auto colB = toFloat4(m_handleBorderColor);
                    // Borde superior
                    v.push_back({ {X0, Y0, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    v.push_back({ {X1, Y0, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    v.push_back({ {X1, Y0 + t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    v.push_back({ {X0, Y0 + t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    i.push_back(bbase + 0); i.push_back(bbase + 1); i.push_back(bbase + 2);
                    i.push_back(bbase + 0); i.push_back(bbase + 2); i.push_back(bbase + 3);
                    // Borde inferior
                    bbase = (uint32_t)v.size();
                    v.push_back({ {X0, Y1 - t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    v.push_back({ {X1, Y1 - t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    v.push_back({ {X1, Y1, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    v.push_back({ {X0, Y1, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    i.push_back(bbase + 0); i.push_back(bbase + 1); i.push_back(bbase + 2);
                    i.push_back(bbase + 0); i.push_back(bbase + 2); i.push_back(bbase + 3);
                    // Borde izquierdo
                    bbase = (uint32_t)v.size();
                    v.push_back({ {X0, Y0 + t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    v.push_back({ {X0 + t, Y0 + t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    v.push_back({ {X0 + t, Y1 - t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    v.push_back({ {X0, Y1 - t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    i.push_back(bbase + 0); i.push_back(bbase + 1); i.push_back(bbase + 2);
                    i.push_back(bbase + 0); i.push_back(bbase + 2); i.push_back(bbase + 3);
                    // Borde derecho
                    bbase = (uint32_t)v.size();
                    v.push_back({ {X1 - t, Y0 + t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    v.push_back({ {X1, Y0 + t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    v.push_back({ {X1, Y1 - t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    v.push_back({ {X1 - t, Y1 - t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    i.push_back(bbase + 0); i.push_back(bbase + 1); i.push_back(bbase + 2);
                    i.push_back(bbase + 0); i.push_back(bbase + 2); i.push_back(bbase + 3);
                };
            addBorderRect(x0, y0, x1, y1, m_handleBorderSize);
        }
    }

    void generateCommands(std::vector<UIDrawCommand>& commands, int layer) override
    {
        // Dibujar fondo y borde del slider (heredado)
        UIRect::generateCommands(commands, layer);

        // El handle se dibuja en una capa superior
        int handleLayer = layer + 2;
        if (commands.size() <= (size_t)handleLayer) commands.resize(handleLayer + 1);
        auto& cmd = commands[handleLayer];

        float cx = m_absX + m_value * m_width;
        float halfSize = m_handleSize * 0.5f;
        float x0 = cx - halfSize;
        float x1 = cx + halfSize;
        float y0 = m_absY + (m_height - m_handleSize) * 0.5f;
        float y1 = y0 + m_handleSize;

        uint32_t base = (uint32_t)cmd.vertices.size();
        auto col = toFloat4(m_handleColor);
        cmd.vertices.push_back({ {x0, y0, 0, 1}, {col[0], col[1], col[2], col[3]} });
        cmd.vertices.push_back({ {x1, y0, 0, 1}, {col[0], col[1], col[2], col[3]} });
        cmd.vertices.push_back({ {x1, y1, 0, 1}, {col[0], col[1], col[2], col[3]} });
        cmd.vertices.push_back({ {x0, y1, 0, 1}, {col[0], col[1], col[2], col[3]} });
        cmd.indices.push_back(base + 0); cmd.indices.push_back(base + 1); cmd.indices.push_back(base + 2);
        cmd.indices.push_back(base + 0); cmd.indices.push_back(base + 2); cmd.indices.push_back(base + 3);

        if (m_handleBorder)
        {
            auto addBorderRect = [&](float X0, float Y0, float X1, float Y1, float t)
                {
                    uint32_t bbase = (uint32_t)cmd.vertices.size();
                    auto colB = toFloat4(m_handleBorderColor);
                    // Superior
                    cmd.vertices.push_back({ {X0, Y0, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    cmd.vertices.push_back({ {X1, Y0, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    cmd.vertices.push_back({ {X1, Y0 + t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    cmd.vertices.push_back({ {X0, Y0 + t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    cmd.indices.push_back(bbase + 0); cmd.indices.push_back(bbase + 1); cmd.indices.push_back(bbase + 2);
                    cmd.indices.push_back(bbase + 0); cmd.indices.push_back(bbase + 2); cmd.indices.push_back(bbase + 3);
                    // Inferior
                    bbase = (uint32_t)cmd.vertices.size();
                    cmd.vertices.push_back({ {X0, Y1 - t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    cmd.vertices.push_back({ {X1, Y1 - t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    cmd.vertices.push_back({ {X1, Y1, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    cmd.vertices.push_back({ {X0, Y1, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    cmd.indices.push_back(bbase + 0); cmd.indices.push_back(bbase + 1); cmd.indices.push_back(bbase + 2);
                    cmd.indices.push_back(bbase + 0); cmd.indices.push_back(bbase + 2); cmd.indices.push_back(bbase + 3);
                    // Izquierdo
                    bbase = (uint32_t)cmd.vertices.size();
                    cmd.vertices.push_back({ {X0, Y0 + t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    cmd.vertices.push_back({ {X0 + t, Y0 + t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    cmd.vertices.push_back({ {X0 + t, Y1 - t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    cmd.vertices.push_back({ {X0, Y1 - t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    cmd.indices.push_back(bbase + 0); cmd.indices.push_back(bbase + 1); cmd.indices.push_back(bbase + 2);
                    cmd.indices.push_back(bbase + 0); cmd.indices.push_back(bbase + 2); cmd.indices.push_back(bbase + 3);
                    // Derecho
                    bbase = (uint32_t)cmd.vertices.size();
                    cmd.vertices.push_back({ {X1 - t, Y0 + t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    cmd.vertices.push_back({ {X1, Y0 + t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    cmd.vertices.push_back({ {X1, Y1 - t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    cmd.vertices.push_back({ {X1 - t, Y1 - t, 0, 1}, {colB[0], colB[1], colB[2], colB[3]} });
                    cmd.indices.push_back(bbase + 0); cmd.indices.push_back(bbase + 1); cmd.indices.push_back(bbase + 2);
                    cmd.indices.push_back(bbase + 0); cmd.indices.push_back(bbase + 2); cmd.indices.push_back(bbase + 3);
                };
            addBorderRect(x0, y0, x1, y1, m_handleBorderSize);
        }
    }

private:
    float m_value;
    bool m_drag;
    std::function<void(float)> m_cb;

    // Handle properties
    float m_handleSize;
    uint32_t m_handleColor;
    bool m_handleBorder;
    float m_handleBorderSize;
    uint32_t m_handleBorderColor;
};

// ------------------------------------------------------------------
// UICheckBox
// ------------------------------------------------------------------
class UICheckBox : public UIRect
{
public:
    UICheckBox(UIElement* p = nullptr) : UIRect(p) {}

    void setChecked(bool v) { m_checked = v; if (m_cb) m_cb(m_checked); }
    bool isChecked() const { return m_checked; }
    void setOnChange(std::function<void(bool)> cb) { m_cb = cb; }

    void onEvent(UIEvent& e) override
    {
        if (e.type == UIEventType::MouseDown)
        {
            if (hitTest(e.x, e.y))
            {
                m_pressed = true;
                e.handled = true;
            }
        }
        else if (e.type == UIEventType::MouseUp)
        {
            if (m_pressed && hitTest(e.x, e.y))
            {
                m_checked = !m_checked;
                if (m_cb) m_cb(m_checked);
                e.handled = true;
            }
            m_pressed = false;
        }
    }

    void generateCommands(std::vector<UIDrawCommand>& commands, int layer) override
    {
        UIRect::generateCommands(commands, layer);
        if (!m_checked) return;

        int fillLayer = layer + 2;
        if (commands.size() <= (size_t)fillLayer) commands.resize(fillLayer + 1);
        auto& fillCmd = commands[fillLayer];

        float padding = 4.0f;
        float x0 = m_absX + padding;
        float y0 = m_absY + padding;
        float x1 = m_absX + m_width - padding;
        float y1 = m_absY + m_height - padding;

        auto colFill = toFloat4(0xFF4CAF50);
        uint32_t baseFill = (uint32_t)fillCmd.vertices.size();
        fillCmd.vertices.push_back({ {x0, y0, 0, 1}, {colFill[0], colFill[1], colFill[2], colFill[3]} });
        fillCmd.vertices.push_back({ {x1, y0, 0, 1}, {colFill[0], colFill[1], colFill[2], colFill[3]} });
        fillCmd.vertices.push_back({ {x1, y1, 0, 1}, {colFill[0], colFill[1], colFill[2], colFill[3]} });
        fillCmd.vertices.push_back({ {x0, y1, 0, 1}, {colFill[0], colFill[1], colFill[2], colFill[3]} });
        fillCmd.indices.push_back(baseFill + 0); fillCmd.indices.push_back(baseFill + 1); fillCmd.indices.push_back(baseFill + 2);
        fillCmd.indices.push_back(baseFill + 0); fillCmd.indices.push_back(baseFill + 2); fillCmd.indices.push_back(baseFill + 3);

        int markLayer = layer + 3;
        if (commands.size() <= (size_t)markLayer) commands.resize(markLayer + 1);
        auto& markCmd = commands[markLayer];

        float w = m_width - 2 * padding;
        float h = m_height - 2 * padding;
        float cx = x0 + w * 0.5f;
        float cy = y0 + h * 0.5f;

        float x1v = x0 + w * 0.2f;
        float y1v = cy;
        float x2v = x0 + w * 0.45f;
        float y2v = y0 + h * 0.7f;
        float x3v = x0 + w * 0.8f;
        float y3v = y0 + h * 0.3f;

        float side = (w < h) ? w : h;
        float thickness = side * 0.12f;
        if (thickness < 2.0f) thickness = 2.0f;

        auto addSegment = [&](float ax, float ay, float bx, float by)
            {
                float dx = bx - ax;
                float dy = by - ay;
                float len = sqrt(dx * dx + dy * dy);
                if (len < 0.001f) return;
                dx /= len;
                dy /= len;
                float perpX = -dy * thickness * 0.5f;
                float perpY = dx * thickness * 0.5f;

                uint32_t baseSeg = (uint32_t)markCmd.vertices.size();
                auto colWhite = toFloat4(0xFFFFFFFF);
                markCmd.vertices.push_back({ {ax + perpX, ay + perpY, 0, 1}, {colWhite[0], colWhite[1], colWhite[2], colWhite[3]} });
                markCmd.vertices.push_back({ {ax - perpX, ay - perpY, 0, 1}, {colWhite[0], colWhite[1], colWhite[2], colWhite[3]} });
                markCmd.vertices.push_back({ {bx - perpX, by - perpY, 0, 1}, {colWhite[0], colWhite[1], colWhite[2], colWhite[3]} });
                markCmd.vertices.push_back({ {bx + perpX, by + perpY, 0, 1}, {colWhite[0], colWhite[1], colWhite[2], colWhite[3]} });
                markCmd.indices.push_back(baseSeg + 0); markCmd.indices.push_back(baseSeg + 1); markCmd.indices.push_back(baseSeg + 2);
                markCmd.indices.push_back(baseSeg + 0); markCmd.indices.push_back(baseSeg + 2); markCmd.indices.push_back(baseSeg + 3);
            };

        addSegment(x1v, y1v, x2v, y2v);
        addSegment(x2v, y2v, x3v, y3v);
    }

private:
    bool m_checked = false;
    bool m_pressed = false;
    std::function<void(bool)> m_cb;
};

// ------------------------------------------------------------------
// UIDropdown
// ------------------------------------------------------------------
class UIDropdown : public UIRect
{
public:
    using OptionCallback = std::function<void(int index, const std::string& text)>;

    UIDropdown(UIElement* parent = nullptr) : UIRect(parent)
    {
        setSize(150, 30);
        m_isOpen = false;
        m_selectedIndex = -1;
        m_itemHeight = 30;
        m_maxVisibleItems = 5;
    }

    void addOption(const std::string& text)
    {
        m_options.push_back(text);
        if (m_selectedIndex == -1 && m_options.size() == 1)
            setSelectedIndex(0);
    }

    void clearOptions() { m_options.clear(); m_selectedIndex = -1; }

    void setSelectedIndex(int idx)
    {
        if (idx >= 0 && idx < (int)m_options.size())
        {
            m_selectedIndex = idx;
            if (m_onChange) m_onChange(m_selectedIndex, m_options[m_selectedIndex]);
        }
    }

    int getSelectedIndex() const { return m_selectedIndex; }
    std::string getSelectedText() const
    {
        if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_options.size())
            return m_options[m_selectedIndex];
        return "";
    }

    void setOnChange(OptionCallback cb) { m_onChange = cb; }

    void open() { if (!m_isOpen && !m_options.empty()) m_isOpen = true; }
    void close() { if (m_isOpen) m_isOpen = false; }
    void toggle() { if (m_isOpen) close(); else open(); }
    bool isOpen() const { return m_isOpen; }
    void setItemHeight(float h) { m_itemHeight = h; }
    void setMaxVisibleItems(int max) { m_maxVisibleItems = max; }

    void onEvent(UIEvent& e) override
    {
        if (e.type == UIEventType::MouseDown)
        {
            if (e.x >= m_absX && e.x < m_absX + m_width &&
                e.y >= m_absY && e.y < m_absY + m_height)
            {
                toggle();
                e.handled = true;
                return;
            }

            if (m_isOpen)
            {
                float panelX = m_absX;
                float panelY = m_absY + m_height;
                float panelW = m_width;
                int visibleCount = std::min<int>((int)m_options.size(), m_maxVisibleItems);
                float panelH = visibleCount * m_itemHeight;

                if (e.x >= panelX && e.x < panelX + panelW &&
                    e.y >= panelY && e.y < panelY + panelH)
                {
                    int idx = (int)((e.y - panelY) / m_itemHeight);
                    if (idx >= 0 && idx < (int)m_options.size())
                    {
                        setSelectedIndex(idx);
                        close();
                    }
                    e.handled = true;
                    return;
                }
                else
                {
                    close();
                    e.handled = true;
                }
            }
        }
    }

    void generateCommands(std::vector<UIDrawCommand>& commands, int layer) override
    {
        UIRect::generateCommands(commands, layer);

        int arrowLayer = layer + 1;
        if (commands.size() <= (size_t)arrowLayer) commands.resize(arrowLayer + 1);
        auto& arrowCmd = commands[arrowLayer];
        float arrowSize = m_height * 0.4f;
        float arrowX = m_absX + m_width - arrowSize - 5;
        float arrowY = m_absY + (m_height - arrowSize) * 0.5f;
        uint32_t baseArrow = (uint32_t)arrowCmd.vertices.size();
        auto colArrow = toFloat4(0xFFFFFFFF);
        arrowCmd.vertices.push_back({ {arrowX, arrowY, 0, 1}, {colArrow[0],colArrow[1],colArrow[2],colArrow[3]} });
        arrowCmd.vertices.push_back({ {arrowX + arrowSize, arrowY, 0, 1}, {colArrow[0],colArrow[1],colArrow[2],colArrow[3]} });
        arrowCmd.vertices.push_back({ {arrowX + arrowSize * 0.5f, arrowY + arrowSize, 0, 1}, {colArrow[0],colArrow[1],colArrow[2],colArrow[3]} });
        arrowCmd.indices.push_back(baseArrow + 0); arrowCmd.indices.push_back(baseArrow + 1); arrowCmd.indices.push_back(baseArrow + 2);

        if (!m_isOpen || m_options.empty()) return;

        int panelLayer = layer + 2;
        if (commands.size() <= (size_t)panelLayer) commands.resize(panelLayer + 1);
        auto& panelCmd = commands[panelLayer];
        float panelX = m_absX, panelY = m_absY + m_height, panelW = m_width;
        int visibleCount = std::min<int>((int)m_options.size(), m_maxVisibleItems);
        float panelH = visibleCount * m_itemHeight;

        uint32_t basePanel = (uint32_t)panelCmd.vertices.size();
        auto colBg = toFloat4(0xFF333333);
        panelCmd.vertices.push_back({ {panelX, panelY, 0, 1}, {colBg[0],colBg[1],colBg[2],colBg[3]} });
        panelCmd.vertices.push_back({ {panelX + panelW, panelY, 0, 1}, {colBg[0],colBg[1],colBg[2],colBg[3]} });
        panelCmd.vertices.push_back({ {panelX + panelW, panelY + panelH, 0, 1}, {colBg[0],colBg[1],colBg[2],colBg[3]} });
        panelCmd.vertices.push_back({ {panelX, panelY + panelH, 0, 1}, {colBg[0],colBg[1],colBg[2],colBg[3]} });
        panelCmd.indices.push_back(basePanel + 0); panelCmd.indices.push_back(basePanel + 1); panelCmd.indices.push_back(basePanel + 2);
        panelCmd.indices.push_back(basePanel + 0); panelCmd.indices.push_back(basePanel + 2); panelCmd.indices.push_back(basePanel + 3);

        auto colBorder = toFloat4(0xFFFFFFFF);
        float t = 1.0f;
        auto addBorder = [&](float X0, float Y0, float X1, float Y1) {
            uint32_t base = (uint32_t)panelCmd.vertices.size();
            panelCmd.vertices.push_back({ {X0,Y0,0,1}, {colBorder[0],colBorder[1],colBorder[2],colBorder[3]} });
            panelCmd.vertices.push_back({ {X1,Y0,0,1}, {colBorder[0],colBorder[1],colBorder[2],colBorder[3]} });
            panelCmd.vertices.push_back({ {X1,Y1,0,1}, {colBorder[0],colBorder[1],colBorder[2],colBorder[3]} });
            panelCmd.vertices.push_back({ {X0,Y1,0,1}, {colBorder[0],colBorder[1],colBorder[2],colBorder[3]} });
            panelCmd.indices.push_back(base + 0); panelCmd.indices.push_back(base + 1); panelCmd.indices.push_back(base + 2);
            panelCmd.indices.push_back(base + 0); panelCmd.indices.push_back(base + 2); panelCmd.indices.push_back(base + 3);
            };
        addBorder(panelX, panelY, panelX + panelW, panelY + t);
        addBorder(panelX, panelY + panelH - t, panelX + panelW, panelY + panelH);
        addBorder(panelX, panelY + t, panelX + t, panelY + panelH - t);
        addBorder(panelX + panelW - t, panelY + t, panelX + panelW, panelY + panelH - t);

        for (int i = 0; i < visibleCount; ++i)
        {
            float optY = panelY + i * m_itemHeight;
            uint32_t optColor = (i == m_selectedIndex) ? 0xFF4444FF : 0xFF555555;
            auto colOpt = toFloat4(optColor);
            uint32_t baseOpt = (uint32_t)panelCmd.vertices.size();
            panelCmd.vertices.push_back({ {panelX + 1, optY + 1, 0, 1}, {colOpt[0],colOpt[1],colOpt[2],colOpt[3]} });
            panelCmd.vertices.push_back({ {panelX + panelW - 1, optY + 1, 0, 1}, {colOpt[0],colOpt[1],colOpt[2],colOpt[3]} });
            panelCmd.vertices.push_back({ {panelX + panelW - 1, optY + m_itemHeight - 1, 0, 1}, {colOpt[0],colOpt[1],colOpt[2],colOpt[3]} });
            panelCmd.vertices.push_back({ {panelX + 1, optY + m_itemHeight - 1, 0, 1}, {colOpt[0],colOpt[1],colOpt[2],colOpt[3]} });
            panelCmd.indices.push_back(baseOpt + 0); panelCmd.indices.push_back(baseOpt + 1); panelCmd.indices.push_back(baseOpt + 2);
            panelCmd.indices.push_back(baseOpt + 0); panelCmd.indices.push_back(baseOpt + 2); panelCmd.indices.push_back(baseOpt + 3);
        }
    }

private:
    std::vector<std::string> m_options;
    int m_selectedIndex;
    float m_itemHeight;
    int m_maxVisibleItems;
    bool m_isOpen;
    OptionCallback m_onChange;
};

// ------------------------------------------------------------------
// UIProgressBar
// ------------------------------------------------------------------
class UIProgressBar : public UIRect
{
public:
    UIProgressBar(UIElement* parent = nullptr) : UIRect(parent), m_value(0.0f), m_fillColor(0xFF00FF00) {}

    void setValue(float v)
    {
        m_value = std::clamp(v, 0.0f, 1.0f);
    }

    float getValue() const { return m_value; }
    void setFillColor(uint32_t color) { m_fillColor = color; }

    void generateCommands(std::vector<UIDrawCommand>& commands, int layer) override
    {
        UIRect::generateCommands(commands, layer);
        if (m_value <= 0.0f) return;

        int fillLayer = layer + 1;
        if (commands.size() <= (size_t)fillLayer) commands.resize(fillLayer + 1);
        auto& cmd = commands[fillLayer];

        float fillWidth = m_width * m_value;
        float x0 = m_absX;
        float y0 = m_absY;
        float x1 = x0 + fillWidth;
        float y1 = y0 + m_height;

        uint32_t base = (uint32_t)cmd.vertices.size();
        auto col = toFloat4(m_fillColor);
        cmd.vertices.push_back({ {x0, y0, 0, 1}, {col[0], col[1], col[2], col[3]} });
        cmd.vertices.push_back({ {x1, y0, 0, 1}, {col[0], col[1], col[2], col[3]} });
        cmd.vertices.push_back({ {x1, y1, 0, 1}, {col[0], col[1], col[2], col[3]} });
        cmd.vertices.push_back({ {x0, y1, 0, 1}, {col[0], col[1], col[2], col[3]} });
        cmd.indices.push_back(base + 0); cmd.indices.push_back(base + 1); cmd.indices.push_back(base + 2);
        cmd.indices.push_back(base + 0); cmd.indices.push_back(base + 2); cmd.indices.push_back(base + 3);
    }

private:
    float m_value;
    uint32_t m_fillColor;
};

// ------------------------------------------------------------------
// UIManager (updated to support new layout)
// ------------------------------------------------------------------
class UIManager
{
public:
    UIManager() : m_root(nullptr), m_activeID(0), m_hoveredID(0), m_screenWidth(0), m_screenHeight(0) {}

    void setRoot(UIElement* r)
    {
        delete m_root;
        m_root = r;
        if (m_root) m_root->setManager(this);
        m_activeID = 0;
        m_hoveredID = 0;
        m_elementMap.clear();
        registerElement(m_root);
    }

    void setScreenSize(uint32_t width, uint32_t height)
    {
        m_screenWidth = width;
        m_screenHeight = height;
    }

    void updateLayout()
    {
        if (m_root)
        {
            // Root element uses entire screen as its parent content area
            m_root->updateLayout(0, 0, (float)m_screenWidth, (float)m_screenHeight);
        }
    }

    void render(std::vector<UIVertex>& v, std::vector<uint32_t>& i)
    {
        if (m_root) m_root->generateVertices(v, i);
    }

    void render(std::vector<UIDrawCommand>& commands)
    {
        if (m_root) m_root->generateCommands(commands, 0);
    }

    void onMouseDown(float x, float y, int b)
    {
        updateLayout();
        UIElement* hit = m_root ? m_root->hitTest(x, y) : nullptr;
        if (hit)
        {
            m_activeID = hit->getID();
            UIEvent e{ UIEventType::MouseDown, x, y, b, false };
            hit->onEvent(e);
        }
    }

    void onMouseUp(float x, float y, int b)
    {
        updateLayout();
        if (m_activeID != 0)
        {
            UIElement* active = getElementByID(m_activeID);
            if (active)
            {
                UIEvent e{ UIEventType::MouseUp, x, y, b, false };
                active->onEvent(e);
            }
            m_activeID = 0;
        }
        else
        {
            UIElement* hit = m_root ? m_root->hitTest(x, y) : nullptr;
            if (hit)
            {
                UIEvent e{ UIEventType::MouseUp, x, y, b, false };
                hit->onEvent(e);
            }
        }
    }

    void onMouseMove(float x, float y)
    {
        updateLayout();
        UIEvent e{ UIEventType::MouseMove, x, y, 0, false };
        if (m_activeID != 0)
        {
            UIElement* active = getElementByID(m_activeID);
            if (active) active->onEvent(e);
        }
        else
        {
            UIElement* hit = m_root ? m_root->hitTest(x, y) : nullptr;
            if (hit) hit->onEvent(e);
            uint32_t newHover = hit ? hit->getID() : 0;
            if (newHover != m_hoveredID) m_hoveredID = newHover;
        }
    }

    void registerElement(UIElement* elem)
    {
        if (!elem) return;
        m_elementMap[elem->getID()] = elem;
        for (auto child : elem->getChildren()) registerElement(child);
    }

    void unregisterElement(uint32_t id) { m_elementMap.erase(id); }
    UIElement* getElementByID(uint32_t id) const
    {
        auto it = m_elementMap.find(id);
        return (it != m_elementMap.end()) ? it->second : nullptr;
    }

private:
    UIElement* m_root;
    uint32_t m_activeID;
    uint32_t m_hoveredID;
    std::unordered_map<uint32_t, UIElement*> m_elementMap;
    uint32_t m_screenWidth, m_screenHeight;
};