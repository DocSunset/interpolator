# Drawing

The drawing routine is deeply embedded in several layers of book keeping:
finding the active interpolator within the tuple, timing the drawing routine,
and finally the drawing routine itself.

The main loop of the program clears the screen, draws, and then presents the
drawing. Only one interpolator is drawn at a time (the active interpolator),
and the main loop is responsible for extracting it.

In order to find the `active_interpolator`, it is necessary to iterate over the
tuple of interpolators using a C++17 fold expression in a generic lambda that
calls the draw function and is applied over the whole tuple; this is the
best way I could think of to iterate over the tuple, and is necessary as a
consequence of the decision to use a tuple and avoid runtime polymorphism.
This is by no means the only way to achieve this (there are probably better
ways), but it's convenient not to have to write a base class for interpolators
at the stage of development I'm currently at. 

The iteration happens in the main loop. An index `i` initialized to 0. Each
call to the draw function is passed this index, and one interpolator. If the
index doesn't match the `active_interpolator`, then the function returns
without doing anything. In this way only the active interpolator is drawn.

```cpp
// @='draw the active interpolator'
unsigned int i = 0;
auto draw = [](unsigned int i, auto& tuple) 
        {if (i++ == ui.active_interpolator()) ui.draw(i, tuple, demo);};
std::apply([&](auto& ... tuples) {((draw(i, tuples)), ...);}, interpolators);
// @/
```

The drawing routine itself is couched in another layer, which simply runs a
timer while the drawing takes place to provide a simple performance benchmark.

```cpp
// @='run the timer and draw'
@{a flag to keep track of static demos optimization}
auto start = std::chrono::high_resolution_clock::now();

for (unsigned int col = 0; col < _texture.cols(); ++col)
{
    for (unsigned int row = 0; row < _texture.rows(); ++row)
    {
        @{draw one pixel}
    }
}

auto stop = std::chrono::high_resolution_clock::now();
auto usec = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
std::cout << i << ": Generated " << _texture.cols() * _texture.rows() << " interpolations in " << usec << " microseconds\n" 
        << "About " << 1000000 * _texture.cols() * _texture.rows() / usec << " interpolations per second" 
        << std::endl;
// @/
```

Finally, the drawing routine itself:

```cpp
// @='draw one pixel'
RGBVec out = {0, 0, 0};
auto q = Vec2{col/(Scalar)_texture.cols(), row/(Scalar)_texture.rows()};
JzAzBzVec interpolated_jab{0, 0, 0};

interpolator.query(q, demo, para, meta, interpolated_jab);

if (contour_lines) 
{
    @{draw contour lines}
}
else out = JzAzBz_to_RGB(interpolated_jab);

_texture(row, col) = RGBAVec{out.x(), out.y(), out.z(), 1};
// @/
```

The default drawing mode simply draws the output of the interpolator (converted
to RGB). An alternative drawing mode shows contour lines that aim to directly
illustrate the topology of the weights used to interpolate between presets.
This kind of visualization is akin to a terrain map where instead of
illustrating elevation of the terrain, the contours illustrate the weight of a
preset. For guidance in reading the contour map, consider the following:

- lines of the same color refer to the weight of the same preset
- points on the sharp edge of a certain contour line all have the same weight
- the change in weight from one line to the next is always the same, given by
  `1/contour_lines`
  (10 by default)
- lines spaced close together represent a rapid change in weight
- lines spaced far apart represent a gradual change
- when a preset maxes out so that no other presets are involved in the output,
  a grid of dots is drawn on top of the contour peak
- it's usually best to focus on contour lines of one colour at a time

Contour lines are drawn if the number of contour lines requested in the context
structure is greater than zero. Additionally, if there is a demonstration
selected (`ui.grabbed` is not `nullptr`), contour lines are not drawn for
other demonstrations; note that in such cases the for loop over demonstrations
is broken after one pass, which itself doesn't even refer to the iteration
index.

The brightness calculation for contour lines is empirically set to look good
and aid clear intuition; there is no special reason the lines are drawn in
this particular way.

```cpp
// @='draw contour lines'
for (unsigned int n = 0; n < demo.size(); ++n)
{
    RGBVec rgb;
    Scalar w;
    if (grabbed) 
    {
        rgb = JzAzBz_to_RGB(grabbed->p);
        w = meta[grabbed_idx].w;
    }
    else 
    {
        rgb = JzAzBz_to_RGB(demo[n].p); 
        w = meta[n].w;
    }
    if (w >= 1.0 - std::numeric_limits<Scalar>::min() * 5)
    {
        // visualize maximum elevation with inverted colour dots
        out = (col % 3) + (row % 3) == 0 ? RGBVec{1,1,1} - rgb : rgb;
    }
    else
    {
        Scalar brightness = std::pow(std::fmod(w * contour_lines, 1.0f), 8);
        brightness = brightness * w;
        out += rgb * brightness;
    }
    if (grabbed) break;
}
// @/
```
