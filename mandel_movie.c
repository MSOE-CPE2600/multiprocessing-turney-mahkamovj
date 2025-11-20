/* File: mandel_movie.c
 * CPE2600 Multiprocessing Lab 11
 * Name: Jaffar Mahkamov
 * Date: 11/19/25
 * Create a Mandelbrot movie using multiple processes
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     
#include <sys/types.h>
#include <sys/wait.h>  
#include <time.h>      
#include <math.h>   
#include <sys/time.h>   

int main(int argc, char *argv[])
{
    int opt;
    int max_procs  = 1;   
    int num_frames = 50;   

    
    double s_start = 0.5;     
    double s_end = 0.0001;    
    int width = 1000;
    int height = 1000;
    int max_iter = 1000;

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    int total_frames_generated = 0;
    int max_concurrent = 0;


    // Parse command line options
    // -p <num_processes>
    // -n <num_frames>
    while ((opt = getopt(argc, argv, "p:n:")) != -1) {
        switch (opt) {
        case 'p':
            max_procs = atoi(optarg);
            break;
        case 'n':
            num_frames = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s [-p num_processes] [-n num_frames]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (max_procs < 1) {
        fprintf(stderr, "num_processes must be >= 1\n");
        exit(EXIT_FAILURE);
    }
    if (num_frames < 1) {
        fprintf(stderr, "num_frames must be >= 1\n");
        exit(EXIT_FAILURE);
    }

    //double xmin_center = -2.5;
    //double xmax_center =  1.0;
    //double ymin_center = -1.5;
    //double ymax_center =  1.5;


    double xcenter = -0.743643887;
    double ycenter =  0.131825904;


    printf("mandelmovie: random center (x=%lf, y=%lf)\n", xcenter, ycenter);
    printf("             frames=%d, max_procs=%d\n", num_frames, max_procs);

    int frame = 0;
    int active_children = 0;

    
    while (frame < num_frames || active_children > 0) {

        if (frame < num_frames && active_children < max_procs) {
            int this_frame = frame;  // capture current frame index
            frame++;

            if (active_children + 1 > max_concurrent) max_concurrent = active_children + 1;

            double t;
            if (num_frames == 1) {
                t = 0.0;
            } else {
                t = (double)this_frame / (double)(num_frames - 1);
            }
            double s_k = s_start * pow(s_end / s_start, t);

            //build strings for mandel arguments
            char x_str[64], y_str[64], s_str[64];
            char w_str[16], h_str[16], m_str[16];
            char outfile[64];

            snprintf(x_str, sizeof(x_str), "%lf", xcenter);
            snprintf(y_str, sizeof(y_str), "%lf", ycenter);
            snprintf(s_str, sizeof(s_str), "%lf", s_k);
            snprintf(w_str, sizeof(w_str), "%d", width);
            snprintf(h_str, sizeof(h_str), "%d", height);
            snprintf(m_str, sizeof(m_str), "%d", max_iter);
            snprintf(outfile, sizeof(outfile), "mandel%d.jpg", this_frame);

            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) {
        
                execlp("./mandel", "mandel",
                       "-x", x_str,
                       "-y", y_str,
                       "-s", s_str,
                       "-W", w_str,
                       "-H", h_str,
                       "-m", m_str,
                       "-o", outfile,
                       (char *)NULL);

                // error catch
                perror("execlp");
                _exit(EXIT_FAILURE);
            } else {
                // Parent: increment active children count
                active_children++;
            }

        } else {
            // Wait for one child to finish.
            int status;
            pid_t done = waitpid(-1, &status, 0);
            if (done > 0) {
                active_children--;
                total_frames_generated++;
            } else {
                
                break;
            }
        }
    }

    printf("mandelmovie: done generating %d frames.\n", num_frames);


    gettimeofday(&end_time, NULL);

    double total_sec = (end_time.tv_sec  - start_time.tv_sec) +(end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    printf("\nRuntime\n");
    printf("Total frames completed: %d\n", total_frames_generated);
    printf("Max processes in flight: %d\n", max_concurrent);
    printf("Total runtime:   %.4f sec\n", total_sec);
    printf("Avg time per frame:  %.6f sec\n",total_sec / total_frames_generated);
    

    return 0;
}
