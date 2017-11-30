#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>  
#include <time.h>

int main(int argc, char** argv){
    int a = atoi(argv[1]);
    int b = atoi(argv[2]);
    int x = atoi(argv[3]);
    int N = atoi(argv[4]);
    double pr = atof(argv[5]);
    int P = atoi(argv[6]);
    int suc_num = 0;
    int tot_step = 0;
    
    struct timeval start, end;
    assert(gettimeofday(&start, NULL) == 0);

    unsigned int *seeds = (unsigned int*)calloc(N, sizeof(unsigned int));
    srand(time(NULL));
    for (int i = 0; i < N; ++i) {
      seeds[i] = rand();
    }

    omp_set_num_threads(P);
    #pragma omp parallel for default(shared) reduction(+: suc_num, tot_step)
    for (int i = 0; i < N; i++) {
        int seed = seeds[i];
        int pos = x;
        float random_val;
        while ((pos < b) && (pos > a)) {
            random_val = (rand_r(&seed) % 10001) * 1.0 / 10000;
            if (random_val > pr) {
                pos++;
            }
            if (random_val < pr) {
                pos--;
            }
            tot_step += 1;
        }
        if (pos >= b) {
            suc_num += 1;
        }
    }
    free(seeds);

    assert(gettimeofday(&end, NULL) == 0);
    double delta = ((end.tv_sec - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
    printf("time: %f\n", delta);

    FILE* file;
    if (!(file = fopen("stats.txt", "w"))) {
        printf("Error opening file\n");
    }
    else {
        fprintf(file, "prob: %f, steps: %f, time: %f, %d, %d, %d, %d, %f, %d\n",(suc_num * 1.0 / N), (tot_step * 1.0 / N), delta, a, b, x, N, pr, P);
        fclose(file);
    }

    return 0;
}
