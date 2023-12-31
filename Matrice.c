#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_ROWS 10
#define MAX_COLS 10
#define THREADS MAX_ROWS

int B[MAX_ROWS][MAX_COLS], C[MAX_COLS][MAX_COLS], A[MAX_ROWS][MAX_COLS], T[MAX_ROWS][MAX_COLS];
pthread_t threads[THREADS * 2];
sem_t empty, full;

typedef struct {
    int row;
} ThreadArgs;

void fill_matrix_random(int rows, int cols, int matrix[MAX_ROWS][MAX_COLS]) {
    int i, j;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            matrix[i][j] = rand() % 10; // Remplir avec des valeurs aléatoires entre 0 et 9
        }
    }
}

void *producer(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int j, k;
    int sum;

    for (j = 0; j < MAX_COLS; j++) {
        sum = 0;
        for (k = 0; k < MAX_COLS; k++) {
            sum += B[args->row][k] * C[k][j];
        }
        T[args->row][j] = sum;
    }

    sem_post(&full);

    free(args); // Libérer la mémoire ici

    return NULL;
}

void *consumer(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int j;

    sem_wait(&full);

    for (j = 0; j < MAX_COLS; j++) {
        A[args->row][j] = T[args->row][j];
    }

    sem_post(&empty);

    free(args); // Libérer la mémoire ici

    return NULL;
}

int main() {
    int i, j;
    int rows_b, cols_b, rows_c, cols_c;

    // Get dimensions for matrix B from the user
    printf("Enter dimensions for Matrix B (rows cols): ");
    scanf("%d %d", &rows_b, &cols_b);

    // Get dimensions for matrix C from the user
    printf("Enter dimensions for Matrix C (rows cols): ");
    scanf("%d %d", &rows_c, &cols_c);

    if (cols_b != rows_c || rows_b > MAX_ROWS || cols_b > MAX_COLS || cols_c > MAX_COLS) {
        printf("Invalid matrix dimensions. Exiting...\n");
        return 1;
    }

    // Fill matrices B and C with random values
    fill_matrix_random(rows_b, cols_b, B);
    fill_matrix_random(rows_c, cols_c, C);

    // Initialize semaphores
    sem_init(&empty, 0, THREADS);
    sem_init(&full, 0, 0);

    // Create threads
    for (i = 0; i < THREADS; i++) {
        ThreadArgs *prod_args = malloc(sizeof(ThreadArgs));
        prod_args->row = i;
        if (pthread_create(&threads[i], NULL, producer, prod_args) != 0) {
            perror("pthread_create error (producer) !");
        }

        ThreadArgs *cons_args = malloc(sizeof(ThreadArgs));
        cons_args->row = i;
        if (pthread_create(&threads[i + THREADS], NULL, consumer, cons_args) != 0) {
            perror("pthread_create error (consumer) !");
        }
    }

    // Wait for the threads to finish
    for (i = 0; i < THREADS * 2; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join error!");
        }
    }

    // Display matrix A (result)
    printf("Matrix A (result):\n");
    for (i = 0; i < THREADS; i++) {
        int firstNonZero = 1;  // Flag to track the first non-zero element
        for (j = 0; j < MAX_COLS; j++) {
            if (A[i][j] != 0) {
                if (!firstNonZero) {
                    printf(" ");
                }
                printf("%d", A[i][j]);
                firstNonZero = 0;
            }
        }
        printf("\n");
    }

    // Destroy semaphores
    sem_destroy(&empty);
    sem_destroy(&full);

    return 0;
}
