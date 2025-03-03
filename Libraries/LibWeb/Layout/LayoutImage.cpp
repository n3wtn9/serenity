/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <LibGUI/Painter.h>
#include <LibGfx/Font.h>
#include <LibGfx/ImageDecoder.h>
#include <LibGfx/StylePainter.h>
#include <LibWeb/Layout/LayoutImage.h>

namespace Web {

LayoutImage::LayoutImage(const Element& element, NonnullRefPtr<StyleProperties> style, const ImageLoader& image_loader)
    : LayoutReplaced(element, move(style))
    , m_image_loader(image_loader)
{
}

LayoutImage::~LayoutImage()
{
}

int LayoutImage::preferred_width() const
{
    auto* decoder = m_image_loader.image_decoder();
    return node().attribute(HTML::AttributeNames::width).to_int().value_or(decoder ? decoder->width() : 0);
}

int LayoutImage::preferred_height() const
{
    auto* decoder = m_image_loader.image_decoder();
    return node().attribute(HTML::AttributeNames::height).to_int().value_or(decoder ? decoder->height() : 0);
}

void LayoutImage::layout(LayoutMode layout_mode)
{
    if (preferred_width() && preferred_height()) {
        set_has_intrinsic_width(true);
        set_has_intrinsic_height(true);
        set_intrinsic_width(preferred_width());
        set_intrinsic_height(preferred_height());
    } else if (renders_as_alt_text()) {
        auto& image_element = to<HTMLImageElement>(node());
        auto& font = Gfx::Font::default_font();
        auto alt = image_element.alt();
        if (alt.is_empty())
            alt = image_element.src();
        set_width(font.width(alt) + 16);
        set_height(font.glyph_height() + 16);
    } else {
        set_width(16);
        set_height(16);
    }

    LayoutReplaced::layout(layout_mode);
}

void LayoutImage::paint(PaintContext& context, PaintPhase phase)
{
    if (!is_visible())
        return;

    // FIXME: This should be done at a different level. Also rect() does not include padding etc!
    if (!context.viewport_rect().intersects(enclosing_int_rect(absolute_rect())))
        return;

    LayoutReplaced::paint(context, phase);

    if (phase == PaintPhase::Foreground) {
        if (renders_as_alt_text()) {
            auto& image_element = to<HTMLImageElement>(node());
            context.painter().set_font(Gfx::Font::default_font());
            Gfx::StylePainter::paint_frame(context.painter(), enclosing_int_rect(absolute_rect()), context.palette(), Gfx::FrameShape::Container, Gfx::FrameShadow::Sunken, 2);
            auto alt = image_element.alt();
            if (alt.is_empty())
                alt = image_element.src();
            context.painter().draw_text(enclosing_int_rect(absolute_rect()), alt, Gfx::TextAlignment::Center, style().color_or_fallback(CSS::PropertyID::Color, document(), Color::Black), Gfx::TextElision::Right);
        } else if (m_image_loader.bitmap()) {
            context.painter().draw_scaled_bitmap(enclosing_int_rect(absolute_rect()), *m_image_loader.bitmap(), m_image_loader.bitmap()->rect());
        }
    }
}

bool LayoutImage::renders_as_alt_text() const
{
    if (is<HTMLImageElement>(node()))
        return !m_image_loader.image_decoder();
    return false;
}

void LayoutImage::set_visible_in_viewport(Badge<LayoutDocument>, bool visible_in_viewport)
{
    m_image_loader.set_visible_in_viewport(visible_in_viewport);
}

}
