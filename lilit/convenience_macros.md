There are a few macros defined in `interpolators.h` to make it a bit
easier to implement the interpolation algorithms with a consistent interface.

The first two simply propagate the member-types of the template parameter
(either the Demo type of an Interpolator or the Interpolator type of a
ShaderInterpolator). Note that these two should be followed by a semicolon,
since it's more comfortable if they appear to be some kind of statement:

```cpp
// @+'interpolator convenience macros'
#define USING_INTERPOLATOR_DEMO_TYPES \
using Demo = Demonstration; \
using Scalar = typename Demo::Scalar; \
using SVector = typename Demo::SVector; \
using PVector = typename Demo::PVector

#define USING_INTERPOLATOR_TYPES \
using Demo = Interpolator::Demo; \
using Scalar = typename Interpolator::Scalar; \
using SVector = typename Interpolator::SVector; \
using PVector = typename Interpolator::PVector; \
using Meta = typename Interpolator::Meta; \
using Para = typename Interpolator::Para
// @/
```

The last few go together. The first opens a struct definition and declares
access operators. The caller has to give it a `Name` argument that should
refer to an array of strings with at least 'N' members. It's expected the
user of the library may iterate (using a plain old index-based for loop)
over the parameters and their names. 

The second adds accessor methods for getting at the data using semantic names
in calling code.

The third is just a closing curly brace and semicolon, provided as a reminder
not to include an opening brace after the first macro in the group, and to
instead use the first and third as a matching pair to enclose the struct
definition. Behold the discomfort of the C preprocessor.

I'm open to suggestions on better ways of achieving this...

```cpp
// @+'interpolator convenience macros'
#define INTERPOLATOR_PARAMETER_STRUCT_START(Names, N) \
struct Para \
{ \
    const Scalar& operator[] (std::size_t n) const {return data[n];} \
          Scalar& operator[] (std::size_t n)       {return data[n];} \
    const char * name(std::size_t n) {return Names[n];} \
    std::size_t size() {return N;} \
    Scalar data[N];

#define INTERPOLATOR_PARAM_ALIAS(name, idx)\
const Scalar& name() const {return data[idx];} \
      Scalar& name()       {return data[idx];}

#define INTERPOLATOR_PARAMETER_STRUCT_END };
// @/
```
