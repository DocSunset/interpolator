# Interpolator View

This system draws a coloured representation of the interpolated output space
using the OpenGL implementation of the active interpolator. It maintains GL
data arrays that reflect the data fed to the interpolator and registers a GL
program that when run will draw the visualisation.

```cpp
// @#'demo/interpolators/
