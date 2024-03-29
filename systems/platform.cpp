#include "platform.h"

#include <chrono>
#include <GLES3/gl3.h>
#include <SDL.h>
#include <SDL_log.h>
#include <SDL_mouse.h>
#include <SDL_video.h>
#include <mapper/mapper_cpp.h>
#include "SDL_events.h"
#include "components/quit_flag.h"
#include "components/window.h"
#include "components/mouse_button.h"
#include "components/mouse_motion.h"
#include "components/mouse_mode.h"
#include "components/mouse_position.h"
#include "components/modifier_keys.h"
#include "components/paint_flag.h"
#include "components/repaint_timer.h"
#include "components/circle.h"
#include "components/time.h"
#include "components/argcv.h"
#include "components/save_file.h"
#include "common/vis.h"
#include "gl/ll.h"
#include "utility/mtof.h"

namespace
{
}

namespace System
{

    class Platform::Implementation
    {
        unsigned int last_time = 0;
        SDL_Window * window;
        SDL_GLContext gl;
        Component::Window win_size;
        entt::registry::entity_type window_entity, mouse_entity;

    public:
        // standard SDL setup boilerplate
        Implementation(bool testing)
            : win_size{500, 500}
        {
            if (SDL_Init(SDL_INIT_VIDEO) != 0)
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                        "Error initializing SDL:\n    %s\n", 
                        SDL_GetError());
                exit(EXIT_FAILURE);
            }
            else SDL_Log("Initialized successfully\n");
            
            // ask to use opengl es3 (for web/emscripten compatibility)
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

            // make the window
            window = SDL_CreateWindow
                    ( "Interpolators"
                    , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED
                    , win_size.w, win_size.h
                    , SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | (testing ? 0 : SDL_WINDOW_RESIZABLE)
                    );

