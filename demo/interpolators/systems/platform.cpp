#include "platform.h"

#include <SDL.h>

namespace System
{

    class PlatformImplementation
    {
        SDL_Window * window;
        SDL_GLContext gl;

    public:
        // standard SDL setup boilerplate
        PlatformImplementation()
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
                    , 500 , 500
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
        }

        // this should arguably delete the window and so on, but since the app is quitting...
        ~PlatformImplementation() { }

        void run(entt::registry& registry)
        {
        }
    };
    
    Platform::Platform()
    {
        pimpl = new PlatformImplementation;
    }
    
    Platform::~Platform()
    {
        delete pimpl;
    }

    void Platform::run(entt::registry& registry)
    {
        pimpl->run(registry);
    }

}

