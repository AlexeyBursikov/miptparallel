#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

void func(int argc, char **argv) {
  int rank;
  int size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  int l = atoi(argv[1]);
  int a = atoi(argv[2]);
  int b = atoi(argv[3]);
  int N = atoi(argv[4]);
    
  unsigned int seed;
  unsigned int *seeds = (unsigned int*)calloc(size, sizeof(unsigned int));
  if (rank == 0) {
    srand(time(NULL));
    for (int i = 0; i < size; ++i) {
      seeds[i] = rand();
    }
  }
  MPI_Scatter(seeds, 1, MPI_UNSIGNED, &seed, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

  int *table = (int*) calloc( a * b * l * l, sizeof(int));
  for(int i = 0; i < N; ++i) {
    int x = rand_r(&seed) % l;
    int y = rand_r(&seed) % l;
    int r = rand_r(&seed) % (a * b);
    ++table[(r / a * l + y) * a * l + r % a * l + x];
  }
  
  MPI_Aint intex;
  MPI_Type_extent(MPI_INT, &intex);
  
  MPI_File fh;
  MPI_File_open(MPI_COMM_WORLD, "data.bin", MPI_MODE_CREATE |  MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
  
  MPI_Datatype view;
  MPI_Type_vector(a*b*l*l, 1, size, MPI_INT, &view);
  MPI_Type_commit(&view);
  
  int offset = rank * intex;
  MPI_File_set_view(fh, offset, MPI_INT, view, "native", MPI_INFO_NULL);
  
  MPI_File_write(fh, table, l * l * size, MPI_INT, MPI_STATUS_IGNORE);
  
  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Type_free(&view);
  MPI_File_close(&fh);
  free(seeds);
  free(table);
}

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  int rank;
  int size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int l = atoi(argv[1]);
  int a = atoi(argv[2]);
  int b = atoi(argv[3]);
  int N = atoi(argv[4]);

  struct timeval start, end;
  if (rank == 0) {
    gettimeofday(&start, NULL);;
  }

  func(argc, argv);

  if (rank == 0) {
    gettimeofday(&end, NULL);
    double delta = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) / 1.e6;
    
    FILE *f = fopen("stats.txt", "a");
    if (f == NULL) {
      printf("Error opening file!\n");
      exit(1);
    }
    fprintf(f, "%d %d %d %d %fs\n", l, a, b, N, delta);
    printf("time: %fs\n", delta);
    fclose(f);
  }
  
  MPI_Finalize();
  return 0;
}
