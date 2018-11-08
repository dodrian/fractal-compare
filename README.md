# fractal-compare

What originally started as an threaded program to generate the Mandelbrot set, an exercise from O'Reilly's *Programming Rust* turned into a performance comparison of different languages.

To compile: `make all`
To generate the pictures: `make picture`
To run the speed comparison: `make test`

The C program uses libpng and pthreads.

Todo:
* See if `gcc` settings can be optimized further
* Research Python optimization
* Fail gracefully from bad inputs
* Another language! (Javascript? Java?)

On my Intel i5-6500 (4 CPUs, 4 cores per CPU) the results were as follows:

![Speed Comparison](/charts/timing_2000_square.png)

Clearly Python isn't nearly as good at heavy computation!  It does improve with more threads though, which indicates it is using parallel processing.

![Rust vs C](/charts/rust_vs_c.png)

Rust is ever-so-slightly faster than C, though it may be a compiler optimization issue.  Both programs benefit significantly from 4 threads, with diminishing returns thereafter.  No speedup after 8 threads.
