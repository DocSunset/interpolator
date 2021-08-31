# Matrix Multiplication as Interpolator

Numerous authors have proposed humble matrix multiplication as a useful
construct for implementing mappings (e.g. Bevilacqua et al, NIME 2005;
Brandstegg et al, NIME 2014; or DeCampo et al, NIME 2014).  Bevilacqua et al
offer a particularly clear and versatile form of this approach, and the
implementation here follows their presentation.

In this technique, as in the other interpolators presented here, the mapping is
defined by the demonstrations from source space to synthesis parameter space.
The demonstrations are embodied in this technique by two matrices, $S_t$ and
$P_t$ where the $t$ subscript denotes that these matrices hold the training
data that defines the mapping matrix.  The training matrices are formed by
concatenating the demonstration data column-wise, i.e. each $s$ vector is
considered as a column vector, and $S_t$ is formed by concatenating them side
by side so that each $s$ forms one column of $S_t$, and similarly for $P_t$ and
each parameter vector $p$.

The mapping is then defined by the equation:

$$$
P_t = M * S_t
$$$

$M$ defines a linear transformation from $n$ to $m$ dimensions, meaning that
its effect can be considered as some hyperdimensional analog to some
combination of stretching, skewing, and rotation.
The demonstration vectors can be augmented by placing appending a $1$ to each
demonstration, or equivalently the training matrices can be augmented by
appending a bottom row of all $1$s; doing so gives a somewhat more versatile
mapping:

$$$
P_t = A * S_t
$$$

where $A$ defines an affine transformation; similar to a linear transformation,
but with the possibility of also translating the coordinate basis.

In either case (we will use $A$), the actual mapping can be determined by
calculating some inverse of $S_t$:

$$$
P_t * S_t^{-1} = A
$$$

We can calculate the inverse of $S_t$ using the singular value decomposition:

$$$
S_t = U * S * V^{t}
S_t^{-1} = U * S^{-1} * V^{t}
A = P_t * U * S^{-1} * V^{t} 
$$$