            if (window == nullptr)
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                        "Error creating window:\n    %s\n", 
                        SDL_GetError());
                if (not testing) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                        "Couldn't create the main window :(", NULL);
                exit(EXIT_FAILURE);
            }
            else SDL_Log("Created window\n");
            
            // set up opengl context
            gl = SDL_GL_CreateContext(window);
            if (gl == nullptr)
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                        "Error creating OpenGL context:\n    %s\n", 
                        SDL_GetError());
                if (not testing) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                        "Couldn't create OpenGL context :(", NULL);
                exit(EXIT_FAILURE);
            }
            else SDL_Log("Created GL context\n");

            SDL_GL_SetSwapInterval(0); // should check for errors
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        void setup_reactive_systems(entt::registry& registry)
        {
            registry.on_update<Component::RelativeMouseMode>()
                .connect<&Platform::Implementation::on_mouse_mode>(*this);
            registry.on_update<Component::MousePosition>()
                .connect<&Platform::Implementation::on_mouse_position>(*this);
        }

        void prepare_registry(entt::registry& registry)
        {
            window_entity = registry.create();
            registry.set<Component::Window>(500.0f, 500.0f);
            registry.emplace<Component::Window>(window_entity, 500.0f, 500.0f);

            mouse_entity = registry.create();
            registry.emplace<Component::LeftMouseButton>(mouse_entity
                    , false
                    , 0
                    , Component::Position::Zero()
                    );
            registry.emplace<Component::MouseMotion>(mouse_entity
                    , Component::Position::Zero()
                    , Component::Position::Zero()
                    );
            registry.emplace<Component::RelativeMouseMode>(mouse_entity, false);
            registry.emplace<Component::MousePosition>(mouse_entity, Component::Position::Zero());

            // set initial context
            registry.set<Component::ShiftModifier>(false);
            registry.set<Component::Time>(SDL_GetTicks64());

            auto args = registry.ctx<Component::ArgCV>();
            if (args.argc > 1) registry.set<Component::SaveFile>(args.argv[1]);
            else registry.set<Component::SaveFile>();
        }

        // this should arguably delete the window and so on, but since the app is quitting...
        ~Implementation() { }

        void on_mouse_mode(entt::registry& registry, entt::entity entity)
        {
            SDL_SetRelativeMouseMode(
                    registry.get<Component::RelativeMouseMode>(entity) ?
                    SDL_TRUE : SDL_FALSE);
        }

        void on_mouse_position(entt::registry& registry, entt::entity entity)
        {
            auto p = registry.get<Component::MousePosition>(entity);
            SDL_WarpMouseInWindow(window, p.x + win_size.w/2.0f, win_size.h/2.0f - p.y);
        }

        void quit(entt::registry& registry)
        {
            registry.set<Component::QuitFlag>(true);
        }

        void poll_events(entt::registry& registry)
        {
            auto& dev = registry.ctx<mapper::Device>();
            SDL_Event ev;
            dev.poll();
            auto got_event = SDL_WaitEventTimeout(&ev, 10);
            registry.set<Component::Time>(SDL_GetTicks64());
            if (got_event) do
            {
                auto win = registry.get<Component::Window>(window_entity);
            switch (ev.type)
            {
            case SDL_QUIT:
            case SDL_APP_TERMINATING:
            case SDL_APP_LOWMEMORY:
                quit(registry);
                break;

            case SDL_WINDOWEVENT:
                switch (ev.window.event)
                {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    win_size = {(float)ev.window.data1, (float)ev.window.data2};
                    registry.set<Component::Window>(win_size.w, win_size.h);
                    registry.replace<Component::Window>(window_entity, win_size.w, win_size.h);
                    glViewport(0, 0 , win_size.w , win_size.h);
                    registry.ctx<Component::PaintFlag>().set();
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    quit(registry);
                    break;
                }
                break;

            case SDL_MOUSEMOTION:
            {
                auto p = Component::Position
                            { float(ev.motion.x) - win_size.w/2.0f
                            , win_size.h/2.0f - float(ev.motion.y)
                            };
                registry.replace<Component::MouseMotion>
                        ( mouse_entity
                        , p
                        , Component::Position
                            { float(ev.motion.xrel)
                            , -float(ev.motion.yrel)
                            }
                        );
                auto& mouse_position = registry.get<Component::MousePosition>(mouse_entity);
                mouse_position = {p.x, p.y};
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
                switch (ev.button.button)
                {
                    case SDL_BUTTON_LEFT:
                    {
                        auto current = registry.get<Component::LeftMouseButton>(mouse_entity);
                        auto position = Component::Position
                                      { float(ev.button.x) - win_size.w/2.0f
                                      , win_size.h/2.0f - float(ev.button.y)
                                      };
                        registry.replace<Component::LeftMouseButton>(mouse_entity
                                , true
                                , ev.button.clicks
                                , position
                                , current.up_position
                                );
                        break;
                    }
                    case SDL_BUTTON_MIDDLE:
                        break;
                    case SDL_BUTTON_RIGHT:
                        break;

                }
                break;

            case SDL_MOUSEBUTTONUP:
                switch (ev.button.button)
                {
                    case SDL_BUTTON_LEFT:
                    {
                        auto current = registry.get<Component::LeftMouseButton>(mouse_entity);
                        auto position = Component::Position
                                      { float(ev.button.x) - win_size.w/2.0f
                                      , win_size.h/2.0f - float(ev.button.y)
                                      };
                        registry.replace<Component::LeftMouseButton>(mouse_entity
                                , false
                                , ev.button.clicks
                                , current.down_position
                                , position
                                );
                        break;
                    }
                    case SDL_BUTTON_MIDDLE:
                        break;
                    case SDL_BUTTON_RIGHT:
                        break;

                }
                break;
            case SDL_MOUSEWHEEL:
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                switch (ev.key.keysym.sym)
                {
                case SDLK_LSHIFT:
                case SDLK_RSHIFT:
                    bool shift;
                    if (ev.type == SDL_KEYDOWN) shift = true;
                    else shift = false;
                    registry.set<Component::ShiftModifier>(ev.type == SDL_KEYDOWN);
                    break;
                case SDLK_LALT:
                case SDLK_RALT:
                    bool alt;
                    if (ev.type == SDL_KEYDOWN) alt = true;
                    else alt = false;
                    break;
                case SDLK_LCTRL:
                case SDLK_RCTRL:
                case SDLK_LGUI:
                case SDLK_RGUI:
                    bool ctrl;
                    if (ev.type == SDL_KEYDOWN) ctrl = true;
                    else ctrl = false;
                    break;
                // default: generate a key command entity?
                }
                break;
            } // switch (ev.type)
            } while (SDL_PollEvent(&ev));
        }

        void run(entt::registry& registry)
        {
            poll_events(registry);
            auto now = SDL_GetTicks64();
            // repaint roughly 60 frames per second
            if ((now - last_time) > 16)
            {
                registry.ctx<Component::RepaintTimer>().set();
                last_time = now;
            }
        }

        void paint(entt::registry& registry)
        {
            glClear(GL_COLOR_BUFFER_BIT);
        }

        void swap_window() const
        {
            SDL_GL_SwapWindow(window);
        }
    };
    
    void Platform::construct_system()
    {
        pimpl = new Implementation(testing);
    }

    void Platform::setup_reactive_systems(entt::registry& registry)
    {
        pimpl->setup_reactive_systems(registry);
    }

    void Platform::prepare_registry(entt::registry& registry)
    {
        pimpl->prepare_registry(registry);
    }
    
    Platform::~Platform()
    {
        delete pimpl;
    }

    void Platform::run(entt::registry& registry)
    {
        pimpl->run(registry);
    }

    void Platform::paint(entt::registry& registry)
    {
        pimpl->paint(registry);
    }

    void Platform::swap_window() const
    {
        return pimpl->swap_window();
    }

}

