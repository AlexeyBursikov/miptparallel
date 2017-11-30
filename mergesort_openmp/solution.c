#include <assert.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int cmp(const void* a, const void* b) {
    return *(int*)a - *(int*)b;
}

void simple_merge(int* array, int lf, int rf, int ls, int rs, int* buffer){
    int size = rf - lf + rs - ls;
    int f_it = lf;
    int s_it = ls;
    for (int i = 0; i < size; i++) {
        if ((f_it < rf) && (s_it < rs)) {
            if (array[f_it] < array[s_it]){
                buffer[i] = array[f_it];
                f_it++;
            }
            else {
                buffer[i] = array[s_it];
                s_it++;
            }
        }
        else {
            if (f_it < rf) {
                buffer[i] = array[f_it];
                f_it++;
            }
            else {
                buffer[i] = array[s_it];
                s_it++;
            }
        }
    }
}

void merge(int* array, int lf, int rf, int ls, int rs, int* buffer) {
    int mid_first = (lf + rf) / 2;
    int mid_second;

    int cur_l = ls;
    int cur_r = rs;
    mid_second = cur_l;

    while (cur_l < cur_r) {
        int it = (cur_r + cur_l) / 2;
        if (array[it] >= array[mid_first]){
            cur_r = it;
        }
        else {
            if (cur_l != it) {
                cur_l = it;
            }
            else{
                break;
            }
        }
    }
    mid_second = cur_r;
   

    int size_f = mid_first - lf + mid_second - ls;
    int size_s = rf - mid_first + rs - mid_second;
    
    #pragma omp parallel sections
    {
        #pragma omp section 
        {
            simple_merge(array, lf, mid_first, ls, mid_second, buffer + lf);
        }

        #pragma omp section
        {
            simple_merge(array, mid_first, rf, mid_second, rs, buffer + lf + size_f);
        } 
    }

    #pragma omp parallel for
    for (int i = lf; i < rs; i++) {
        array[i] = buffer[i];
    }
}  


void merge_sort_iteration(int* array, int begin, int end, int basket_size, int* buffer){
    int size = end - begin;

    if (size <= basket_size){
        qsort(array + begin, size, sizeof(int), cmp);
    }
    else {

        int mid = (begin + end) / 2;
        int* f_part;
        int* s_part;
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                merge_sort_iteration(array, begin, mid, basket_size, buffer);
            }
            #pragma omp section
            {
                merge_sort_iteration(array, mid, end, basket_size, buffer);
            }
        }

        merge(array, begin, mid, mid, end, buffer);
    }
}

void merge_sort(int* array, int size, int basket_size){
    int* buffer = malloc(sizeof(int) * size);

    merge_sort_iteration(array, 0, size, basket_size, buffer);

    free(buffer);
}

void initialize(int* array, int n){
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        array[i] = rand() % 1000;
    }
}

int main(int argc, char** argv) {
    unsigned int n = atoi(argv[1]);
    unsigned int basket_size = atoi(argv[2]);
    unsigned int P = atoi(argv[3]);

    omp_set_num_threads(P);
    int* array = malloc(sizeof(int) * n);
    initialize(array, n);

    FILE* data;
    if (!(data = fopen("data.txt", "w"))) {
        printf("Error opening file\n");
    }
    else {
        for (int i = 0; i < n; i++) {
            fprintf(data, "%d ", array[i]);
        } 
        fprintf(data, "\n");
    }
    
    
    struct timeval start, end;
    assert(gettimeofday(&start, NULL) == 0);
        
    merge_sort(array, n, basket_size);
    //qsort(array, n, sizeof(int), cmp);

    assert(gettimeofday(&end, NULL) == 0);
    double delta = ((end.tv_sec - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
    printf("time: %.5fs\n", delta);

    FILE* stats;
    if (!(stats = fopen("stats.txt", "w"))) {
        printf("Error opening file\n");
    }
    else {
        fprintf(stats, "%.5fs %d %d %d\n", delta, n, basket_size, P);
        fclose(stats);
    }


    if (!data) {
        printf("Error opening file\n");
    }
    else {
        for (int i = 0; i < n; i++) {
            fprintf(data, "%d ", array[i]);
        } 
        fprintf(data, "\n");
    	fclose(data);
    }


    free(array);
    return 0;
}
