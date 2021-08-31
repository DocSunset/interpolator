#include "program.h"
#include <cassert>
#include <iostream>
#include <cstdlib>

namespace GL::Boilerplate
{
    GLuint create_program(const char * name, GLuint vert, GLuint frag)
    {
        assert(vert != 0);
        assert(frag != 0);
    
        GLuint program = glCreateProgram();
        if (not program)
        {
            std::cerr << "Couldn't create shader program: " << name << "\n";
            return 0;
        }
        
        glAttachShader(program, vert);
        glAttachShader(program, frag);
        glLinkProgram(program);
    
        GLint linked = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (not linked)
        {
            std::cerr << "Shader linking failed: " << name << "\n";
            GLint logLength = 0;
            glGetShaderiv(program, GL_INFO_LOG_LENGTH, &logLength);
            std::cerr << "Log length is " << logLength << "\n";
            if (logLength != 0)
            {
                GLchar * errLog = (GLchar*)malloc(logLength);
                if (errLog)
                {
                    glGetProgramInfoLog(program, logLength, &logLength, errLog);
                    std::cerr << errLog << "\n";
                    free(errLog);
                }
                else std::cerr << "Couldn't malloc program log.\n";
            }
        
            glDeleteProgram(program);
            return 0;
        }
    
        glDeleteShader(vert);
        glDeleteShader(frag);
        return program;
    }
}
