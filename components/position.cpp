#include "position.h"
#include "utility/random.h"

namespace Component
{
    Position Position::Random()
    {
        return {Utility::brandf() * 100, Utility::brandf() * 100};
    }
}
