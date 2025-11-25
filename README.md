# System Programming Lab 12 Multiprocessing
### Name: Jaffar Mahkamov

---

## Overview

This lab explores unix multiprocessing with the generation of Mandelbrot fractal images using fork(), execlp(), waitpid(), and runtime performance measurement  

Executables produced:

1. **`mandel`** – generates a single Mandelbrot image in JPEG format.  
2. **`mandel_movie`** – launches multiple `mandel` processes in parallel to create a sequence of zoom frames, can enter -p #, to select how many processes to use, -n # to choose the amount of frames to generate

The frames can then be stitched into a video using fmpeg -i
mandel%d.jpg mandel.mpg , allowing us to examine how concurrency affects runtime.

![alt text](<images/CPE2600 lab 11 runtime.png>)
![alt text](<images/Screenshot CPE2600 Lab12.png>)

---
## Discussion

In this lab, I extended th mandelbrot program to support multithreading using the pthread library. The compute_image function was modified to partition the image generation work. Now the image height is split into regions based on the thread count.

i.
Both techniques gave a big performance gain versus the single-threaded implementation.
Multiprocessing seemed to have a higher impact on runtime in this lab, as seen in the table.

ii.
The data shows that the best runtime was with 20 processes and 10 threads,
or 10 processes and 5 threads, each having very similar results and the fastest 
of all the combinations.

## Build Instructions

To compile both programs:

```bash
make

./mandel_movie 

