.PHONY: rust python c

all : rust python c


rust:
	cargo build --release --manifest-path=rust/Cargo.toml


python :
	python -m compileall python

c:
	gcc  -Ofast c/mandelbrot.c -o c/mandelbrot -lpng -lpthread

test: all
	echo "Running test, this takes several minutes..."
	./test.sh

picture  : all
	rust/target/release/mandelbrot rust_fractal.png 2000x2000 -0.5,0.5 -0.3,0.7 8
	python python/mandelbrot.pyc py_fractal.png 2000x2000 -0.5,0.5 -0.3,0.7 8
	c/mandelbrot c_fractal.png 2000x2000 -0.5,0.5 -0.3,0.7 8

clean :
	rm *.png ||:
	cargo clean --manifest-path=rust/Cargo.toml
	rm python/*.pyc ||:
	rm c/mandelbrot  ||:
	rm test_results.csv ||:
