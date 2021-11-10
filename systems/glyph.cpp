#include "glyph.h"
#include "components/glyph.h"
#include "components/window.h"
#include "gl/vertex_array.h"

#include "shader/circle.h"

namespace
{
    struct GlyphAttributes
    {
        float radius; // pixels
        float center_point[2];
        float fill_color[4];
    };

    void prepare_texture(entt::registry& registry, entt::entity entity)
    {
        // prepare a texture
        registry.emplace_or_replace<GlyphAttributes>(entity,
                GlyphAttributes
                { 50
                , {0,0} // to be updated during run phase
                , {1,1,1,1}
                });
    }
}

namespace System
{
    struct Glyph::Implementation
    {
        GL::LL::Program program;
        GL::VertexAttributeArray array;
        GL::LL::VertexArray vao;
        GL::LL::Buffer vbo;

        void window_uniform(Component::Window& win)
        {
            program.use();
            auto prog = program.gl_handle();
            auto window = glGetUniformLocation(prog, "window");
            glUniform2f(window, win.w, win.h);
        }

        void update_window(entt::registry& registry, entt::registry::entity_type entity)
        {
            Component::Window win = registry.get<Component::Window>(entity);
            window_uniform(win);
        }

        Implementation()
            : program{vertex_shader, fragment_shader}
            , array{program}
            , vao{}
            , vbo(GL::LL::Buffer::Target::ARRAY, GL::LL::Buffer::Usage::DYNAMIC_DRAW)
        {
            if (GL::LL::any_error()) GL::LL::error_print("glyph painter post-init gl error\n");
            auto vaobind = bind(vao);
            auto vbobind = bind(vbo);
            const auto& attributes = array.attributes();

            for (int i = 0; i < attributes.size(); ++i)
                vaobind.enable_attrib_pointer(attributes, attributes.index_of(attributes[i].name()));
        }

        void setup_reactive_systems(entt::registry& registry)
        {
            registry.on_update<Component::Window>().connect<&Implementation::update_window>(*this);
            registry.on_construct<Component::Glyph>().connect<&prepare_texture>();
        }

        void prepare_registry(entt::registry& registry)
        {
            auto view = registry.view<Component::Window>();
            assert(view.size() == 1);
            Component::Window win = **(view.raw());
            window_uniform(win);

            auto entity = registry.create();
            registry.emplace<Component::Glyph>(entity
                    , "test"
                    , Component::Position{0,0}
                    );
        }

        void run(entt::registry& registry)
        {
            //// should be using an observer
            //auto view = registry.view<Component::Glyph>();
            //for (auto&& [entity, glyph] : view.each())
            //{
            //    auto& attribs = registry.get<GlyphAttributes>(entity);
            //    attribs.center_point[0] = glyph.center_point.x;
            //    attribs.center_point[1] = glyph.center_point.y;
            //}

            //// actually draw the glyphs
            auto view = registry.view<GlyphAttributes>();

            auto size = view.size();
            if (size == 0) return;

            auto * glyphs = *(view.raw());

            auto buffbind = bind(vbo);
            buffbind.buffer_data(size * sizeof(GlyphAttributes), glyphs);

            // draw glyphs
            GL::LL::any_error();
            program.use();
            auto vaobind = bind(vao);
            glDrawArrays(GL_POINTS, 0, size);
        }
    };

    /* pimpl boilerplate *****************************************/

    Glyph::Glyph()
    {
        pimpl = new Implementation();
    }

    void Glyph::setup_reactive_systems(entt::registry& registry)
    {
        pimpl->setup_reactive_systems(registry);
    }

    void Glyph::prepare_registry(entt::registry& registry)
    {
        pimpl->prepare_registry(registry);
    }

    Glyph::~Glyph()
    {
        free(pimpl);
    }
    
    void Glyph::run(entt::registry& registry)
    {
        pimpl->run(registry);
    }
}
