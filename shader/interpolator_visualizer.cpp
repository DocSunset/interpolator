#include "interpolator_visualizer.h"
#include <cstring>
#include <string>

namespace Shader::Interpolator
{
    char * assemble_shader(const char * interp_func, int s, int p, int r, int u)
    {
        std::string source
            = std::string(define_s) + std::to_string(s)
            + std::string(define_p) + std::to_string(p)
            + ((r > 0) ? (std::string(define_r) + std::to_string(r)) : std::string(""))
            + std::string(variables)
            + std::string(loader)
            + std::string(interp_func)
            + std::string(main)
            ;

        return strdup(source.c_str());
    }
}
