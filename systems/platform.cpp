#include "platform.h"

#include <GLES3/gl3.h>
#include <SDL.h>
#include <SDL_video.h>
#include "components/quit_flag.h"
#include "components/window.h"
#include "components/mouse_button.h"
#include "components/modifier_keys.h"
#include "gl/ll.h"

namespace System
{

    class Platform::PlatformImplementation
    {
        SDL_Window * window;
        SDL_GLContext gl;
        Component::Window win_size;
        entt::registry::entity_type window_entity, mouse_left_button_entity;

    public:
        // standard SDL setup boilerplate
        PlatformImplementation(entt::registry& registry)
            : win_size{500, 500}
        {
            // no audio yet...
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
                    , SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
                    );

            if (window == nullptr)
            {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                        "Error creating window:\n    %s\n", 
                        SDL_GetError());
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
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
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
                        "Couldn't create OpenGL context :(", NULL);
                exit(EXIT_FAILURE);
            }
            else SDL_Log("Created GL context\n");

            SDL_GL_SetSwapInterval(1); // should check for errors
            
            // create platform entities
            window_entity = registry.create();
            registry.emplace<Component::Window>(window_entity, 500, 500);
            mouse_left_button_entity = registry.create();
            registry.emplace<Component::LeftMouseButton>(mouse_left_button_entity
                    , false
                    , 0
                    , Component::Position{0,0}
                    );
            
            // set initial context
            registry.set<Component::ShiftModifier>(false);
        }

        // this should arguably delete the window and so on, but since the app is quitting...
        ~PlatformImplementation() { }

        void quit(entt::registry& registry)
        {
            registry.set<Component::QuitFlag>(true);
        }

        void poll_events(entt::registry& registry)
        {
            auto win = registry.get<Component::Window>(window_entity);
            SDL_Event ev;
            while (SDL_PollEvent(&ev)) switch (ev.type)
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
                    win_size = {(unsigned int)ev.window.data1, (unsigned int)ev.window.data2};
                    registry.replace<Component::Window>(window_entity , win_size.w , win_size.h);
                    glViewport(0, 0 , win_size.w , win_size.h);
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    quit(registry);
                    break;
                }
                break;
           case SDL_MOUSEMOTION:
                if (ev.motion.state == 0)
                {
                }
                else 
                {
                    if ((ev.motion.state & SDL_BUTTON_LMASK) != 0)
                    {
                    }
                    if ((ev.motion.state & SDL_BUTTON_RMASK) != 0)
                    {
                    }
                    if ((ev.motion.state & SDL_BUTTON_MMASK) != 0)
                    {
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                switch (ev.button.button)
                {
                    case SDL_BUTTON_LEFT:
                    {
                        auto current = registry.get<Component::LeftMouseButton>(mouse_left_button_entity);
                        registry.replace<Component::LeftMouseButton>
                                ( mouse_left_button_entity
                                , true
                                , ev.button.clicks
                                , Component::Position
                                    { float(ev.button.x) - win_size.w/2.0f
                                    , win_size.h/2.0f - float(ev.button.y)
                                    }
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
                        auto current = registry.get<Component::LeftMouseButton>(mouse_left_button_entity);
                        registry.replace<Component::LeftMouseButton>
                                ( mouse_left_button_entity
                                , false
                                , ev.button.clicks
                                , current.down_position
                                , Component::Position
                                    { float(ev.button.x) - win_size.w/2.0f
                                    , win_size.h/2.0f - float(ev.button.y)
                                    }
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
            }
        }

        void run(entt::registry& registry)
        {
            poll_events(registry);

            SDL_GL_SwapWindow(window);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        void * get_window() const
        {
            return window;
        }
    };
    
    Platform::Platform(entt::registry& registry)
    {
        pimpl = new PlatformImplementation(registry);
    }
    
    Platform::~Platform()
    {
        delete pimpl;
    }

    void Platform::run(entt::registry& registry)
    {
        pimpl->run(registry);
    }

    void * Platform::window() const
    {
        return pimpl->get_window();
    }

}

