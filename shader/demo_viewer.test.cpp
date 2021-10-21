#include <GLES3/gl3.h>
#include <catch2/catch.hpp>
#include "test/common.h"
#include "test/shader_common.h"
#include "shader/demo_viewer.h"

TEST_CASE("Shaders", "[shaders]")
{
    PLATFORM_SETUP();
    GL::LL::VertexArray vao{};
    GL::LL::Buffer vbo{GL::LL::Buffer::Target::ARRAY, GL::LL::Buffer::Usage::STATIC_DRAW};
    auto window = (SDL_Window *)platform.window();
    glViewport(0, 0, 500, 500);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(window);

    SECTION("Demo viewer")
    {
        DemoViewerAttributes dots[2] = { { {-100,-100}, {1,1,1,1}, {1,0,0,1} }
                             , { {100,100}, {1,1,1,1}, {0,1,0,1} }
                             };

        GL::LL::Program program{vertex_shader, fragment_shader};
        GL::VertexAttributeArray array{program};
        auto vaobind = bind(vao);
        auto vbobind = bind(vbo);
        const auto& attributes = array.attributes();
        vaobind.enable_attrib_pointer(attributes, attributes.index_of("position"));
        vaobind.enable_attrib_pointer(attributes, attributes.index_of("fill_color_in"));
        vaobind.enable_attrib_pointer(attributes, attributes.index_of("ring_color_in"));
        vbobind.buffer_data(2 * sizeof(DemoViewerAttributes), dots);
        program.use();
        glUniform2f(glGetUniformLocation(program.gl_handle(), "window"), 500, 500);

        auto start = SDL_GetTicks();
        while (SDL_GetTicks() - start < 500)
        {
            glDrawArrays(GL_POINTS, 0, 2);
            SDL_GL_SwapWindow(window);
        }
    }
}
