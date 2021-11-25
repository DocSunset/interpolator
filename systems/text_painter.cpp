#include "text_painter.h"
#include "components/text.h"
#include "components/window.h"
#include "gl/ll/attribute_manifest.h"
#include "gl/ll/buffer.h"
#include "gl/ll/vertex_array.h"
#include "gl/ll/error.h"
#include "artery-font/artery-font.h"
#include "artery-font/std-artery-font.h"
#include "artery-font/stdio-serialization.h"
#include <iostream>
#include <numeric>

#include "shader/text.h"

namespace
{
    struct Glyph
    {
        //float color[4];
        //float border[4];

        // in normalized screen coordinates, the size and position of the
        // rectangle on screen in which this glyph fits
        struct BoundingBox
        {
            float left;
            float bottom;
            float right;
            float top;
        } bbox;

        // the normalized texture coordinates for the glyph in the font atlas
        struct TextureBox
        {
            float left;
            float bottom;
            float right;
            float top;
        } tbox;

        //float border_thickness;
        //float border_transition;
        //float blur_radius;

        int _id;
    };
}

namespace System
{
    struct TextPainter::Implementation
    {
        artery_font::StdArteryFont<float> font;
        GL::LL::Program program;
        GL::LL::VertexArray vao;
        GL::LL::Buffer attrib_buffer;
        std::vector<Glyph> glyph;
        Component::Window window;

        void update_window(entt::registry& registry, entt::registry::entity_type entity)
        {
            window = registry.get<Component::Window>(entity);
        }
        
        Implementation()
            : program{vertex_shader, fragment_shader}
            , vao{}
            , attrib_buffer(GL::LL::Buffer::Target::ARRAY, GL::LL::Buffer::Usage::DYNAMIC_DRAW)
        {
            artery_font::readFile(font, "resources/fira-code.arfont");
            if (font.variants.length() == 0) std::cout << "Error reading font file. Imminent crash likely.\n";
            // set up font for easy reading

            if (GL::LL::any_error()) GL::LL::error_print("text painter post-init gl error\n");
            const auto& attributes = GL::LL::AttributeManifest(program);

            auto vaobind = bind(vao);

            auto abobind = bind(attrib_buffer);
            for (int i = 0; i < attributes.size(); ++i)
            {
                vaobind.enable_attrib_pointer(attributes, attributes.index_of(attributes[i].name()));
                if (i == attributes.size() - 1) continue;
                // The last attribute is a_vertex_id; it is not an instanced
                // attribute, and is the same for every instance of the text
                // primitive. It is used to determine which vertex is currently
                // processed by the vertex shader so that it can be positioned
                // appropriately by the vertex shader. A geometry shader would be
                // preferable, but these are not available in the subset of OpenGL
                // ES 3 available on the web, which we intend to target.
                vaobind.attrib_divisor(attributes, attributes.index_of(attributes[i].name()), 1);
            }
        }
        
        artery_font::Glyph<float> get_glyph(char codepoint)
        {
            auto& aglyphs = font.variants[0].glyphs;
            for (auto& ag : aglyphs.vector) if (codepoint == ag.codepoint) return ag;
            return font.variants[0].glyphs[0];
        }
    };

    void TextPainter::construct_system()
    {
        pimpl = new Implementation();
    }

    void TextPainter::setup_reactive_systems(entt::registry& registry)
    {
        registry.on_update<Component::Window>().connect<&Implementation::update_window>(*(this->pimpl));
    }

    void TextPainter::prepare_registry(entt::registry& registry)
    {
        auto tester = registry.create();
        auto view = registry.view<Component::Window>();
        assert(view.size() == 1);
        pimpl->window = **(view.raw());

        registry.emplace<Component::Text>(tester, "This is a test. 😊", Component::Color{1,1,1,0.1}, 0.0f, 0.0f, 100000.0f, 100.0f);

        for (Glyph i{}; i._id < 4; ++i._id)
        {
            // these glyphs are never seen, but their ID values are used by all
            pimpl->glyph.push_back(i);
        }
    }

    void TextPainter::prepare_to_paint(entt::registry& registry)
    {
        // make glyphs for every character in every text on screen
        auto view = registry.view<Component::Text>();
        auto size = view.size();
        if (size == 0) return;

        auto * texts = *(view.raw());

        std::size_t num_codepoints = std::transform_reduce(texts, texts + size, 0, 
                std::plus(), 
                [&](auto text){return text.string.size();});
        num_codepoints += 4; // for the invisible ID serving glyphs
        if (num_codepoints > pimpl->glyph.capacity())
            pimpl->glyph.resize(num_codepoints);

        auto font = pimpl->font.variants[0];
        auto image_w = pimpl->font.images[0].width;
        auto image_h = pimpl->font.images[0].height;
        auto& glyph = pimpl->glyph;
        auto& window = pimpl->window;
        std::size_t i = 4; // for the invisible ID serving glyphs
        std::for_each(texts, texts + size, [&](auto& text)
        {
            auto line_width = text.right - text.left;
            auto line_height = text.font_size * font.metrics.lineHeight;
            float line_position = 0;
            float line_number = 1;
            for (auto codepoint : text.string)
            {
                auto artery_glyph = pimpl->get_glyph(codepoint);
                auto advance = text.font_size * artery_glyph.advance.h;
                if (line_position + advance > line_width)
                {
                    line_position = 0;
                    ++line_number;
                }
                float left = text.left + line_position;
                float bottom = text.top - (line_number * line_height);
                if (bottom < text.bottom)
                {
                    #ifdef DEBUG
                        std::cout 
                            << "warning: ran out of space when line-breaking a text:\n"
                            << text.string << "\n";
                    #endif
                    break; // ran out of space
                }
                glyph[i] = Glyph
                        //{ { text.color.r, text.color.g, text.color.b, text.color.a }
                        { { (left   + artery_glyph.planeBounds.l * text.font_size) / (window.w / 2.0f)
                          , (bottom + artery_glyph.planeBounds.b * text.font_size) / (window.h / 2.0f)
                          , (left   + artery_glyph.planeBounds.r * text.font_size) / (window.w / 2.0f)
                          , (bottom + artery_glyph.planeBounds.t * text.font_size) / (window.h / 2.0f)
                          }
                        , { artery_glyph.imageBounds.l / image_w
                          , artery_glyph.imageBounds.b / image_h
                          , artery_glyph.imageBounds.r / image_w
                          , artery_glyph.imageBounds.t / image_h
                          }
                        };
                line_position += advance;
                ++i;
            }
        });
    }

    void TextPainter::paint(entt::registry& registry)
    {
        auto size = pimpl->glyph.size();
        Glyph * glyphs = pimpl->glyph.data();
        if (size == 0) return;
        for (auto& glyph : pimpl->glyph)
        {
            std::cout << glyph.bbox.left << " " << glyph.bbox.bottom << " " << glyph.bbox.right << " " << glyph.bbox.top << "\n";
        }
        std::cout << "\n";

        auto buffbind = bind(pimpl->attrib_buffer);
        buffbind.buffer_data(size * sizeof(Glyph), glyphs);

        // draw lines
        GL::LL::any_error();
        pimpl->program.use();
        auto vaobind = bind(pimpl->vao);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, size);
    }

    TextPainter::~TextPainter()
    {
        free(pimpl);
    }
}
