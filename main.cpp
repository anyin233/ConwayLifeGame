#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "omp.h"

#define BINARY_OUT

#define N 2048
int a[N][N] = {0};
int tmp[N][N] = {0};
int dx[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
int dy[9] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};


void readPattern(char *filename) {
    FILE *file = fopen(filename, "r");
    char line[256];
    int x = -1, y = -1;

    int row, col;
    for (int i = 0; fgets(line, 256, file) != NULL; ++i) {
        if (i == 0) {
            continue;
        } else if (i == 1) {
            sscanf(line, "x = %d, y = %d", &x, &y);
            row = (N - y) / 2;
            col = (N - x) / 2;
        } else {
            // printf("%s", line);
            int len = strlen(line);
            int run_count = 1, tag = 0, cur = 0;
            while (cur < len) {
                if ('0' <= line[cur] && line[cur] <= '9') {
                    sscanf(line + cur, "%d", &run_count);
                    // printf("run count %d\n", run_count);
                    while ((cur < len) && ('0' <= line[cur] && line[cur] <= '9')) {
                        // printf("cur %d %c \n", cur, line[cur]);
                        cur++;
                    }
                } else if (line[cur] == 'b' || line[cur] == 'o') {
                    tag = line[cur];
                    for (int k = 0; k < run_count; ++k) {
                        if (tag == 'o') a[row][col + k] = 1;
                    }
                    col += run_count;
                    run_count = 1;
                    cur++;
                } else if (line[cur] == '$') {
                    for (int k = 0; k < run_count; ++k) {
                        row++;
                    }
                    col = (N - x) / 2;
                    run_count = 1;
                    cur++;

                } else if (line[cur] == '\n') {
                    cur++;
                } else if (line[cur] == '!') {
                    return;
                }
            }
        }
    }
}


void runConwayLifeGame(int max_iter) {
    int state[1 << 9];
    int i, j, ii, jj, cnt;
    unsigned bitmap;

    for (bitmap = 0; bitmap < 1 << 9; ++bitmap) { // initial bitmap looking table
        for (cnt = j = 0; j < 9; ++j) {
            if (bitmap & 1 << j) {
                cnt++;
            }
        }
        if (bitmap & 020) // cur state is center is live
        {
            if (cnt == 2 || cnt == 3) {
                state[bitmap] = 1;
            } else {
                state[bitmap] = 0;
            }
        } else // cur state is center is dead
        {
            if (cnt == 3) {
                state[bitmap] = 1;
            } else {
                state[bitmap] = 0;
            }
        }
    }

    for (int iter = 0; iter < max_iter; ++iter) {
        printf("Iter %d...\n", iter);
#pragma omp parallel for default(none) shared(dy, dx, a, tmp, state) private(i, j, ii, jj, cnt, bitmap) schedule(guided)
        for (i = 1; i < N - 1; ++i) {
            int k;
            bitmap = 0;
            j = 1;
            for (k = 3; k < 9; ++k)
            {
                ii = i + dx[k];
                jj = j + dy[k];
                bitmap |= a[ii][jj] << k;
            }
            tmp[i][j] = state[bitmap];
            for (j = 2; j < N - 1; ++j) {
                bitmap = (bitmap << 3) & 0x1FF;
                for (k = 6; k < 9; ++k) { // check count of alive cell
                    ii = i + dx[k];
                    jj = j + dy[k];
                    bitmap |= a[ii][jj] << k;
                }
                tmp[i][j] = state[bitmap];
            }
        }
        // for (int i = 0; i < N; ++i) {
        //   for (int j = 0; j < N; ++j) {
        //     a[i][j] = tmp[i][j];
        //   }
        // }
        memcpy(a, tmp, sizeof tmp);
#ifdef DEBUG
        char filename[10];
    sprintf(filename, "./output/iter%d", iter);
    FILE* f = fopen(filename, "w");
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
        fprintf(f, "%d ", tmp[i][j]);
      }
      fprintf(f, "\n");
    }
    fclose(f);
#endif
    }

    FILE *f = fopen("test_output", "wb");
#ifdef BINARY_OUT
    fwrite(a, N*N, sizeof(int), f);
#else
    char *str = (char *) malloc((N * (N + 1) + 1) * sizeof(char));
    for (int i = 0; i < N; ++i) {
        int j;
        for (j = 0; j < N; ++j) {
            str[i * (N + 1) + j] = a[i][j] + '0';
        }
        str[i * (N + 1) + j] = '\n';
    }
    str[N * (N + 1)] = 0;
    fwrite(str, N * (N + 1), sizeof(char), f);
    free(str);
#endif
    fclose(f);
}

int main(int argc, char **argv) {

    readPattern("test_pattern");

    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);

    runConwayLifeGame(1000);

    gettimeofday(&end, NULL);
    printf("%lf ms\n", ((end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000.0));
}
