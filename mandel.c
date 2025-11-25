/// 
//  mandel.c
// CPE2600 - 111: Multiprocessing Lab 11
//  Based on example code found here:
//  https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
//
//  Converted to use jpg instead of BMP and other minor changes
//  
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "jpegrw.h"

typedef struct {
    imgRawImage *img;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    int max;
    int start_y;
    int end_y;
    int width;
    int height;
} ThreadArgs;

// Local routines
static int iteration_to_color(int i, int max);
static int iterations_at_point(double x, double y, int max);
static void compute_image(imgRawImage *img, double xmin, double xmax,
                          double ymin, double ymax, int max, int num_threads);
static void show_help();
void* compute_thread(void* args); // Worker thread function

int main(int argc, char *argv[])
{
    char c;

    // Default configuration
    const char *outfile = "mandel.jpg";
    double xcenter = 0;
    double ycenter = 0;
    double xscale = 4;
    double yscale = 0; // calculated later
    int    image_width = 1000;
    int    image_height = 1000;
    int    max = 1000;
    int    num_threads = 1;

    //Parse command line arguments
    while((c = getopt(argc, argv, "x:y:s:W:H:m:o:ht:")) != -1){
        switch(c) {
            case 'x': xcenter = atof(optarg); break;
            case 'y': ycenter = atof(optarg); break;
            case 's': xscale = atof(optarg); break;
            case 'W': image_width = atoi(optarg); break;
            case 'H': image_height = atoi(optarg); break;
            case 'm': max = atoi(optarg); break;
            case 'o': outfile = optarg; break;
            case 't': num_threads = atoi(optarg); break;
            case 'h': show_help(); exit(1); break;
        }
    }

    // Safety check for threads
    if (num_threads < 1) num_threads = 1;
    if (num_threads > 20) num_threads = 20; // Cap at 20 as per common sense/lab limits

    // Calculate y scale
    yscale = xscale / image_width * image_height;

    printf("mandel: x=%lf y=%lf xscale=%lf yscale=%lf max=%d threads=%d outfile=%s\n",
           xcenter, ycenter, xscale, yscale, max, num_threads, outfile);

    // Init image
    imgRawImage* img = initRawImage(image_width, image_height);
    setImageCOLOR(img, 0);

    // Compute image using threads
    compute_image(img, xcenter-xscale/2, xcenter+xscale/2,
                  ycenter-yscale/2, ycenter+yscale/2, max, num_threads);

    // Save and free
    storeJpegImageFile(img, outfile);
    freeRawImage(img);

    return 0;
}

/*
 * Worker thread function.
 * calculates Mandelbrot iterations for a specific slice of rows
 */
void* compute_thread(void* args){
    ThreadArgs* targs =(ThreadArgs*)args;

    int i, j;
    int width = targs->width;
    int height = targs->height;

    for (j = targs->start_y; j < targs->end_y; j++) {
        for (i = 0; i < width; i++) {
            //Determine the point in x,y space for that pixel.
            double x = targs->xmin + i *(targs->xmax - targs->xmin) / width;
            double y = targs->ymin + j * (targs->ymax - targs->ymin) / height;

            // Compute the iterations at that point.
            int iters = iterations_at_point(x, y, targs->max);

            // Set the pixel 
            setPixelCOLOR(targs->img, i,j, iteration_to_color(iters, targs->max));
        }
    }
    pthread_exit(NULL);
}

/*
 * Orchestrates the threads to compute the image.
 */
void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max, int num_threads)
{
    pthread_t threads[num_threads];
    ThreadArgs thread_args[num_threads];
    int height = img->height;

    // Calculate how many rows each thread handles
    int rows_per_thread = height / num_threads;

    for (int i = 0; i < num_threads; i++) {
        thread_args[i].img = img;
        thread_args[i].xmin = xmin;
        thread_args[i].xmax = xmax;
        thread_args[i].ymin = ymin;
        thread_args[i].ymax = ymax;
        thread_args[i].max = max;
        thread_args[i].width = img->width;
        thread_args[i].height = img->height;

        // Calculate slice range
        thread_args[i].start_y = i * rows_per_thread;
        
        // The last thread takes all remaining rows (handles rounding errors)
        if (i == num_threads - 1) {
            thread_args[i].end_y = height;
        } else {
            thread_args[i].end_y = (i + 1) * rows_per_thread;
        }

        // Create the thread
        if (pthread_create(&threads[i], NULL, compute_thread, (void*)&thread_args[i]) != 0) {
            perror("pthread_create");
            exit(1);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

int iterations_at_point(double x, double y, int max)
{
    double x0 = x;
    double y0 = y;
    int iter = 0;

    while ((x*x + y*y <= 4) && iter < max) {
        double xt = x*x - y*y + x0;
        double yt = 2*x*y + y0;
        x = xt;
        y = yt;
        iter++;
    }
    return iter;
}

int iteration_to_color(int iters, int max)
{
    int color = 0xFFFFFF * iters / (double)max;
    return color;
}

void show_help()
{
    printf("Use: mandel [options]\n");
    printf("Where options are:\n");
    printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
    printf("-x <coord>  X coordinate of image center point. (default=0)\n");
    printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
    printf("-s <scale>  Scale of the image in Mandelbrot coordinates (X-axis). (default=4)\n");
    printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
    printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
    printf("-o <file>   Set output file. (default=mandel.jpg)\n");
    printf("-t <threads> Number of threads to use. (default=1)\n");
    printf("-h          Show this help text.\n");
}