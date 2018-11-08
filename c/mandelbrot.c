#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <complex.h>
#include <png.h>
#include <pthread.h>

struct iTuple {
  int x;
  int y;
};

struct dTuple{
  double x;
  double y;
};


struct render_arguments{
  char* pixels;
  struct iTuple bounds;
  struct dTuple upper_left;
  struct dTuple lower_right;
};

// splits string into two, str gets truncated at first index of sep,
// returns second string
// not enough error checking, bad Dorian!
char* split_str(char* str, char sep)
{
  char* str2 = strchr(str, sep);
  if(*str2 == '\0')
  {
    printf("Error parsing string %s\n", str);
    exit(1);
  }
  // set seperator to null char
  *str2 = '\0';
  // move pointer
  str2++;
  return str2;

}

// Convert a pixel in the 2d array bounds to a complex number on the
// complex plane specifed by upper_left and lower_right
struct dTuple pixel_to_point(struct iTuple bounds, struct iTuple pixel,
                                struct dTuple upper_left, struct dTuple lower_right)
{
  struct dTuple point;
  point.x = lower_right.x - upper_left.x;
  point.y = upper_left.y - lower_right.y;
  point.x = upper_left.x + pixel.x * point.x / bounds.x;
  point.y = upper_left.y - pixel.y * point.y / bounds.y;
  return point;
}


// Test a complex number - if it does not leave a circle radius 2 centered on the
// origin we assume it is part of the mandelbrot set.
char escapes(double complex c, int limit)
{
  double complex z = 0;
  for(int i = 1; i < limit; i++)
  {
    z = z * z + c;
    if(creal(z) * creal(z) + cimag(z) * cimag(z) > 4)
    {
      return i;
    }
  }
  return 0;
}

// Render a portion of the Mandelbrot set into a buffer
// Expects render_arguments struct as the argument:
//   char* buffer to render into
//   struct iTuple bounds with the pixel diminsions of the buffer
//   structs dTuple upper_left and lower_right of the area on the complex plane to render
void *render(void *arguments)
{
    struct render_arguments* ra = arguments;
     for(int r = 0; r<ra->bounds.y; r++)
     {
       for(int c = 0; c < ra->bounds.x; c++)
       {
         struct iTuple lpoint = {.x = c, .y = r};
         struct dTuple point = pixel_to_point(ra->bounds,
                                              lpoint,
                                              ra->upper_left,
                                              ra->lower_right);
         int i = r * ra->bounds.x + c;

         ra->pixels[i] = escapes(point.x +  point.y * I, 255);
         if (ra->pixels[i] != 0)
            ra->pixels[i] = 255 -ra->pixels[i];
       }
     }
}


int main ( int argc, char **argv)
{
  if(argc < 5){
    printf("Usage: mandelbrot FILE PIXELS UPPERLEFT LOWERRIGHT [MAX_THREADS [OMIT_DRAWING]]\n");
    printf("Example: mandelbrot mandel.png 1000x1000 -2.0,2.0 2.0,-2.0 4 1\n");
    return 1;
  }
  struct iTuple bounds;
  struct dTuple upper_left;
  struct dTuple lower_right;
  int num_threads;

  bounds.y = atoi(split_str(argv[2], 'x'));
  bounds.x = atoi(argv[2]);

  upper_left.y = strtod(split_str(argv[3], ','), NULL);
  upper_left.x = strtod(argv[3], NULL);

  lower_right.y = strtod(split_str(argv[4], ','), NULL);
  lower_right.x = strtod(argv[4], NULL);

  if(argc > 5)
  {
    num_threads = atoi(argv[5]);
  }
  else
  {
    num_threads = 8;
  }
  // printf("Bounds: %d x %d\n", bounds.x, bounds.y);
  // printf("Upper left: %f , %f\n", upper_left.x, upper_left.y);
  // printf("Lower right: %f , %f\n", lower_right.x, lower_right.y);

  // use space on heap to avoid overflowing stack for large images
  char* pixels = malloc(sizeof(char) * bounds.x * bounds.y);
  // Divide image into equal chunks and give each its own pthread
  pthread_t thread_ids[num_threads];
  struct render_arguments thread_args[num_threads];
  int shard_rows = bounds.y / num_threads + 1;
  for(int ii=0; ii < num_threads; ii++)
  {
    int top = shard_rows * ii;
    struct iTuple shard_bounds;
    shard_bounds.x = bounds.x;
    shard_bounds.y = (bounds.y < top + shard_rows) ? bounds.y - shard_rows * ii: shard_rows;
    thread_args[ii] = (struct render_arguments){
              &pixels[shard_bounds.x * top],
              shard_bounds,
              pixel_to_point(bounds, (struct iTuple){0, top}, upper_left, lower_right),
              pixel_to_point(bounds, (struct iTuple){bounds.x, top + shard_bounds.y}, upper_left, lower_right)
          };
    pthread_create(&thread_ids[ii], NULL, render, &thread_args[ii]);
  }
  // wait to finish
  for(int ii=0; ii < num_threads; ii++)
  {
    pthread_join(thread_ids[ii], NULL);
  }

  // Only write to file if there is no 7th arg
  // (so that file IO doesn't skew computational speed tests)
  if(argc < 7)
  {
    FILE *fp = fopen(argv[1], "wb"); if (!fp) abort();
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL, NULL, NULL); if(!png) abort();
    png_infop info = png_create_info_struct(png); if (!info) abort();
    if (setjmp(png_jmpbuf(png))) {
        fprintf(stderr, "Error during png creation\n");
        abort();
    }

    png_init_io(png, fp);
    png_set_IHDR(png, info, bounds.x, bounds.y, 8, PNG_COLOR_TYPE_GRAY,
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    for(int i = 0; i < bounds.x * bounds.y; i+=bounds.x)
      png_write_row(png, pixels + i);
    png_write_end(png, NULL);
  }

  return 0;
}
