#pragma once

#include <GLES3/gl3.h>

/* low level GL interface
 *
 * This level is meant to serve as a very thin wrapper over OpenGL ES 3.0
 * providing a reasonably one-to-one interface to the latter, with type safety,
 * lifetime management, and automatic error checking that is disabled in
 * release-mode builds.
 */

#include "ll/shader.h"
#include "ll/program.h"
#include "ll/buffer.h"
#include "ll/vertex_array.h"
#include "ll/attributes.h"
#include "ll/attribute_manifest.h"
#include "ll/error.h"
