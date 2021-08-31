#pragma once

#include <GLES3/gl3.h>

/* low level GL interface
 *
 * This level is meant to serve as a very thin wrapper over OpenGL ES 3.0
 * providing a direct one-to-one interface to the latter, with type safety and
 * debug-mode-only automatic error checking.
 */

#include "ll/shader.h"
