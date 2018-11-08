#!/usr/bin/python
# Python comparison to mandlebrot rust example
# (c) Dorian Westacott 2018
# MIT License


import sys
# python threading in cpython does not use multiple cores, instead use multiprocessing
from multiprocessing import Process, Array


def parse_pair(s, separator):
    pair = s.split(separator)
    if len(pair) != 2:
        raise Exception("Error parsing %s" % s)
    try:
        return (int(pair[0]), int(pair[1]))
    except:
        return (float(pair[0]), float(pair[1]))


def pixel_to_point(bounds, pixel, upper_left, lower_right):
    (width, height) = (lower_right[0] - upper_left[0],
                        upper_left[1] - lower_right[1])
    return (upper_left[0] + pixel[0] * width / bounds[0],
            upper_left[1] - pixel[1] * height / bounds[1])


def escapes(c, limit):
    z = complex(0.0,0.0)
    for i in xrange(0, limit):
        z = z * z + c
        if (z.real * z.real + z.imag * z.imag) > 4:
            return i
    return None

def render(pixels, start_row, bounds, upper_left, lower_right):
    assert len(pixels) >= start_row * bounds[0] + bounds[0] * bounds[1]
    for r in xrange(0, bounds[1]):
        for c in xrange(0, bounds[0]):
            point = pixel_to_point(bounds, (c, r), upper_left, lower_right)
            i = start_row * bounds[0] + r * bounds[0] + c
            escape_val = escapes(complex(point[0],point[1]), 255)
            pixels[i] = 255 - escape_val if escape_val is not None else 0

def write_bitmap(filename, pixels, bounds):
    #import here to not slow execution of the benchmark script
    import png
    with open(filename, 'wb') as output:
        encoder = png.Writer(bounds[0], bounds[1], greyscale=True)
        encoder.write(output, [pixels[i:i+bounds[0]] for i in range(0, len(pixels), bounds[0])])



if __name__ == '__main__':
    if len(sys.argv) < 5:
        print "Usage: mandelbrot FILE PIXELS UPPERLEFT LOWERRIGHT [MAX_THREADS [OMIT_DRAWING]]"
        print "Example: %s mandel.png 1000x1000 -2.0,2.0 2.0,-2.0 4 1" % sys.argv[0]
        sys.exit(1)
    bounds = parse_pair(sys.argv[2], 'x')
    upper_left = parse_pair(sys.argv[3], ',')
    lower_right = parse_pair(sys.argv[4], ',')
    #pixels = [None] * bounds[0] * bounds[1]
    pixels = Array('B', bounds[0] * bounds[1], lock=False)
    num_threads = int(sys.argv[5]) if len(sys.argv) >= 6 else 8
    shard_rows = bounds[1] / num_threads + 1
    threads= [None] * num_threads
    for ii in xrange(0, num_threads):
        top =  shard_rows * ii
        height = shard_rows if top + shard_rows < bounds[1] else bounds[1] - shard_rows * ii
        shard_bounds = (bounds[0], height)
        proc_args = (pixels, top, shard_bounds,
                        pixel_to_point(bounds, (0, top), upper_left, lower_right),
                        pixel_to_point(bounds, (bounds[0], top + height), upper_left, lower_right))
        threads[ii] = Process(target=render, args=proc_args)
        threads[ii].start()

    for thread in threads:
        thread.join()

    # omit drawing if OMIT_DRAWING is set
    if len(sys.argv) < 7:
        write_bitmap(sys.argv[1], pixels, bounds)
